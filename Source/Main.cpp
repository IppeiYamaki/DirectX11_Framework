#include <Windows.h>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/ApplicationSettings.h"

#include "Game/GameMain.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    Engine::Logger::Initialize();

    Engine::ApplicationSettings settings{};
    settings.m_hInstance = hInstance;
    settings.m_cmdShow = cmdShow;
    settings.m_title = L"DirectX11_Framework";
    settings.m_width = 1280;
    settings.m_height = 720;
    settings.m_isVSyncEnabled = true;

    Engine::Application app;
    if (!app.Initialize(settings)) {
        Engine::Logger::Error("Application Initialize failed.");
        app.Finalize();
        Engine::Logger::Finalize();
        return -1;
    }

    Game::GameMain game;
    const int result = app.Run(game); // game.Initialize/Finalize ‚Í Run “à‚ÅŒÄ‚Î‚ê‚é

    app.Finalize();
    Engine::Logger::Finalize();
    return result;
}
