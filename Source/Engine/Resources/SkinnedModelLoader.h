#pragma once

#include <memory>
#include <string>

struct ID3D11Device;

namespace Engine {

    class AssetManager;
    class SkinnedModel;

    std::shared_ptr<SkinnedModel> LoadSkinnedModel(
        ID3D11Device* device,
        AssetManager& assets,
        const std::wstring& modelPath
    );

    /**
     * @brief アニメだけ別ファイルから取り込んで、既存SkinnedModelへ追加する
     * @param clipName 空なら「ファイル内のアニメ名」を使う。複数アニメがある場合は全追加。
     */
    bool AddAnimationFromFile(
        SkinnedModel& model,
        AssetManager& assets,
        const std::wstring& animPath,
        const std::string& clipName = std::string()
    );

} // namespace Engine
