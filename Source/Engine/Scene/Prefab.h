/// @file   Prefab.h
/// @brief  Prefab基底インターフェース
#pragma once

namespace Game {
    struct SceneContext;
}

namespace Engine {
    using SceneContext = Game::SceneContext;
    class Entity;

    /// @brief Prefab基底インターフェース
    class PrefabBase {
    public:
        virtual ~PrefabBase() = default;
    };

    /// @brief SpawnDesc付きPrefabの共通インターフェース
    template<class TDesc>
    class Prefab : public PrefabBase {
    public:
        using SpawnDesc = TDesc;

        virtual ~Prefab() = default;

        /// @brief PrefabからEntityを生成
        /// @param ctx Scene利用コンテキスト情報
        /// @param desc 生成パラメータ
        /// @return 生成されたEntity
        virtual Entity* Spawn(SceneContext& ctx, const TDesc& desc) = 0;
    };

} // namespace Engine
