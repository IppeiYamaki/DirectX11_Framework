#pragma once

#include <DirectXMath.h>
#include <list>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    class Transform final : public Component {
    private:
		Vector3 m_position;                                 /// 
		Vector3 m_rotation;                                 /// 
		Vector3 m_scale{ 1.0f, 1.0f, 1.0f };                /// 

        Transform* m_parent = nullptr;                      /// 
        std::list<Transform*> m_children;                   /// 

		mutable DirectX::XMFLOAT4X4 m_localToWorldMatrix{}; /// 
		mutable DirectX::XMFLOAT4X4 m_worldToLocalMatrix{}; /// 
		mutable bool                m_isDirty = true;       /// 

		//============================================================
		// 
		//============================================================
		/// @brief 
		/// @param 
		/// @return 
		static float NormalizeAngle(float angle);

		/// @brief Vector3
		/// @param angles 
		/// @return 
		static Vector3 NormalizeAngle(const Vector3& angles);

		//============================================================
		// 
		//============================================================
		/// @brief 
		/// @param child 
		/// @note Entity::SetParent() 
		void AddChild(Transform* child);

		/// @brief 
		/// @param child 
		/// @note Entity::SetParent() 
		void RemoveChild(Transform* child);

		/// @brief 
		/// @brief 
		void SetDirty();

    public:
        Transform() = default;
        ~Transform() override = default;

        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;

        //============================================================
        // Position
        //============================================================
		/// @brief 
        /// @param position 
        void SetPosition(const Vector3& position);
        /// @brief 
		/// @param x 
		/// @param y 
		/// @param z 
		void SetPosition(float x, float y, float z);

		/// @brief 
        /// @param delta 
        void AddPosition(const Vector3& delta);
		/// @brief 
		/// @param deltaX 
		/// @param deltaY 
		/// @param deltaZ 
		void AddPosition(float deltaX, float deltaY, float deltaZ);

		/// @brief 
		/// @return 
        const Vector3& GetPosition() const;

		/// @brief 
		/// @param worldPosition 
		/// @note 
		void SetWorldPosition(const Vector3& worldPosition);
		/// @brief 
		/// @param x 
		/// @param y 
		/// @param z 
		void SetWorldPosition(float x, float y, float z);

        
		/// @brief 
		/// @param deltaWorldPosition 
		void AddWorldPosition(const Vector3& deltaWorldPosition);
		/// @brief 
		/// @return 
		const Vector3 GetWorldPosition() const;


        //============================================================
        // Rotation (Euler degrees)
        // x = pitch, y = yaw, z = roll
        //============================================================
        /// @brief 
        /// @param eulerDegrees 
        void SetRotationEulerDegrees(const Vector3& eulerDegrees);
		/// @brief 
		/// @param deltaDegrees 
        void AddRotationEulerDegrees(const Vector3& deltaDegrees);
        /// @brief 
        /// @return 
        const Vector3& GetRotationEulerDegrees() const;

		//============================================================
		//camera

        /// @brief 
        /// @param yawDegrees    
        /// @param pitchDegrees  
        /// @param rollDegrees   
        void SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);
		/// @brief 
		/// @param yawDeltaDegrees      
		/// @param pitchDeltaDegrees    
		/// @param rollDeltaDegrees     
        void AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees = 0.0f);

        //============================================================
        // Scale
        //============================================================
        /// @brief 
        /// @param scale 
        void SetScale(const Vector3& scale);
        /// @brief
        /// @param uniformScale 
        void SetUniformScale(float uniformScale);
        /// @brief 
        /// @param delta 
		void AddScale(const Vector3& deltaScale);
        /// @brief 
        /// @return 
        const Vector3& GetScale() const;

        //============================================================
        // Parent-Child Hierarchy
        //============================================================
        /// @brief 親Transformを設定（nullptrでルートに）
        /// @param parent 親Transform（nullptrで親なし）
        void SetParent(Transform* parent);

        /// @brief 親Transformを取得
        /// @return 親Transform（なければnullptr）
        [[nodiscard]] Transform* GetParent() const;

        /// @brief 子Transformのリストを取得
        /// @return 子Transformのリスト
        [[nodiscard]] const std::list<Transform*>& GetChildren() const;

        //============================================================
        // Matrix / Direction
        //============================================================
		/// @brief 
		/// @return 
        const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

		/// @brief 
		/// @return 
        Vector3 GetRight() const;
		/// @brief 
		/// @return 
        Vector3 GetUp() const;
		/// @brief 
		/// @return 
        Vector3 GetForward() const;

    private:
		/// @brief 
        void MarkDirty();
		/// @brief 
        void RebuildWorldIfDirty() const;

		/// @brief 
		/// @param eulerDegrees 
		/// @return 
        static DirectX::XMVECTOR MakeRotationQuaternion(const Vector3& eulerDegrees);

    };

} // namespace Engine
