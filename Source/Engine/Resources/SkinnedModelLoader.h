#pragma once

#include <memory>
#include <string>

struct ID3D11Device;

namespace Engine {

    class AssetManager;
    class SkinnedModel;

	/// @brief SkinnedModel を Assimp で読み込む。OBJ/FBX/GLTF/DAE など対応。
	/// @param device D3D11デバイス
	/// @param assets AssetManager（テクスチャロード用、ベースディレクトリ解決用）
    /// @param modelPath モデルファイルのパス
    /// @return ロードされたスキンモデル（失敗時は nullptr）
    std::shared_ptr<SkinnedModel> LoadSkinnedModel(
        ID3D11Device* device,
        AssetManager& assets,
        const std::wstring& modelPath
    );

    /// @brief アニメだけ別ファイルから取り込んで、既存SkinnedModelへ追加する
	/// @param model 追加先のSkinnedModel
	/// @param assets AssetManager（テクスチャロード用、ベースディレクトリ解決用）
    /// @param animPath アニメーションファイルのパス
	/// @param clipName 追加するクリップの名前（省略した場合はファイル内のアニメーション名を使用。複数アニメーションがある場合は "clipName_AnimN" 形式で自動生成）
	/// @return 成功した場合は true、失敗した場合は false
    bool AddAnimationFromFile(
        SkinnedModel& model,
        AssetManager& assets,
        const std::wstring& animPath,
        const std::string& clipName = std::string()
    );

} // namespace Engine
