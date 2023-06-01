from sizes import sizes
from items import *
from dice import Dice
from stats import *

class Character(object):
    id = Identifier("id")
    name = Identifier("name")
    color = Property("color")
    portrait = Property('portrait')
    level = Stat("level")
    physical = Ability("physical")
    mental = Ability("mental")
    social = Ability("social")
    hp = CounterStat('hp')
    sp = CounterStat('sp')
    mp = CounterStat('mp')
    defense = Stat("defense")

    def __init__(self, **kwargs):
        self.effects = list()
        self.worn = list()
        size = kwargs.get('size', sizes['m'])
        if type(size) == str:
            size = sizes.get(size)
        self.stats = dict(id=None,
                          name='Nemo',
                          color=None,
                          portrait=None,
                          level=1,
                          physical = size['physical'],
                          mental=1,
                          social=1,
                          defense=0
        )
        self.stats = {stat:kwargs.get(stat, value) for stat, value in self.stats.items()}
        self.stats['hp'] = Counter(self.max_hp())
        self.hp = kwargs.get('hp', self.hp)
        self.stats['sp'] = Counter(self.max_sp())
        self.sp = kwargs.get('sp', self.sp)
        self.stats['mp'] = Counter(self.max_mp())
        self.mp = kwargs.get('mp', self.mp)

        self.skill = kwargs.get('skills', [])
        self.inventory = kwargs.get('inventory',[])
        # TODO: find a way to put equipped status in a record, but still allow duplicate items and duplicate location
        # list of tuples? or dict with locations, then list of items there.
        self.held = dict(main=None, off=None) # TODO: less hands for paws, more hands for weird creatures
        if self.inventory:
            self.auto_equip()

    def __getitem__(self, key):
        return self.stats[key]

    def __setitem__(self, key, value):
        raise NotImplementedError("Not allowed (yet)")

    def __delitem__(self, key):
        raise NotImplementedError("Not allowed (yet)")

    def max_hp(self):
        return 4 + self.level + sum(self.get_boni('hp').values())

    def max_sp(self):
        return self.mental + sum(self.get_boni('sp').values())

    def max_mp(self):
        return self.social + sum(self.get_boni('mp').values())

    def max_ap(self):
        return 3 +  sum(self.get_boni('ap').values())

    def get_max(self, stat):
        if stat=='hp':
            return self.max_hp()
        elif stat=='sp':
            return self.max_sp()
        elif stat=='mp':
            return self.max_mp()
        elif stat=='ap':
            return self.max_ap()
        else:
            raise ValueError(f"Unknown counter {stat}")

    def get_boni(self, stat):
        boni = dict()
        for e in self.effects:
            boni.update(e.get_boni(stat))
        for w in self.worn:
            boni.update(w.get_boni(stat))
        boni={k:v for k,v in boni.items() if v is not None}
        return boni

    def defense_dice(self):
        return Dice.for_ability(self.physical)+Dice(bonus=self.get_boni('defense').values())

    def attack(self, enemy, number=0, bonus=None):
        attack_roll = self.attack_dice(number, bonus).roll()
        defense_roll = enemy.defense_dice().roll()
        damage = attack_roll.total - defense_roll.total
        if damage < 0:
            return f"{self.name} misses {enemy.name} ({attack_roll} VS {defense_roll})"
        else:
            enemy.damage(damage)
            return f"{self.name} attacks ({attack_roll} VS {defense_roll}) {enemy.name} ({enemy['hp']})[-{damage}]"

    def damage(self, dmg):
        hp = self.hp - int(dmg)
        self.hp = hp

    def attack_dice(self, nr=0, bonus=None):
        result = Dice.for_ability(self.physical)
        if nr == 1:   # check for dual wielding
            if weapon:=self.off_hand():
                result += weapon.bonus()
            elif nr:
                result += nr*-2
        else:
            if weapon:=self.main_hand():
                result += weapon.bonus()
            if nr:
                result += nr*-2
        if bonus is not None:
            result+=bonus
        return result

    def bodyweight(self):
        return self.size.weight

    def weight(self):
        return self.bodweight() + self.carried()

    def carried(self):
        return sum(i.weight() for i in self.inventory)

    def capacity(self):
        return self.physical

    def obtain(self, items):
        self.inventory += items

    def main_hand(self):
        return self.held['main']

    def off_hand(self):
        return self.held['off']

    def item(self, name):
        return [i for i in self.inventory if i.name==name]

    def auto_equip(self):
        for i in self.inventory:
            if not self.main_hand() and i.hands() >= 1:
                self.held['main'] = i
            elif not self.off_hand() and \
                (not self.main_hand() or self.main_hand().hands() < 2) and \
                abs(i.hands())==1:
                self.held['off'] = i
            if not self.worn:
                if type(i) == Armor:
                    self.worn = [i]
        return self

    def __str__(self):
        return f"""{self.name}: Level {self.level}
    {self.physical} Physical: {self['hp']} HP
    {self.mental} Mental: {self['sp']} SP
    {self.social} Social: {self['mp']} MP
    Defense {self.defense_dice()} Attack {self.attack_dice()}
    Inventory[{self.carried()}/{self.capacity()}] {" ".join(str(i) for i in self.inventory) or "Empty"} 
    {" ".join(str(s) for s in self.skill)}"""

    def alive(self):
        return bool(self.hp) and bool(self.level)

    def moralized(self):
        return bool(self.mp)

    def active(self):
        return self.alive() and self.moralized()


    races = [dict(name='Imp', size='xs', social=3, mental=3),
             dict(name='Fairy', size='xs', social=4, mental=2),
             dict(name='Pixie', size='xs', social=2, mental=4),
             dict(name='Halfling', size='s', social=3, mental=2),
             dict(name='Gnome', size='s', social=2, mental=3),
             dict(name='Dwarf', size='s', physical=3, mental=2, social=2),
             dict(name='Orc', size='m', physical=4, mental=1, social=2),
             dict(name='Elf', size='m', mental=3, social=1),
             dict(name='Satyr', size='m', mental=1, social=3),
             dict(name='Human', size='m', social=2, mental=2),
             dict(name='Ogre', size='l')]

    @staticmethod
    def random_character(level=1):
        assert level == 1 # not yet implemented
        c=Character(**random.choice(Character.races))
        c.obtain(c.random_equipment())
        c.auto_equip()
        return c

    @staticmethod
    def random_monster(level):
        total_ability = 6+level
        assert total_ability>=3 # minium ability. Lowest level would be -3
        remaining_ability = total_ability-2 # reserve two for 1 each social and mental
        size = random.choice([s for s in sizes.values() if s['physical'] < remaining_ability])
        remaining_ability = total_ability - size['physical']
        social_bonus = random.randint(0, remaining_ability)
        mental_bonus = remaining_ability - social_bonus
        return Character(name=f'{size["name"]} Monster', size=size, social=1+social_bonus, mental=1+mental_bonus)


    def random_equipment(self):
        capacity = self.physical
        if capacity < 2:
            weapon = random.choice([Weapon(), RangedWeapon()])
        else:
            weapon = random.choice([Weapon(), Weapon(heavy=True), RangedWeapon(), RangedWeapon(heavy=True)])
        capacity -= weapon.weight()
        if capacity and weapon.hands() == 1:
            offhand = random.choice([Weapon(), Shield() if capacity else None, None])
            capacity -= offhand.weight() if offhand else 0
        else:
            offhand = None
        if capacity and (armor_rating := random.randint(0, min(capacity, 3))):
            armor = Armor(rating=armor_rating)
            capacity -= armor.weight()
        else:
            armor = None
        return [i for i in (weapon, offhand, armor) if i]
