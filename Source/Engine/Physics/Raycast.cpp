#include "Raycast.h"

#include "Engine/Physics/Ray.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"

#include <cmath>
#include <limits>
#include <algorithm>

namespace Engine {

    //============================================================
    // RaycastResult
    //============================================================

    RaycastResult::RaycastResult()
        : m_hitObject(nullptr)
        , m_distance(0.0f)
        , m_hitPoint(Vector3::Zero())
        , m_normal(Vector3::UnitY())
        , m_hasHit(false) {
    }

    bool RaycastResult::HasHit() const {
        return m_hasHit;
    }

    GameObject* RaycastResult::GetHitObject() const {
        return m_hitObject;
    }

    float RaycastResult::GetDistance() const {
        return m_distance;
    }

    const Vector3& RaycastResult::GetHitPoint() const {
        return m_hitPoint;
    }

    const Vector3& RaycastResult::GetNormal() const {
        return m_normal;
    }

    //============================================================
    // Physics - Raycast
    //============================================================

    bool Physics::Raycast(const Ray& ray, Scene* scene, RaycastResult& result, float maxDistance) {
        if (!scene) {
            return false;
        }

        result = RaycastResult(); // リセット

        float closestDistance = maxDistance;
        GameObject* closestObject = nullptr;

        // シーン内の全GameObjectに対してテスト
        // 簡易実装：各オブジェクトを球体として扱う
        auto objects = scene->FindObjectsWhere([](const GameObject* obj) {
            return obj->IsActive();
        });

        for (auto* obj : objects) {
            if (!obj) continue;

            // オブジェクトの位置を中心とした簡易球体判定
            Vector3 center = obj->GetPosition();
            Vector3 scale = obj->GetScale();
            float radius = (std::max)(scale.x, (std::max)(scale.y, scale.z)) * 0.5f;

            float distance = 0.0f;
            if (RaySphereIntersect(ray, center, radius, distance)) {
                if (distance < closestDistance && distance >= 0.0f) {
                    closestDistance = distance;
                    closestObject = obj;
                }
            }
        }

        if (closestObject) {
            result.m_hasHit = true;
            result.m_hitObject = closestObject;
            result.m_distance = closestDistance;
            result.m_hitPoint = ray.GetPoint(closestDistance);
            // 簡易法線計算（球体の場合）
            Vector3 toHit = result.m_hitPoint - closestObject->GetPosition();
            result.m_normal = toHit.Normalized();
            return true;
        }

        return false;
    }

    bool Physics::Raycast(const Ray& ray, Scene* scene, float maxDistance) {
        RaycastResult result;
        return Raycast(ray, scene, result, maxDistance);
    }

    //============================================================
    // Physics - Sphere Intersection
    //============================================================

    bool Physics::RaySphereIntersect(const Ray& ray, const Vector3& center, float radius, float& outDistance) {
        // レイの原点から球の中心へのベクトル
        Vector3 oc = ray.GetOrigin() - center;

        // 二次方程式の係数を計算
        // |origin + t*direction - center|^2 = radius^2
        // a*t^2 + b*t + c = 0
        float a = Vector3::Dot(ray.GetDirection(), ray.GetDirection());
        float b = 2.0f * Vector3::Dot(oc, ray.GetDirection());
        float c = Vector3::Dot(oc, oc) - radius * radius;

        // 判別式
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f) {
            return false; // 交差なし
        }

        // 最も近い交点を計算
        float sqrtD = std::sqrt(discriminant);
        float t1 = (-b - sqrtD) / (2.0f * a);
        float t2 = (-b + sqrtD) / (2.0f * a);

        // 最も近い正の交点を選択
        if (t1 >= 0.0f) {
            outDistance = t1;
            return true;
        }
        if (t2 >= 0.0f) {
            outDistance = t2;
            return true;
        }

        return false; // レイの後方で交差
    }

    //============================================================
    // Physics - AABB Intersection
    //============================================================

    bool Physics::RayAABBIntersect(const Ray& ray, const Vector3& min, const Vector3& max, float& outDistance) {
        const Vector3& origin = ray.GetOrigin();
        const Vector3& dir = ray.GetDirection();

        float tMin = 0.0f;
        float tMax = std::numeric_limits<float>::max();

        // X軸のスラブテスト
        if (std::abs(dir.x) < Vector3::kEpsilon) {
            if (origin.x < min.x || origin.x > max.x) {
                return false;
            }
        }
        else {
            float invD = 1.0f / dir.x;
            float t1 = (min.x - origin.x) * invD;
            float t2 = (max.x - origin.x) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) return false;
        }

        // Y軸のスラブテスト
        if (std::abs(dir.y) < Vector3::kEpsilon) {
            if (origin.y < min.y || origin.y > max.y) {
                return false;
            }
        }
        else {
            float invD = 1.0f / dir.y;
            float t1 = (min.y - origin.y) * invD;
            float t2 = (max.y - origin.y) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) return false;
        }

        // Z軸のスラブテスト
        if (std::abs(dir.z) < Vector3::kEpsilon) {
            if (origin.z < min.z || origin.z > max.z) {
                return false;
            }
        }
        else {
            float invD = 1.0f / dir.z;
            float t1 = (min.z - origin.z) * invD;
            float t2 = (max.z - origin.z) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) return false;
        }

        outDistance = tMin;
        return true;
    }

} // namespace Engine
