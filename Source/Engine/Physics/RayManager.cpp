/// @file   RayManager.cpp
/// @brief  レイの生成と管理を担当するクラス実装
#include "RayManager.h"

namespace Engine {

    //============================================================
    // Ray Management
    //============================================================

    std::size_t RayManager::AddRay(const Vector3& origin, const Vector3& direction) {
        m_rays.emplace_back(origin, direction);
        return m_rays.size() - 1;
    }

    std::size_t RayManager::AddRay(const Ray& ray) {
        m_rays.push_back(ray);
        return m_rays.size() - 1;
    }

    Ray* RayManager::GetRay(std::size_t index) {
        if (index < m_rays.size()) {
            return &m_rays[index];
        }
        return nullptr;
    }

    const Ray* RayManager::GetRay(std::size_t index) const {
        if (index < m_rays.size()) {
            return &m_rays[index];
        }
        return nullptr;
    }

    bool RayManager::RemoveRay(std::size_t index) {
        if (index < m_rays.size()) {
            m_rays.erase(m_rays.begin() + static_cast<std::ptrdiff_t>(index));
            return true;
        }
        return false;
    }

    void RayManager::ClearRays() {
        m_rays.clear();
    }

    std::size_t RayManager::GetRayCount() const {
        return m_rays.size();
    }

    //============================================================
    // Ray Operations
    //============================================================

    bool RayManager::UpdateRayOrigin(std::size_t index, const Vector3& origin) {
        if (index < m_rays.size()) {
            m_rays[index].SetOrigin(origin);
            return true;
        }
        return false;
    }

    bool RayManager::UpdateRayDirection(std::size_t index, const Vector3& direction) {
        if (index < m_rays.size()) {
            m_rays[index].SetDirection(direction);
            return true;
        }
        return false;
    }

} // namespace Engine
