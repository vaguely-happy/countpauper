#include "stdafx.h"
#include <gl/GL.h>
#include "Engine/Text.h"
#include "Attack.h"
#include "Game.h"
#include "Actor.h"
#include "Skills.h"

namespace Game
{

Attack::Attack(const Actor& actor, const Actor& target, const Skill& skill) :
    TargetedAction(skill, actor, target)
{
}

std::wstring Attack::Description() const
{
    return skill.name;
}

std::unique_ptr<GameState> Attack::Act(const IGame& game) const
{
    State attacker = game.Get(actor);
    State victim(game.Get(target));
    if (!attacker.IsPossible(skill, victim))
        return nullptr;
    auto ret = std::make_unique<GameState>(game);
    auto damage = attacker.AttackDamage(skill) - victim.Mitigation();
    attacker.mp -= skill.mp;
    victim.body.Hurt(AttackVector({ Plane::All, 0 }), damage.Wound(actor.Description()));

    ret->Adjust(actor, attacker);
    ret->Adjust(target, victim);
    return std::move(ret);
}

void Attack::Render(const State& state) const
{
    glColor4ub(255, 0, 0, 255);
    glPushMatrix();
    glTranslatef(float(state.position.x) + 0.5f, 0.5f, float(state.position.y) + 0.5f);
    Position v = target.GetPosition() - state.position;
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(float(v.x), 0, float(v.y));
    glEnd();
    Engine::Font::system.Select();
    Engine::glText(skill.name);
    glPopMatrix();
}



} // ::Game