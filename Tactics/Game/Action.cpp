#include "stdafx.h"
#include "Action.h"
#include "Move.h"
#include "Attack.h"
#include "Game.h"
#include "Game/Skills.h"
#include "State.h"
#include "Actor.h"

namespace Game
{
    Action::Action(const Skill& skill, const Actor& actor) :
        skill(skill),
        actor(actor)
    {
    }


	std::wstring Action::Description() const
	{
		return skill.name;
	}
	
	TargetedAction::TargetedAction(const Skill& skill, const Actor& actor, const Target& target) :
        Action(skill, actor),
        target(target)
    {
    }

	Score TargetedAction::Chance(const IGame& game) const
	{
		State attacker = game.Get(actor);
		auto targetActor = dynamic_cast<const Actor*>(&target);
		if (targetActor)
		{
			State victim = game.Get(*targetActor);
			return attacker.Chance(skill, victim);
		}
		else
		{
			return attacker.Chance(skill);
		}
	}

	Position TargetedAction::GetPosition() const
	{
		return target.GetPosition();
	}

	std::wstring TargetedAction::Description() const
	{
		return skill.name + L"@" +target.Description();
	}

	AimedAction::AimedAction(const Skill& skill, const Actor& actor, const Target& target, const Body::Part& part) :
		TargetedAction(skill, actor, target),
		part(part)
	{

	}

	std::map<unsigned, std::function<Action*(const State& state, const Game& game)>> Action::keymap =
    {};
    /*
        { VK_UP, [](const State&, const Game& game){ return new South(*game.ActiveActor()); } },
        { VK_RIGHT, [](const State&, const Game& game){ return new East(*game.ActiveActor()); } },
        { VK_DOWN, [](const State&, const Game&  game){ return new North(*game.ActiveActor()); } },
        { VK_LEFT, [](const State&, const Game& game){ return new West(*game.ActiveActor()); } },
        { VK_SPACE, [](const State& state, const Game& game)
        {
            auto& skill = game.skills.front();
            const auto& actor = *game.ActiveActor();
            auto targets = game.FindTargetsInRange(state, skill);
            if (targets.empty())
                return (Action*)nullptr;
            return skill.CreateAction(actor, *targets.front());
        } },
    };
    */
} // ::Game