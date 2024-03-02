#include <GL/glut.h>
#include "UI/Window.h"
#include "UI/Application.h"
#include "UI/Logging.h"
#include "Game/Game.h"

#include "Game/Race.h"
#include "Game/Creature.h"

#include "Utility/Singleton.h"
static constexpr Engine::Logging::Level MainLogging = Engine::Logging::Warning;

int main(int argc, char**argv)
{
    try
    {
        Engine::Singleton<Engine::Application> app(argc, argv);
        Engine::Window window;

        window.SetTitle("Tactics III");
        Game::Game game(window.GetScene());
        Engine::Label lbl("User\nInterface\nLabel");
        window.GetHUD().push_back(&lbl);
        app->Run();
    }
    catch(const std::exception& ex)
    {
        Engine::Logging::Log<MainLogging, Engine::Logging::Fatal>("Exception: %s", ex.what());
        return 1;
    }
    return 0;
}
