#include "Game/Attack.h"
#include "Geometry/Line.h"
#include "Utility/Random.h"
#include "UI/Avatar.h"
#include <GL/gl.h>

namespace Game
{

Attack::Attack(Actor& actor, Actor& target) :
    Action(actor),
    target(target)
{
}

void Attack::Render() const
{
    glColor3d(1.0, 1.0, 0.0);
    Engine::Line line(actor.GetAppearance().GetCoordinate(), target.GetAppearance().GetCoordinate());
    line += Engine::Vector::Z;
    line.Render();
}

// TODO: shoul;d still include map and be collected with more free functions or utility functions of actor

double HitChance(const Actor& actor, const Actor& target)
{
    auto hitScore = target.GetStats().Get(Stat::dodge) + target.GetStats().Get(Stat::block);    // TODO: front
    return 1.0;
}


void Attack::Execute(std::ostream& log) const
{
    actor.GetCounts().Cost(Stat::ap, AP());
    auto chance = HitChance(actor, target);
    if (chance < Engine::Random().Chance())
    {
        log << actor.GetAppearance().Name() << "misses " << target.GetAppearance().Name() << std::endl;  // TODO: this hit chance should be specified in miss, block, parry obstacle
        return;
    }

    auto damage =  actor.GetStats().Get(Stat::offense) - target.GetStats().Get(Stat::defense);
    if ( damage.Total() > 0 )
    {
        target.GetCounts().Cost(Stat::hp, damage.Total(), true);
        log << actor.GetAppearance().Name() << " deals " << damage.Description() << " damage to " << target.GetAppearance().Name() << std::endl;
    }
    else
    {
        log << actor.GetAppearance().Name() << " deals " << target.GetAppearance().Name() << " a glancing blow" << std::endl;
    }
}

unsigned Attack::AP() const
{
    return 1;
}

std::string Attack::Description() const
{
    return std::string("Attack ") + std::string(target.GetAppearance().Name());
}

}
