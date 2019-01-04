#include "stdafx.h"
#include <gl/GL.h>
#include "Affect.h"
#include "Game.h"
#include "Actor.h"
#include "Skills.h"

namespace Game
{

Affect::Affect(const Actor& actor, const Actor& target, const Skill& skill, Trajectory trajectory) :
    TargetedAction(skill, actor, target, trajectory)
{
}

std::wstring Affect::Description() const
{
    return skill.name;
}

std::unique_ptr<GameState> Affect::Act(const IGame& game) const
{
    State attacker = game.Get(actor);
    State victim(game.Get(target));
    if (!attacker.IsPossible(skill, victim))
        return nullptr;
    auto ret = std::make_unique<GameState>(game, actor);
    attacker.mp -= skill.mp;
    // TODO effect
    ret->Adjust(actor, attacker);
    ret->Adjust(target, victim);
    return std::move(ret);
}

std::unique_ptr<GameState> Affect::Fail(const IGame& game) const
{
    State attacker = game.Get(actor);
    State victim(game.Get(target));
    if (!attacker.IsPossible(skill, victim))
        return nullptr;
    auto ret = std::make_unique<GameState>(game, actor);
    attacker.mp -= skill.mp;
    ret->Adjust(actor, attacker);
    return std::move(ret);
}

void Affect::Render(const State& state) const
{
    glColor4ub(0, 255, 0, 255);
    glPushMatrix();
    glTranslatef(float(state.position.x) + 0.5f, 0.5f, float(state.position.y) + 0.5f);
    Position v = target.GetPosition() - state.position;
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(float(v.x), 0, float(v.y));
    glEnd();

    glRasterPos3i(0, 0, 1); // set start position
    glListBase(1000);
    std::string text(skill.name.begin(), skill.name.end());
    glCallLists(text.size(), GL_UNSIGNED_BYTE, text.c_str());

    glPopMatrix();


}



} // ::Game