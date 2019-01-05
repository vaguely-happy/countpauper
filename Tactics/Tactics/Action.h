#pragma once

#include <map>
#include <functional>
#include "Game/Trajectory.h"
#include "State.h"

namespace Game
{
    class Actor;
    class Game;

    class Action
    {
    public:
        Action(const Skill& skill, const Actor& actor, Trajectory trajectory);
        virtual ~Action() = default;
        virtual std::unique_ptr<GameState> Act(const IGame& game) const = 0;
        virtual std::unique_ptr<GameState> Fail(const IGame& game) const = 0;
        virtual void Render(const State& state) const = 0;
        virtual std::wstring Description() const = 0;
        static std::map<unsigned, std::function<Action*(const State& state, const Game& game)>> keymap;
        static std::map<std::wstring, std::function<Action*(const State& state, const Game& game)>> typemap;
        const Actor& actor;
        const Skill& skill;
        const Trajectory trajectory;
    };
    
    class TargetedAction : public Action
    {
    public:
        TargetedAction(const Skill& skill, const Actor& actor, const Actor& target, Trajectory trajectory);
        const Actor& target;
    };

}   // ::Game

