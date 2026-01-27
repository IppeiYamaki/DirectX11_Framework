#pragma once

#include <memory>

#include "Engine/Core/ApplicationSettings.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/IGame.h"

#include "Engine/Platform/Window.h"
#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/World.h"

namespace Engine {

    class Scene;

    /**
     * @brief Engine �̎i�ߓ��i���������� + ���C�����[�v�Œ�j
     *
     * - Window / GraphicsDevice / RenderSystem / World / Time �����L����
     * - ���[�v�͂����ŉ񂷁iWinMain �𔖂�����j
     */
    class Application final {
    public:
        Application() = default;
        ~Application(); // unique_ptr �̂��� cpp �Œ�`�i�O���錾�^�̊��S�^���K�v�j

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(const ApplicationSettings& settings);
        void Finalize();
        void Reset();

        //============================================================
        // Main loop
        //============================================================
        int Run(IGame& game);

        //============================================================
        // Control
        //============================================================
        void RequestQuit();
        bool IsQuitRequested() const;
        bool IsInitialized() const;

        //============================================================
        // Getters�i��const�Q��Get�͌����֎~ �� �|�C���^�ŕԂ��j
        //============================================================
        const ApplicationSettings& GetSettings() const;

        const Time& GetTime() const;

              World* GetWorld();
        const World* GetWorld() const;

        // Engine�����p�i�Q�[������͐G��Ȃ��z��B�K�v�Ȃ���J�͈͂��������j
              Window* GetWindow();
        const Window* GetWindow() const;

              GraphicsDevice* GetGraphicsDevice();
        const GraphicsDevice* GetGraphicsDevice() const;

              RenderSystem* GetRenderSystem();
        const RenderSystem* GetRenderSystem() const;

              Scene* GetScene();
        const Scene* GetScene() const;

    private:
		bool                            m_isInitialized     = false;    // �������ς݃t���O
        bool                            m_isQuitRequested   = false;    // �I���v���t���O

		ApplicationSettings             m_settings{};                   // �A�v���P�[�V�����ݒ�
		Time                            m_time{};                       // ���ԊǗ�

		std::unique_ptr<Window>         m_window;                       // �E�B���h�E�Ǘ�
		std::unique_ptr<GraphicsDevice> m_graphicsDevice;               // �O���t�B�b�N�X�f�o�C�X�Ǘ�
		std::unique_ptr<RenderSystem>   m_renderSystem;                 // �����_�����O�Ǘ�
		std::unique_ptr<World>          m_world;                        // �V�[���Ǘ�
    };

} // namespace Engine
