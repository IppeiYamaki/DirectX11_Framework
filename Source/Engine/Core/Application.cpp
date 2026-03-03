#include "Application.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Logger.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"

#include "Engine/Scene/World.h"
#include "Engine/Scene/Scene.h"

namespace Engine {

    Application::~Application() = default;

    bool Application::Initialize(const ApplicationSettings& settings) {
        if (m_isInitialized) return true;

		//============================================================
		// subsystem �̏�����
        //============================================================
        m_settings = settings;
        if (!m_settings.IsValid()) {
            Logger::Error("ApplicationSettings is invalid.");
            return false;
        }

        m_time.Initialize();

        //============================================================
		// subsystem �̐���
		//============================================================
		// ���ԂɈˑ��֌W������̂Ő������̕ύX�s��
		// Window -> Input -> GraphicsDevice -> RenderSystem -> World
        m_window            = std::make_unique<Window>();
        m_graphicsDevice    = std::make_unique<GraphicsDevice>();
        m_renderSystem      = std::make_unique<RenderSystem>();
        m_world             = std::make_unique<World>();

        //============================================================
		// subsystem �̏�����
        //============================================================

		// window �̏�����
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

		// input �̏�����
        Input::Initialize(m_window->GetHwnd());


		// graphicsDevice �̏�����
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

		// renderSystem �̏�����
        if (!m_renderSystem->Initialize(*m_graphicsDevice)) {
            Logger::Error("RenderSystem Initialize failed.");
            Finalize();
            return false;
        }

		// world �̏�����
        if (!m_world->Initialize()) {
            Logger::Error("World Initialize failed.");
            Finalize();
            return false;
        }

		m_isQuitRequested = false;      // �I���v���t���O�N���A
		m_isInitialized = true; 		// �������ς݃t���O�Z�b�g

        Logger::Info("Application initialized.");
        return true;
    }

    void Application::Finalize() {
        if (!m_isInitialized) {

            m_world.reset();
            m_renderSystem.reset();
            m_graphicsDevice.reset();
            Input::Finalize();
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

    }

    int Application::Run(IGame& game) {
        ASSERT(m_isInitialized);


        if (!game.Initialize(*this)) {
            Logger::Error("Game Initialize failed in Application::Run.");
            return -1;
        }


        while (!m_isQuitRequested) {
            Input::BeginFrame();


            if (!m_window->PumpMessages()) {
                break;
            }

            m_time.Tick();
            const float deltaTime = m_time.GetDeltaTime();


            game.Update(deltaTime);

            if (m_world) {
                m_world->Update(deltaTime);
                m_world->LateUpdate(deltaTime); // 
            }

            // 
            if (m_world) {
                m_world->Draw();
            }
            game.Draw();

            // UI描画はRenderSystem::Draw内で3D描画後、Present前に実行される
            // フェード描画はUI描画の後、Present前に実行される
            // デバッグ描画は3Dオブジェクト描画後、UI描画前に実行される（Debug Layerで最上位に表示）
            // ImGuiデバッグUIはフェード描画後、Present直前に実行される
            m_renderSystem->Draw(*m_world, game.GetCanvas(), game.GetFadeSystem(),
                                 game.GetDebugVisualizationSystem(), GetScene(), game.GetLightSystem(),
                                 game.GetDebugImGuiSystem());
        }

        game.Finalize();
        return 0;
    }

    //============================================================
    // �I���v��
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

    Scene* Application::GetScene() {
        return m_world ? m_world->GetScene() : nullptr;
    }

    const Scene* Application::GetScene() const {
        return m_world ? m_world->GetScene() : nullptr;
    }

} // namespace Engine
