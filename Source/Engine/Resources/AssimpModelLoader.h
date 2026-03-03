#pragma once

#include <memory>
#include <string>

struct ID3D11Device;

namespace Engine {
    class AssetManager;
    class Model;

    /// @brief サポートされているモデル形式かどうかを確認
    /// @param filePath ファイルパス（ワイド文字列）
    /// @return サポートされていれば true
    bool IsSupportedModelFormat(const std::wstring& filePath);

    /// @brief Assimp を使用してモデルをロード（OBJ/FBX/GLTF/DAE など対応）
    /// @brief aiProcess_Triangulate で三角形に分割
    /// @brief aiProcess_ConvertToLeftHanded で DirectX 用左手座標系に変換
    /// @brief aiProcess_GenSmoothNormals で法線を生成
    /// @brief aiProcess_CalcTangentSpace でタンジェント空間を計算
    /// @brief テクスチャパスは自動的に相対パスに変換
    /// 
    /// @param device D3D11デバイス
    /// @param assets AssetManager（テクスチャロード用）
    /// @param modelPath モデルファイルのパス
    /// @return ロードされたモデル（失敗時は nullptr）
    std::shared_ptr<Model> LoadAssimpModel(
        ID3D11Device* device,
        AssetManager& assets,
        const std::wstring& modelPath
    );

} // namespace Engine
