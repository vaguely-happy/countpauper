#include "Game/Plan.h"
#include "Game/Move.h"
#include "UI/Avatar.h"
#include <numeric>
#include <GL/gl.h>

namespace Game
{

Plan::Plan() :
    Scenery(mesh)
{
}

Plan::Plan(Plan&& other) :
    Scenery(mesh),
    mesh(std::move(other.mesh))
{
}

Plan& Plan::operator=(Plan&& rhs)
{
    actions.clear();
    std::swap(actions, rhs.actions);
    std::swap(mesh, rhs.mesh);
    return *this;
}

void Plan::Render() const
{
    for(const auto& action: actions)
        action->Render();
}

std::string_view Plan::Name() const
{
    return "Plan";
}

std::string Plan::Description() const
{
    std::stringstream ss;
    for(const auto& act: actions)
    {
        if (ss.tellg())
        {
            ss << "\n";
        }
        ss << act->AP() << "AP: ";
        ss << act->Description();
    }
    return ss.str();
}

unsigned Plan::AP() const
{
    return std::accumulate(actions.begin(), actions.end(), 0, [](unsigned ap, const std::unique_ptr<Action>& a)
    {
        return ap + a->AP();
    });
}

void Plan::Execute()
{
   for(const auto& action: actions)
        action->Execute();
}

Plan Plan::Move(Avatar& actor, const Game& game, Engine::Position destination)
{
    Plan result;
    result.actions.emplace_back(std::move(std::make_unique<::Game::Move>(actor, game, destination)));
    return result;
}



}
