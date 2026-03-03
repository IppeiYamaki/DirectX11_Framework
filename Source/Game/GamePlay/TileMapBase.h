/// @file   TileMapBase.h
/// @brief  タイルマップ管理の基底クラス
#pragma once

#include "Game/Gameplay/GridPosition.h"
#include "Game/Gameplay/TileCell.h"
#include "Game/Gameplay/RoomInstance.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace Game {

    class RoomInstance;
    class TrapInstance;
    class CharacterInstance;

    /// @brief タイルマップ管理の基底クラス
    /// @note  全タイルの状態を一元管理し、部屋・トラップ・キャラクターの操作を提供
    class TileMapBase {
    public:
        /// @brief コンストラクタ
        /// @param width マップの幅（タイル数）
        /// @param height マップの高さ（タイル数）
        /// @param tileSize タイルの大きさ（ワールド座標）
        TileMapBase(int width = 10, int height = 10, float tileSize = 1.0f);

        /// @brief デストラクタ
        virtual ~TileMapBase() = default;

        // コピー禁止
        TileMapBase(const TileMapBase&) = delete;
        TileMapBase& operator=(const TileMapBase&) = delete;

        //============================================================
        // マップ初期化
        //============================================================

        /// @brief マップを初期化
        /// @param width マップの幅（タイル数）
        /// @param height マップの高さ（タイル数）
        void Initialize(int width, int height);

        /// @brief マップをクリア
        void Clear();

        //============================================================
        // マップ情報取得
        //============================================================

        /// @brief マップの幅を取得
        /// @return マップの幅（タイル数）
        [[nodiscard]] int GetWidth() const;

        /// @brief マップの高さを取得
        /// @return マップの高さ（タイル数）
        [[nodiscard]] int GetHeight() const;

        /// @brief タイルサイズを取得
        /// @return タイルサイズ（ワールド座標）
        [[nodiscard]] float GetTileSize() const;

        /// @brief タイルサイズを設定
        /// @param size タイルサイズ（ワールド座標）
        void SetTileSize(float size);

        /// @brief 有効な位置か確認
        /// @param position グリッド位置
        /// @return 有効な位置ならtrue
        [[nodiscard]] bool IsValidPosition(const GridPosition& position) const;

        //============================================================
        // タイル取得
        //============================================================

        /// @brief タイルを取得
        /// @param position グリッド位置
        /// @return タイルへのポインタ（無効な位置ならnullptr）
        [[nodiscard]] TileCell* GetTile(const GridPosition& position);

        /// @brief タイルを取得（const版）
        /// @param position グリッド位置
        /// @return タイルへのconstポインタ（無効な位置ならnullptr）
        [[nodiscard]] const TileCell* GetTile(const GridPosition& position) const;

        /// @brief タイルを取得（x, y座標）
        /// @param x X座標
        /// @param y Y座標
        /// @return タイルへのポインタ（無効な位置ならnullptr）
        [[nodiscard]] TileCell* GetTile(int x, int y);

        /// @brief タイルを取得（x, y座標、const版）
        /// @param x X座標
        /// @param y Y座標
        /// @return タイルへのconstポインタ（無効な位置ならnullptr）
        [[nodiscard]] const TileCell* GetTile(int x, int y) const;

        //============================================================
        // 部屋管理
        //============================================================

        /// @brief 部屋を配置
        /// @param position グリッド位置
        /// @param room 配置する部屋
        /// @return 配置成功ならtrue
        bool PlaceRoom(const GridPosition& position, std::shared_ptr<RoomInstance> room);

        /// @brief 部屋を削除
        /// @param position グリッド位置
        /// @return 削除成功ならtrue
        bool RemoveRoom(const GridPosition& position);

        /// @brief 部屋を取得
        /// @param position グリッド位置
        /// @return 部屋へのポインタ（未設定ならnullptr）
        [[nodiscard]] RoomInstance* GetRoom(const GridPosition& position) const;

        /// @brief 部屋を移動
        /// @param from 移動元
        /// @param to 移動先
        /// @return 移動成功ならtrue
        bool MoveRoom(const GridPosition& from, const GridPosition& to);

        /// @brief 指定した種類の部屋を持つタイル位置を取得
        /// @param type 部屋の種類
        /// @return タイル位置のベクター
        [[nodiscard]] std::vector<GridPosition> GetTilesWithRoomType(RoomType type) const;

        /// @brief 全ての部屋を持つタイル位置を取得
        /// @return タイル位置のベクター
        [[nodiscard]] std::vector<GridPosition> GetTilesWithRooms() const;

        //============================================================
        // トラップ管理
        //============================================================

        /// @brief トラップを追加
        /// @param position グリッド位置
        /// @param trap 追加するトラップ
        /// @return 追加成功ならtrue
        bool AddTrap(const GridPosition& position, std::shared_ptr<TrapInstance> trap);

        /// @brief トラップを削除
        /// @param position グリッド位置
        /// @param trap 削除するトラップ
        /// @return 削除成功ならtrue
        bool RemoveTrap(const GridPosition& position, TrapInstance* trap);

        /// @brief 指定位置の全トラップを削除
        /// @param position グリッド位置
        void ClearTraps(const GridPosition& position);

        /// @brief トラップを持つタイル位置を取得
        /// @return タイル位置のベクター
        [[nodiscard]] std::vector<GridPosition> GetTilesWithTraps() const;

        //============================================================
        // キャラクター移動処理
        //============================================================

        /// @brief キャラクターをタイルに移動
        /// @param character 移動するキャラクター
        /// @param from 移動元（nullptrなら新規入場）
        /// @param to 移動先
        /// @return 移動成功ならtrue
        bool MoveCharacter(CharacterInstance* character, 
                          const GridPosition* from, 
                          const GridPosition& to);

        /// @brief キャラクターの入室処理を実行
        /// @param character 入室するキャラクター
        /// @param position 入室位置
        void ProcessCharacterEnter(CharacterInstance* character, const GridPosition& position);

        /// @brief キャラクターの退室処理を実行
        /// @param character 退室するキャラクター
        /// @param position 退室位置
        void ProcessCharacterExit(CharacterInstance* character, const GridPosition& position);

        //============================================================
        // 座標変換
        //============================================================

        /// @brief グリッド座標からワールド座標に変換
        /// @param position グリッド位置
        /// @return ワールド座標
        [[nodiscard]] Engine::Vector3 GridToWorld(const GridPosition& position) const;

        /// @brief ワールド座標からグリッド座標に変換
        /// @param worldPos ワールド座標
        /// @return グリッド位置
        [[nodiscard]] GridPosition WorldToGrid(const Engine::Vector3& worldPos) const;

        /// @brief グリッド座標からワールド座標に変換（2D）
        /// @param position グリッド位置
        /// @return ワールド座標（2D）
        [[nodiscard]] Engine::Vector2 GridToWorld2D(const GridPosition& position) const;

        //============================================================
        // 隣接タイル取得
        //============================================================

        /// @brief 隣接タイルを取得（4方向）
        /// @param position 中心位置
        /// @return 隣接タイル位置のベクター
        [[nodiscard]] std::vector<GridPosition> GetAdjacentPositions(const GridPosition& position) const;

        /// @brief 通過可能な隣接タイルを取得
        /// @param position 中心位置
        /// @return 通過可能な隣接タイル位置のベクター
        [[nodiscard]] std::vector<GridPosition> GetPassableAdjacentPositions(const GridPosition& position) const;

        //============================================================
        // ターン処理
        //============================================================

        /// @brief ターン終了時の処理
        void OnTurnEnd();

        //============================================================
        // イテレーション
        //============================================================

        /// @brief 全タイルに対して処理を実行
        /// @param func 実行する関数（TileCell&を受け取る）
        void ForEachTile(const std::function<void(TileCell&)>& func);

        /// @brief 全タイルに対して処理を実行（const版）
        /// @param func 実行する関数（const TileCell&を受け取る）
        void ForEachTile(const std::function<void(const TileCell&)>& func) const;

        //============================================================
        // デバッグ
        //============================================================

        /// @brief マップの状態をログ出力
        void DebugLog() const;

        //============================================================
        // マップ原点管理
        //============================================================

        /// @brief マップ全体の原点を設定
        /// @param origin マップの原点座標
        void SetMapOrigin(const Engine::Vector3& origin);

        /// @brief マップの原点を取得
        /// @return マップの原点座標
        [[nodiscard]] const Engine::Vector3& GetMapOrigin() const;

        /// @brief グリッド座標をワールド座標に変換（マップ原点を考慮）
        /// @param gridPos グリッド位置
        /// @return ワールド座標
        [[nodiscard]] Engine::Vector3 GridToWorldPosition(const GridPosition& gridPos) const;

    protected:
        /// @brief タイルを追加
        /// @param position グリッド位置
        void AddTile(const GridPosition& position);

    private:
        int m_width = 10;                                       ///< マップの幅
        int m_height = 10;                                      ///< マップの高さ
        float m_tileSize = 1.0f;                                ///< タイルサイズ
        std::unordered_map<GridPosition, TileCell> m_tiles;     ///< タイル格納マップ
        Engine::Vector3 m_mapOrigin{0.0f, 0.0f, 0.0f};          ///< マップ原点
    };

} // namespace Game
