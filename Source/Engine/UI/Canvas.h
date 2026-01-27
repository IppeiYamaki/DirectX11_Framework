/// @file   Canvas.h
/// @brief  UI要素を管理するCanvasクラス
#pragma once

#include <memory>
#include <vector>
#include <algorithm>

namespace Engine {

    class UIElement;
    class RenderSystem;

    /// @brief UI要素を管理するCanvas
    /// @note  UnityのCanvasのように、UI要素を画面手前に表示する機能を提供
    class Canvas final {
    public:
        Canvas() = default;
        ~Canvas();

        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  screenWidth 画面幅
        /// @param  screenHeight 画面高さ
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(float screenWidth, float screenHeight);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Element Management
        //============================================================

        /// @brief UI要素をキャンバスに追加
        /// @param element 追加するUI要素（所有権を移譲）
        /// @return 追加されたUI要素へのポインタ
        UIElement* AddElement(std::unique_ptr<UIElement> element);

        /// @brief  指定した型のUI要素を生成して追加
        /// @tparam T UIElementを継承した型
        /// @tparam Args コンストラクタ引数型
        /// @param  args コンストラクタ引数
        /// @return 生成されたUI要素へのポインタ
        template <class T, class... Args>
        [[nodiscard]] T* CreateElement(Args&&... args) {
            static_assert(std::is_base_of_v<UIElement, T>, "T must derive from UIElement");
            
            auto element = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = element.get();
            
            AddElement(std::move(element));
            return raw;
        }

        /// @brief UI要素を削除
        /// @param element 削除するUI要素
        void RemoveElement(UIElement* element);

        /// @brief 全UI要素を削除
        void ClearElements();

        /// @brief  UI要素数を取得
        /// @return UI要素数
        [[nodiscard]] std::size_t GetElementCount() const;

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

        /// @brief キャンバス内のUI要素を描画
        /// @param renderSystem RenderSystem参照
        void Render(RenderSystem* renderSystem);

        //============================================================
        // Screen
        //============================================================

        /// @brief 画面サイズを更新
        /// @param width 画面幅
        /// @param height 画面高さ
        /// @note  ウィンドウリサイズ時にApplication側から呼び出す必要がある
        void SetScreenSize(float width, float height);

        /// @brief  画面幅を取得
        /// @return 画面幅
        [[nodiscard]] float GetScreenWidth() const;

        /// @brief  画面高さを取得
        /// @return 画面高さ
        [[nodiscard]] float GetScreenHeight() const;

        //============================================================
        // State
        //============================================================

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        /// @brief 有効化
        void Enable();

        /// @brief 無効化
        void Disable();

        /// @brief  有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        //============================================================
        // Mouse Input
        //============================================================

        /// @brief マウスクリックを処理
        /// @param mouseX マウスX座標（スクリーン座標）
        /// @param mouseY マウスY座標（スクリーン座標）
        /// @return クリックがUI要素に当たった場合true
        bool HandleMouseClick(float mouseX, float mouseY);

        /// @brief 指定座標にあるUI要素を取得
        /// @param mouseX マウスX座標
        /// @param mouseY マウスY座標
        /// @return 座標上にあるUI要素（最前面のもの、なければnullptr）
        [[nodiscard]] UIElement* GetElementAt(float mouseX, float mouseY) const;

    private:
        /// @brief 描画順序でUI要素をソート
        void SortElements();

    private:
        std::vector<std::unique_ptr<UIElement>> m_elements; ///< UI要素のリスト
        float m_screenWidth = 1280.0f;                       ///< 画面幅
        float m_screenHeight = 720.0f;                       ///< 画面高さ
        bool m_isInitialized = false;                        ///< 初期化済みフラグ
        bool m_isEnabled = true;                             ///< 有効フラグ
        bool m_needsSort = false;                            ///< ソート必要フラグ
    };

} // namespace Engine
