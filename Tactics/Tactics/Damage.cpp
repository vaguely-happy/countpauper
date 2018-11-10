#include "stdafx.h"
#include "Damage.h"
#include <sstream>

namespace Game
{
    const Wound::Table Wound::table = 
    {
        { Wound::Type::Blunt, {
            { ImmunePain, { L"Immune", L"Invulnerable", 0 } },
            { 0, { L"Healthy", L"Block", 0 } },
            { 1, { L"Bruised", L"Bruise", 0 } },
            { 3, { L"Battered", L"Batter", -1 } },
            { 6, { L"Crushed", L"Crush", -2 } },
            { 10, { L"Broken", L"Break", -3 } },
            { MaxPain, { L"Shattered", L"Shatter", -20 } }
        } },
        { Wound::Type::Sharp, {
            { ImmunePain, { L"Immune", L"Invulnerable", 0 } },
            { 0, { L"Healthy", L"Slip", 0 } },
            { 1, { L"Scratched", L"Scratch", 0 } },
            { 3, { L"Grazed", L"Graze", -1 } },
            { 6, { L"Cut", L"Cut", -2 } },
            { 10, { L"Hemorrhaging", L"Rent", -3 } },
            { MaxPain, { L"Severed", L"Sever", -20 } }
        } },        
        { Wound::Type::Burn, {
            { ImmunePain, { L"Immune", L"Invulnerable", 0 } },
            { 0, { L"Healthy", L"Fizzle", 0 } },
            { 1, { L"Singed", L"Singe", 0 } },
            { 3, { L"Scorched", L"Scorch", -1 } },
            { 6, { L"Seared", L"Sear", -2 } },
            { 10, { L"Burned", L"Burn", -3 } },
            { MaxPain, { L"Charred", L"Char", -20 } }
        } },
        { Wound::Type::Disease, {
            { ImmunePain, { L"Immune", L"Invulnerable", 0 } },
            { 0, { L"Healthy", L"Resist", 0 } },
            { 1, { L"Tainted", L"Taint", -1 } },
            { 3, { L"Infected", L"Infect", -2 } },
            { 6, { L"Festering", L"Fester", -3 } },
            { 10, { L"Decaying", L"Decay", -4 } },
            { MaxPain, { L"Disintegrated", L"Disintegrate", -20 } }
        } },
        { Wound::Type::Spirit, {
            { ImmunePain, { L"Immune", L"Invulnerable", 0 } },
            { 0, { L"Healthy", L"Endure", 0 } },
            { 1, { L"Frail", L"Enfeeble", -1 } },
            { 3, { L"Impaired", L"Impairs", -2 } },
            { 6, { L"Crippled", L"Cripples", -3 } },
            { 10, { L"Paralyzed", L"Paralyse", -4 } },
            { MaxPain, { L"Devastated", L"Devastate", -20 } }
        } }
    };

    const Wound& Wound::find(Wound::Type type, Pain severity)
    {
        const auto& table = Wound::table.at(type);
        auto it = table.lower_bound(severity);
        if (it == table.end())
            return table.rbegin()->second;
        else
            return it->second;
    }

    Damage::Damage() : 
        Damage(Score(), Score(), Score(), Score(), Score())
    {
    }

    Damage::Damage(const Score& sharp, const Score& blunt, const Score& burn, const Score& disease, const Score& spirit) :
        damage({ { Wound::Type::Sharp, sharp },
        { Wound::Type::Blunt, blunt },
        { Wound::Type::Burn, burn },
        { Wound::Type::Disease, disease },
        { Wound::Type::Spirit, spirit } })
    {
    }

    Damage::Damage(int sharp, int crush, int burn, int disease, int spirit)
    {
        SetSharp(sharp);
        SetBlunt(crush);
        SetBurn(burn);
        SetDisease(disease);
        SetSpirit(spirit);
    }


    std::wstring Damage::StateDescription() const
    {
        auto worst = FindWorst();
        std::wstringstream ss;
        auto wound = Wound::find(worst.first, worst.second);
        return wound.description;
    }

    std::wstring Damage::ActionDescription() const
    {
        auto worst = FindWorst();
        std::wstringstream ss;
        auto wound = Wound::find(worst.first, worst.second);
        return wound.action + L"(" + damage.at(worst.first).Description()+L")";
    }

    bool Damage::Hurt() const
    {
        auto worst = FindWorst();
        return worst.second > 0;
    }

    int Damage::StatPenalty() const
    {
        auto worst = FindWorst();
        auto wound = Wound::find(worst.first, worst.second);
        return wound.penalty;
    }
    bool Damage::Disabled() const
    {
        auto worst = FindWorst();
        if (worst.second == 0) return false;
        return unsigned(worst.second) >= MaxPain;
    }

    std::pair<Wound::Type, unsigned> Damage::FindWorst() const
    {
        Wound::Type worst = Wound::Type::Sharp;
        unsigned worstPain = 0;
        for (const auto& d: damage)
        {
            auto pain = d.second.Value();
            if (pain > worstPain)
            {
                worstPain = pain;
                worst = d.first;
            }
        }
        return std::make_pair(worst,worstPain);
    }

    Damage Damage::Wound(const std::wstring& description) const
    {
        return Damage(Score(description, Sharp().Value()),
            Score(description, Blunt().Value()),
            Score(description, Burn().Value()),
            Score(description, Disease().Value()),
            Score(description, Spirit().Value()));
    }

    std::wistream& operator>>(std::wistream& s, Damage& damage)
    {
        wchar_t comma;
        s >> damage.damage[Wound::Type::Sharp] >> comma;
        s >> damage.damage[Wound::Type::Blunt] >> comma;
        s >> damage.damage[Wound::Type::Burn] >> comma;
        s >> damage.damage[Wound::Type::Disease] >> comma;
        s >> damage.damage[Wound::Type::Spirit];
        return s;
    }

    std::wostream& operator<<(std::wostream& s, const Damage& damage)
    {
        s << damage.damage.at(Wound::Type::Sharp).Value() << L",";
        s << damage.damage.at(Wound::Type::Blunt).Value() << L",";
        s << damage.damage.at(Wound::Type::Burn).Value() << L",";
        s << damage.damage.at(Wound::Type::Disease).Value() << L",";
        s << damage.damage.at(Wound::Type::Spirit).Value();
        return s;
    }

    Damage Damage::operator+(const Damage& other) const
    {
        Damage o(*this);
        o += other;
        return o;
    }


    Damage& Damage::operator+=(const Damage& other)
    {

        for (auto& d : damage)
        {
            d.second += other.damage.at(d.first);
        }
        return *this;
    }

    Damage operator-(const Damage& damage, const Damage& mitigation)
    {   // NB: damage mitigation only
        Damage result(damage);
        for (auto& d : result.damage)
        {
            // if there is no damage to mitigation, penalties are not applied
            if (d.second.Value() > 0)
            {
                d.second -= mitigation.damage.at(d.first);
            }
        }
        return result;
    }

}
