#include "MaterialLibrary.h"

namespace Engine {

    void MaterialLibrary::Initialize(const MaterialBuildContext& ctx) {
        m_ctx = ctx;
    }

    void MaterialLibrary::Finalize() {
        m_cache.clear();
        m_ctx = {};
    }

} // namespace Engine
