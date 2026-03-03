/// @file   Room_Corridor.h
/// @brief  デフォルト廊下クラス
#pragma once

#include "Game/Gameplay/RoomInstance.h"
#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief デフォルト廊下クラス
    /// @note  タイルマップのデフォルトタイルとして使用される廊下
    /// @note  ビジュアルオブジェクト(m_visualObject)はSceneが所有・管理する。
    ///        Room_Corridorはビジュアルへのポインタを保持するのみで、破棄責任はSceneにある。
    class Room_Corridor : public RoomInstance {
    public:
        /// @brief コンストラクタ
        Room_Corridor();

        /// @brief デストラクタ
        ~Room_Corridor() override = default;

        // コピー禁止
        Room_Corridor(const Room_Corridor&) = delete;
        Room_Corridor& operator=(const Room_Corridor&) = delete;

        //============================================================
        // ビジュアル管理
        //============================================================

        /// @brief ビジュアルを生成
        /// @param ctx シーンコンテキスト
        /// @param worldPosition ワールド座標での配置位置
        /// @return 生成成功ならtrue
        bool CreateVisual(Engine::SceneContext& ctx, const Engine::Vector3& worldPosition);

        /// @brief ビジュアルオブジェクトを設定
        /// @param visualObject ビジュアル用のGameObject
        void SetVisualObject(Engine::GameObject* visualObject);

        /// @brief ビジュアルオブジェクトを取得
        /// @return ビジュアル用のGameObject
        [[nodiscard]] Engine::GameObject* GetVisualObject() const;

        /// @brief ビジュアルの位置を更新
        /// @param worldPosition 新しいワールド座標
        void UpdateVisualPosition(const Engine::Vector3& worldPosition);

        //============================================================
        // RoomInstance オーバーライド
        //============================================================

        /// @brief キャラクターが部屋に入った時の効果（廊下は効果なし）
        /// @param character 入ったキャラクター
        void OnCharacterEnter(CharacterInstance* character) override;

        /// @brief 削除可能か確認（廊下は削除可能）
        /// @return 常にtrue
        [[nodiscard]] bool CanBeRemoved() const override;

    private:
        Engine::GameObject* m_visualObject = nullptr;  ///< ビジュアル用のGameObject
    };

} // namespace Game
