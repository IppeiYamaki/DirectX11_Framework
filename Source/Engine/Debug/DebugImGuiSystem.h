/// @file   DebugImGuiSystem.h
/// @brief  ImGuiベースのデバッグUI管理システム
#pragma once

#include <d3d11.h>
#include <Windows.h>

namespace Engine {

    class GraphicsDevice;
    class Window;
    class Scene;
    class LightSystem;
    class DebugVisualizationSystem;

    /// @brief ImGuiベースのデバッグUI管理システム
    /// @note  開発者モード時にGameObject/Lightの各種プロパティを編集可能にする
    class DebugImGuiSystem final {
    public:
        DebugImGuiSystem() = default;
        ~DebugImGuiSystem();

        DebugImGuiSystem(const DebugImGuiSystem&) = delete;
        DebugImGuiSystem& operator=(const DebugImGuiSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  hwnd ウィンドウハンドル
        /// @param  device D3D11デバイス
        /// @param  context D3D11デバイスコンテキスト
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Frame
        //============================================================

        /// @brief フレーム開始（ImGui NewFrame）
        void BeginFrame();

        /// @brief  デバッグUIを描画（ImGui内部のUIウィジェット構築）
        /// @param  scene Scene参照
        /// @param  lightSystem LightSystem参照
        /// @param  debugVisualization デバッグ可視化システム参照
        void Update(Scene* scene, LightSystem* lightSystem, DebugVisualizationSystem* debugVisualization);

        /// @brief 描画実行（ImGui Render）
        void Render();

        //============================================================
        // Mode Control
        //============================================================

        /// @brief  デバッグUIを有効/無効切替
        /// @param  enabled 有効にする場合true
        void SetEnabled(bool enabled);

        /// @brief  デバッグUIが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        /// @brief  デバッグUIをトグル
        void ToggleEnabled();

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        //============================================================
        // Win32 Message Handler
        //============================================================

        /// @brief  Win32メッセージをImGuiに転送
        /// @param  hwnd ウィンドウハンドル
        /// @param  msg メッセージ
        /// @param  wParam WPARAM
        /// @param  lParam LPARAM
        /// @return メッセージ処理済みなら0以外
        static LRESULT WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        /// @brief  シーン階層ウィンドウを描画
        /// @param  scene Scene参照
        void DrawSceneHierarchyWindow(Scene* scene);

        /// @brief  インスペクターウィンドウを描画
        /// @param  scene Scene参照
        void DrawInspectorWindow(Scene* scene);

        /// @brief  ライト管理ウィンドウを描画
        /// @param  lightSystem LightSystem参照
        void DrawLightManagerWindow(LightSystem* lightSystem);

        /// @brief  デバッグ設定ウィンドウを描画
        /// @param  debugVisualization デバッグ可視化システム参照
        void DrawDebugSettingsWindow(DebugVisualizationSystem* debugVisualization);

    private:
        bool m_isInitialized = false;      ///< 初期化済みフラグ
        bool m_isEnabled = false;          ///< デバッグUI有効フラグ

        // 選択中のGameObjectインデックス（-1は未選択）
        int m_selectedObjectIndex = -1;

        // 選択中のライトインデックス（-1は未選択）
        int m_selectedLightIndex = -1;

        // ウィンドウ表示フラグ
        bool m_showSceneHierarchy = true;   ///< シーン階層ウィンドウ表示
        bool m_showInspector = true;        ///< インスペクターウィンドウ表示
        bool m_showLightManager = true;     ///< ライト管理ウィンドウ表示
        bool m_showDebugSettings = true;    ///< デバッグ設定ウィンドウ表示
    };

} // namespace Engine
