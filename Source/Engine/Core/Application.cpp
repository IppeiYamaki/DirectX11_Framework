#include "Application.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Logger.h"

// ここは “実装側” で依存を持つ（Coreヘッダを軽く保つため）
// ※ これらのファイルは次に Platform/Graphics/Scene 側で作成する想定です
#include "Engine/Platform/Window.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"

#include "Engine/Scene/World.h"

namespace Engine {

    Application::~Application() = default;

    bool Application::Initialize(const ApplicationSettings& settings) {
        if (m_isInitialized) return true;

        m_settings = settings;
        if (!m_settings.IsValid()) {
            Logger::Error("ApplicationSettings is invalid.");
            return false;
        }

        m_time.Initialize();

        // 生成（Create）と初期化（Initialize）は混ぜない方針だが、
        // ここでは「所有物を作ってInitializeを呼ぶ」のは Application の責務としてOK。
        m_window = std::make_unique<Window>();
        m_graphicsDevice = std::make_unique<GraphicsDevice>();
        m_renderSystem = std::make_unique<RenderSystem>();
        m_world = std::make_unique<World>();

        // --- Window ---
        // 想定: bool Window::Initialize(HINSTANCE, int cmdShow, const std::wstring& title, int w, int h, bool resizable);
        if (!m_window->Initialize(
            m_settings.m_hInstance,
            m_settings.m_cmdShow,
            m_settings.m_title,
            m_settings.m_width,
            m_settings.m_height,
            m_settings.m_isResizable
        )) {
            Logger::Error("Window Initialize failed.");
            Finalize();
            return false;
        }

        // --- GraphicsDevice ---
        // 想定: bool GraphicsDevice::Initialize(HWND hwnd, int w, int h, bool vsync);
        if (!m_graphicsDevice->Initialize(
            m_window->GetHwnd(),
            m_settings.m_width,
            m_settings.m_height,
            m_settings.m_isVSyncEnabled
        )) {
            Logger::Error("GraphicsDevice Initialize failed.");
            Finalize();
            return false;
        }

        // --- RenderSystem ---
        // 想定: bool RenderSystem::Initialize(GraphicsDevice& device);
        if (!m_renderSystem->Initialize(*m_graphicsDevice)) {
            Logger::Error("RenderSystem Initialize failed.");
            Finalize();
            return false;
        }

        // --- World ---
        if (!m_world->Initialize()) {
            Logger::Error("World Initialize failed.");
            Finalize();
            return false;
        }

        m_isQuitRequested = false;
        m_isInitialized = true;

        Logger::Info("Application initialized.");
        return true;
    }

    void Application::Finalize() {
        if (!m_isInitialized) {
            // 未初期化でも安全に複数回呼べる設計
            m_world.reset();
            m_renderSystem.reset();
            m_graphicsDevice.reset();
            m_window.reset();
            m_time.Finalize();
            return;
        }

        Logger::Info("Application finalize start.");

        if (m_world) {
            m_world->Finalize();
        }
        if (m_renderSystem) {
            m_renderSystem->Finalize();
        }
        if (m_graphicsDevice) {
            m_graphicsDevice->Finalize();
        }
        if (m_window) {
            m_window->Finalize();
        }

        m_world.reset();
        m_renderSystem.reset();
        m_graphicsDevice.reset();
        m_window.reset();

        m_time.Finalize();

        m_isInitialized = false;
        m_isQuitRequested = false;

        Logger::Info("Application finalized.");
    }

    void Application::Reset() {
        if (!m_isInitialized) return;

        m_time.Reset();
        if (m_world) m_world->Reset();
        // RenderSystem/GraphicsDevice の Reset は必要になったら追加
    }

    int Application::Run(IGame& game) {
        ASSERT(m_isInitialized);

        // Game initialize（World/Assetsなどは app 経由で渡す）
        if (!game.Initialize(*this)) {
            Logger::Error("Game Initialize failed in Application::Run.");
            return -1;
        }

        // メインループ
        while (!m_isQuitRequested) {
            // 想定: bool Window::PumpMessages(); （false で終了）
            if (!m_window->PumpMessages()) {
                break;
            }

            m_time.Tick();
            const float deltaTime = m_time.GetDeltaTime();

            // 更新順：Game → World（生成・予約などを先にやれる）
            game.Update(deltaTime);

            if (m_world) {
                m_world->Update(deltaTime);
                m_world->LateUpdate(deltaTime); // World側に無いなら後で調整
            }

            // Draw要求収集
            if (m_world) {
                m_world->Draw();
            }
            game.Draw();

            // 実描画（Clear → Queue処理 → Present）
            // 想定: void RenderSystem::Draw(World& world);
            m_renderSystem->Draw(*m_world);
        }

        game.Finalize();
        return 0;
    }

    //============================================================
    // Control
    //============================================================

    void Application::RequestQuit() {
        m_isQuitRequested = true;
    }

    bool Application::IsQuitRequested() const {
        return m_isQuitRequested;
    }

    bool Application::IsInitialized() const {
        return m_isInitialized;
    }

    //============================================================
    // Getters
    //============================================================

    const ApplicationSettings& Application::GetSettings() const {
        return m_settings;
    }

    const Time& Application::GetTime() const {
        return m_time;
    }

    World* Application::GetWorld() {
        return m_world.get();
    }

    const World* Application::GetWorld() const {
        return m_world.get();
    }

    Window* Application::GetWindow() {
        return m_window.get();
    }

    const Window* Application::GetWindow() const {
        return m_window.get();
    }

    GraphicsDevice* Application::GetGraphicsDevice() {
        return m_graphicsDevice.get();
    }

    const GraphicsDevice* Application::GetGraphicsDevice() const {
        return m_graphicsDevice.get();
    }

    RenderSystem* Application::GetRenderSystem() {
        return m_renderSystem.get();
    }

    const RenderSystem* Application::GetRenderSystem() const {
        return m_renderSystem.get();
    }

} // namespace Engine
