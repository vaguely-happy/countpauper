import sys
import random
import json


##################### Load data
primary_stats=json.load(open("primary_stats.json"))
secondary_stats=json.load(open("secondary_stats.json"))
archetypes=json.load(open("archetypes.json"))
builds=json.load(open("builds.json"))
armor_stats=json.load(open("armor_stats.json"))
race_stats=json.load(open("race_stats.json"))


################ Test data

def test_primary_table_length(stats, req, key="table"):
        return [f"length of {key} in {name} is not {req}" for name, stat in stats.items() if len(stat.get(key,[])) != req]


def test_secondaries_exist(primary, secondary):
        return [f"boost {boost} in {name} does not exist"
                for name, stat in primary.items() for boost in stat.get("boost") if boost not in secondary]

def compute_range(boost, base, primary_stats):
        return [base, base+sum(
                stat["boost"].count(boost) for stat in primary_stats
        )]

def test_secondary_range(primary_stats, seconadry_stats):
        return [f"range of {secondary} is {computed_range} but should be {range}"
                for secondary, stats in seconadry_stats.items()
                        if (computed_range:=compute_range(secondary, stats.get("base", 0), primary_stats.values())) != (range:=stats.get("range"))]
def test_stats():
        return (test_primary_table_length(primary_stats, 15, "boost") +
                test_secondaries_exist(primary_stats, secondary_stats.keys()) +
                test_secondary_range(primary_stats, secondary_stats))








################### BUILD characters

def build_stat_tables():
        result = dict()
        for name, stat in primary_stats.items():
                boosts = {s: secondary_stats.get(s, dict()) for s in set(stat.get("boost")) }
                secondary_stat = {s:stat.get("base",0) * stat.get("multiplier", 1) for s, stat in boosts.items()}
                table = [dict(secondary_stat)]
                for boost in stat.get("boost"):
                        secondary_stat[boost] = secondary_stat.get(boost, 0) + boosts.get(boost).get("multiplier", 1)
                        table.append(dict(secondary_stat))
                result[name] = table
        return result


def build_plan(archetype, plan):
        "return an array of primary stat names by assigning archetype stat order to a balance plan"
        return [archetype[stat-1] for stat in plan]

def build_primary_stats(race_stats, build_plan):
        result = []
        stats=dict(level=0)
        stats.update({ stat: stat_def.get('base' ,0) + race_stats.get(stat, 0) for stat,stat_def in primary_stats.items() })
        for level in range(0, 10):
                while True:
                        boost = build_plan.pop()
                        new_stat  = stats.get(boost, 0) + 1
                        maxStat = 10+race_stats.get(boost, 0)
                        if new_stat <= maxStat:
                                stats[boost] = new_stat
                                stats["level"] += 1
                                result.append(dict(stats))
                                break

        return result

def derive_secondary_stats(primary_stat, stat_tables):
        if not stat_tables:
                return primary_stat
        result = dict(primary_stat)
        for primary, level in primary_stat.items():
                if primary=="level":
                        continue
                result.update(stat_tables.get(primary)[level])
        return result

# TODO this is a very rough heuristic where equipment is selected to account for defense and attack
# this could be according to a strategy (favor defense/offense range/melee attack/combat)
def derive_combat_stats(stats):
        weapon_weight = stats.get("carry")
        armor_weight = stats.get("endurance")
        shield_weight = 0                       # TODO would have to downscale weapon
        enchantment = stats.get("attune")
        # assume all magic items are + defense or + damage
        defense_enchantment = enchantment // 2
        offense_enchantment = enchantment - defense_enchantment
        stats["defense"] = armor_weight + shield_weight + defense_enchantment
        stats["offense"] = 2 + 2 * weapon_weight + stats.get("damage") + offense_enchantment
        stats["hp"] = stats.get("hd") * stats.get("level")
        return stats

def build_stats(race_stats, build_plan, stat_tables=None):
        "return an array of 10 levels with all primary stat scores at that level"

        primary_stats = build_primary_stats(race_stats, build_plan)
        return [derive_combat_stats(derive_secondary_stats(stat, stat_tables)) for stat in primary_stats]


def build_character(race, archetype, build, stat_tables=None):
        rstat=race_stats.get(race)
        astat=archetypes.get(archetype)
        build=builds.get(build)
        primary_stats_per_level=build_stats(rstat["stat_bonus"], build_plan(astat, build), stat_tables)
        return primary_stats_per_level

################# Simulate combat

def compute_DPR(attacker, defender):
        offense = attacker.get("offense")
        defense = attacker.get("defense")
        crit = attacker.get("crit")
        dodge = defender.get("dodge")
        hit = 1.0 - dodge # TODO: can a crit miss? it seems weak currently
        actions = attacker.get("action")  # TODO: lose an action to a faster attacker if using ranged
        # Attack formula.
        # Hit chance = 0.5 - dodge (range and such not taken into account.
        # Normal hit damage = offense - defense
        # Crit damage = 2* offense - defense (ignoring defense is too weak against casters unless that's the rock paper scissor fighter -> caster -> thief)
        return actions * ((1-crit) * (offense - defense) + crit * (2*offense))  * hit


############### Render results


row_width = 7
def format_stats(stats):
        return "\t".join(f"{v}"[:row_width] for v in stats.values())

def format_table(table):
        "Format a list of dictionaries (assumed to have the same keys as the first) into a TSV"
        header = [head[:row_width] for head in table[0].keys()]
        return "\t".join(header) + nl + nl.join(format_stats(lvl_stats) for lvl_stats in table)

nl="\n"
if __name__ == "__main__":
        if test_result := test_stats():
                print(f"tests failed: {nl.join(test_result)}")
                exit()
        print("tests passed")

        stat_tables = build_stat_tables()

        # for stat,table in stat_tables.items():
        #         print(f"{stat}:")
        #         print(format_table(table))

        fighter = build_character("human", "fighter", "balanced", stat_tables)
        thief = build_character("human", "thief", "balanced", stat_tables)

        print(format_table(fighter))
        print("=========== VS ======= ")
        print(format_table(thief))

        for level in range(0, 10):
                fighter_stats = fighter[level]
                thief_stats = thief[level]
                level = fighter_stats.get("level")
                fighter_dpr = compute_DPR(fighter_stats, thief_stats)
                fighter_hp = fighter_stats.get("hp")

                thief_dpr = compute_DPR(thief_stats, fighter_stats)
                thief_hp = thief_stats.get("hp")

                fighter_survival = fighter_hp / thief_dpr
                thief_survival = thief_hp / fighter_dpr

                if fighter_survival > thief_survival:
                        win_hp = fighter_hp - (thief_survival * thief_dpr)
                else:
                        win_hp = (fighter_survival * fighter_dpr) - thief_hp

                print(f"[{level}] {thief_hp} hp / DPR {fighter_dpr:.2f} = {thief_survival:.2f} rounds == VS == {fighter_hp} hp / DPR {thief_dpr:.2f} = {fighter_survival:.2f} rounds  {'WIN' if win_hp>0 else 'LOSE'} ({abs(win_hp):.0f}hp)")


TODO in a separate script
# function to export the stats table, but flip it around from requested stat trace back to dependency with a single table each
# enumerate each of them an integer id too, to match the C representation and link using that instead of string
# make a list of tertiary stats with special things like hp, hands, weight, resistances, see the C interface
# some special stats (HD) here may get something hardcoded like the multiplier id or in the original json the multiplier is a string stat name instead of id
# or special carrying capacity is the sum of two
