#pragma once

#include <memory>
#include <istream>
#include <list>
#include "Engine/Coordinate.h"
#include "Game/Direction.h"
#include "Map.h"
#include "IGame.h"
#include "Item.h"
#include "Skills.h"


namespace Game
{
    class Object;
    struct Position;
    class Plan;
    class Actor;
    class State;

    class Game : public IGame
    {
    public:
        enum class Selection
        {
            None = 0,
            Map,
            Object
        };
        Game();
        virtual ~Game();
        void Start();

        //IGame
        State Get(const Actor& actor) const override;
        void Adjust(const Actor& actor, const State& state) override;
        void Apply(IGame& root) const override;

        void Tick();
        void Render() const;
        void Key(unsigned short code);
        bool CanBe(const Position& position) const override;
        bool CanGo(const Position& from, Direction direction) const override;
        bool Cover(const Position& from, const Position& to) const override;
        void Click(Selection selection, unsigned value);
        std::vector<Actor*> FindTargets(const State& from, const Skill& skill) const;
        std::vector<Actor*> FindTargetsInRange(const State& from, const Skill& skill) const;
        Actor* ActiveActor() const override;
        void MakePlan(Actor& actor, const Skill& skill);

        const Type::Armor& FindArmor(const std::wstring& name) const;
        const Type::Armor::Material&  FindArmorMaterial(const std::wstring& name, const Type::Armor& armor) const;
        const Type::Armor::Bonus& FindArmorBonus(const std::wstring& name, const Type::Armor& armor) const;
        const Type::Weapon& FindWeapon(const std::wstring& name) const;
        const Type::Weapon::Material&  FindWeaponMaterial(const std::wstring& name, const Type::Weapon& armor) const;
        const Type::Weapon::Bonus& FindWeaponBonus(const std::wstring& name, const Type::Weapon& armor) const;

        Engine::Coordinate focus;
        Skills skills;
    protected:
        void AI(Actor* actor);
        void Next();
        void FocusActor();
        std::wstring Description() const override;
        void TestDumpAllItems(std::wostream& out) const;

        friend std::wistream& operator>>(std::wistream& s, Game& game);
        Map map;
        typedef std::list<std::unique_ptr<Object>> Objects;
        Objects objects;
        Objects::iterator turn;
        std::unique_ptr<Plan> plan;

        std::vector<Type::Armor> armors;
        std::vector<Type::Armor::Material> armorMaterials;
        std::vector<Type::Armor::Bonus> armorBoni;
        std::vector<Type::Weapon> weapons;
        std::vector<Type::Weapon::Material> weaponMaterials;
        std::vector<Type::Weapon::Bonus> weaponBoni;
    };
    std::wistream& operator>>(std::wistream& s, Game& game);

}   // ::Game

