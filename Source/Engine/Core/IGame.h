#pragma once

namespace Engine {

    class Application;

    /**
     * @brief ゲーム側（GameSystem）と Engine（Application）を分離するためのインターフェース
     *
     * - Engine は「どう回すか」を担当（Window/Time/Render/World を持つ）
     * - Game は「何を作るか」を担当（初期Entity生成、シーン構築、ゲーム進行）
     */
    class IGame {
    public:
        virtual ~IGame() = default;

        /**
         * @brief ゲーム初期化
         * @param app Engine側アプリケーション（World/Assets等へアクセスする入口）
         * @return 成功なら true
         */
        virtual bool Initialize(Application& app) = 0;

        /**
         * @brief ゲーム終了処理（安全に複数回呼ばれても壊れない設計を推奨）
         */
        virtual void Finalize() = 0;

        /**
         * @brief 毎フレーム更新
         * @param deltaTime 秒
         */
        virtual void Update(float deltaTime) = 0;

        /**
         * @brief 毎フレーム描画（描画要求の送出）
         */
        virtual void Draw() = 0;
    };

} // namespace Engine
