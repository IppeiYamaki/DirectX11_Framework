#pragma once

#include <memory>
#include <string>

struct ID3D11Device;

namespace Engine {
    class AssetManager;
    class Model;

    /**
     * @brief OBJ(+MTL)を読み込んで Model を作る
     * - usemtl / mtllib / map_Kd 対応
     * - 頂点レイアウトは DefaultVS (Pos/Normal/Color/UV) に合わせる
     */
    std::shared_ptr<Model> LoadObjModel(ID3D11Device* device, AssetManager& assets, const std::wstring& objPath);
} // namespace Engine
