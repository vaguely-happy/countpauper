#include "File/Image.h"
#include "Game/Map.h"
#include "UI/Avatar.h"
#include "Game/Game.h"
#include "Game/Equipment.h"
#include "UI/Scene.h"
#include "UI/Application.h"
#include "UI/GameMessages.h"
#include "UI/WindowMessages.h"
#include "UI/Window.h"
#include "UI/Label.h"
#include "UI/HUD.h"

namespace Game
{

Game::Game(Engine::Scene& scene) :
    scene(scene),
    races(Race::Parse(Engine::LoadJson("data/race_stats.json"))),
    items(),
    map(Engine::Image("data/map20.png"))
{
    Creature::definition.Parse(Engine::LoadJson("data/creature.json"));
    Item::definition.Parse(Engine::LoadJson("data/item_stats.json"));
    items = ItemDatabase(Engine::LoadJson("data/items.json"));
    Engine::Application::Get().bus.Subscribe(*this,
    {
        Engine::MessageType<Selected>(),
        Engine::MessageType<Engine::ClickOn>(),
        Engine::MessageType<Engine::KeyPressed>()
    });
    scene.Add(map);
    scene.Add(plan);
    scene.GetCamera().Move(Engine::Coordinate(map.GetSize().x/2, -1, map.GetSize().z));

    Focus(Engine::Position(map.GetSize().x / 2, map.GetSize().y / 2, 0));
    auto& velglarn = avatars.emplace_back(std::move(std::make_unique<Avatar>("Velg'larn", races.at(0))));
    velglarn->Move(map, Engine::Position{3, 2});
    const auto& dagger = *items.Find("dagger");
    const ItemBonus* daggerMaterial = items.FindBonus(Restrictions({Restriction::material, Restriction::melee,dagger.GetMaterial()})).front();
    velglarn->GetEquipment().Equip(Equipment(dagger, {daggerMaterial}));

    scene.Add(*velglarn);
    auto& elgcaress = avatars.emplace_back(std::move(std::make_unique<Avatar>("Elg'caress", races.at(2))));
    elgcaress->Move(map, Engine::Position{5, 8});
    const auto& club = *items.Find("club");
    const ItemBonus* clubMaterial = items.FindBonus(Restrictions({Restriction::material, Restriction::melee,club.GetMaterial()})).at(3);
    elgcaress->GetEquipment().Equip(Equipment(club, {clubMaterial}));

    scene.Add(*elgcaress);
    avatars[turn]->Select(true);
}

const HeightMap& Game::GetMap() const
{
    return map;
}

bool Game::Obstacle(Engine::Position at, const Actor* except) const
{
    for(const auto& avatar : avatars)
    {
        if (avatar.get() == except)
            continue;
        if (avatar->Position() == at)
            return true;
    }
    // TODO: objects, size &  height (use Bounds)
    return false;
}

void Game::Focus(Engine::Position pos)
{
    Focus(map.GroundCoord(pos));
}

void Game::Focus(Engine::Coordinate coord)
{
    if (auto trackCamera = dynamic_cast<Engine::TrackingCamera*>(&scene.GetCamera()))
    {
        trackCamera->Track(coord);
    }
    else
    {
        scene.GetCamera().Face(coord);
    }
}

void Game::OnMessage(const Engine::Message& message)
{
    if (auto clickOn = message.Cast<Engine::ClickOn>())
    {
        auto target = dynamic_cast<const Avatar*>(clickOn->object);
        if (clickOn->object == &map)
        {
            auto mapSize = map.GetSize();
            Engine::Position desination(
                    clickOn->sub % mapSize.x,
                    clickOn->sub / mapSize.x,
                    0);
            plan = Plan::Move(Current(), *this, desination);
        }
        else if (target && target!=& Current())
        {
            plan = Plan::Attack(Current(), *this, const_cast<Avatar&>(*target));
        }
        auto lbl = Engine::Window::CurrentWindow()->GetHUD().Find<Engine::Label>("right_lbl");
        lbl->SetText(plan.Description());
        Changed();
    }
    else if (auto selected = message.Cast<Selected>())
    {
        if (selected->avatar)
        {
            Focus(selected->avatar->GetCoordinate());
            auto lbl = Engine::Window::CurrentWindow()->GetHUD().Find<Engine::Label>("left_lbl");
            lbl->SetText(selected->avatar->Sheet());
        }
        else
        {
             Focus(Engine::Position(map.GetSize().x / 2, map.GetSize().y / 2, 0));
        }
        Changed();
    } else if (auto key = message.Cast<Engine::KeyPressed>())
    {
        if (key->ascii == 13)
        {
            auto desc = plan.Execute();
            auto log = Engine::Window::CurrentWindow()->GetHUD().Find<Engine::Label>("log_lbl");
            log->AddText(desc);

            plan = Plan();
            if (Current().GetCounts().Available(Stat::ap) == 0)
                Next();
            else
                Current().GetCounts().Reset(Counter::Reset::action);
            Changed();
        }
    }
}

Avatar& Game::Current() const
{
    return *avatars[turn];
}

void Game::Next()
{
    Current().Select(false);
    turn++;
    if (turn>=avatars.size())
    {
        turn = 0;
        ++round;
    }
    Current().GetCounts().Reset(Counter::Reset::turn);
    Current().Select(true);
}

void Game::Changed()
{
        Engine::Application::Get().bus.Post(Engine::Redraw(&Current()));
}

}
