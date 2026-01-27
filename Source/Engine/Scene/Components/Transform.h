#pragma once

#include <DirectXMath.h>
#include <list>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    class Transform final : public Component {
    private:
		Vector3 m_position;                                 /// ���[�J�����W�ł̈ʒu
		Vector3 m_rotation;                                 /// ���[�J�����W�ł̉�]�i�I�C���[�p�F�x�P�ʁj
		Vector3 m_scale{ 1.0f, 1.0f, 1.0f };                /// ���[�J�����W�ł̃X�P�[��

        Transform* m_parent = nullptr;                      /// �eTransform�ւ̃|�C���^
        std::list<Transform*> m_children;                   /// �qTransform�̃��X�g

		mutable DirectX::XMFLOAT4X4 m_localToWorldMatrix{}; /// ���[�J�����烏�[���h�ւ̕ϊ��s��
		mutable DirectX::XMFLOAT4X4 m_worldToLocalMatrix{}; /// ���[���h���烍�[�J���ւ̕ϊ��s��
		mutable bool                m_isDirty = true;       /// �ϊ��s��ւ̍X�V���K�v���ǂ����̃t���O

		//============================================================
		// ���K��
		//============================================================
		/// @brief �p�x��0�`360�x�͈̔͂ɐ��K��
		/// @param angle ���K������p�x�i�x�P�ʁj
		/// @return ���K����̊p�x�i�x�P�ʁj
		static float NormalizeAngle(float angle);

		/// @brief Vector3�̊e���̊p�x��0�`360�x�͈̔͂ɐ��K��
		/// @param angles ���K������p�x�x�N�g���i�x�P�ʁj
		/// @return ���K����̊p�x�x�N�g���i�x�P�ʁj
		static Vector3 NormalizeAngle(const Vector3& angles);

		//============================================================
		// �q/�e�̐ݒ�E�擾
		//============================================================
		/// @brief �qTransform�����X�g�ɒǉ�
		/// @param child �ǉ�����qTransform
		/// @note Entity::SetParent() �o�R�œ�������Ă΂��z��
		void AddChild(Transform* child);

		/// @brief �qTransform�����X�g����폜
		/// @param child �폜����qTransform
		/// @note Entity::SetParent() �o�R�œ�������Ă΂��z��
		void RemoveChild(Transform* child);

		/// @brief �ϊ��s��̍Čv�Z���K�v�Ȃ��Ƃ��}�[�N����
		/// @brief �}�[�N���q�ɂ��`�d����
		void SetDirty();

    public:
        Transform() = default;
        ~Transform() override = default;

        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;

        //============================================================
        // Position
        //============================================================
		/// @brief ���[�J�����W�ł̈ʒu�̐ݒ�
        /// @param position �ʒu
        void SetPosition(const Vector3& position);
        /// @brief ���[�J�����W�ł̈ʒu�̐ݒ�i�ʐݒ�j
		/// @param x X���W
		/// @param y Y���W
		/// @param z Z���W
		void SetPosition(float x, float y, float z);

		/// @brief ���[�J�����W�ł̈ʒu�̉��Z
        /// @param delta ���Z�ʒu
        void AddPosition(const Vector3& delta);
		/// @brief ���[�J�����W�ł̈ʒu�̉��Z�i�ʎw��j
		/// @param deltaX X���W���Z�l
		/// @param deltaY Y���W���Z�l
		/// @param deltaZ Z���W���Z�l
		void AddPosition(float deltaX, float deltaY, float deltaZ);

		/// @brief ���[�J�����W�ł̈ʒu�̎擾
		/// @return ���[�J�����W�ł̈ʒu�̎Q��
        const Vector3& GetPosition() const;

		/// @brief ���[���h���W�ł̈ʒu�̐ݒ�
		/// @param worldPosition ���[���h���W�ł̈ʒu
		/// @note �e������ꍇ�̓��[�J�����W�ɕϊ����Đݒ肷��
		void SetWorldPosition(const Vector3& worldPosition);
		/// @brief ���[���h���W�ł̈ʒu�̐ݒ�i�ʎw��j
		/// @param x ���[���hX���W
		/// @param y ���[���hY���W
		/// @param z ���[���hZ���W
		void SetWorldPosition(float x, float y, float z);

        
		/// @brief ���[���h���W�ł̈ʒu�̉��Z
		/// @param deltaWorldPosition ���[���h���W�ł̉��Z�ʒu
		void AddWorldPosition(const Vector3& deltaWorldPosition);
		/// @brief ���[���h���W�ł̈ʒu�̎擾
		/// @return ���[���h���W�ł̈ʒu
		const Vector3 GetWorldPosition() const;


        //============================================================
        // Rotation (Euler degrees)
        // x = pitch, y = yaw, z = roll
        //============================================================
        /// @brief ��]�̐ݒ�i�I�C���[�p�F�x�P�ʁj
        /// @param eulerDegrees �I�C���[�p�i�x�P�ʁj
        void SetRotationEulerDegrees(const Vector3& eulerDegrees);
		/// @brief ��]�̉��Z�i�I�C���[�p�F�x�P�ʁj
		/// @param deltaDegrees ���Z�I�C���[�p�i�x�P�ʁj
        void AddRotationEulerDegrees(const Vector3& deltaDegrees);
        /// @brief ��]�̎擾�i�I�C���[�p�F�x�P�ʁj
        /// @return �I�C���[�p�i�x�P�ʁj�̎Q��
        const Vector3& GetRotationEulerDegrees() const;

		//============================================================
		//camera�p�� Yaw/Pitch/Roll �w��֐�

        /// @brief ��]�̐ݒ�iYaw/Pitch/Roll�F�x�P�ʁj
        /// @param yawDegrees           Yaw�l�i�x�P�ʁj
        /// @param pitchDegrees         Pitch�l�i�x�P�ʁj
        /// @param rollDegrees          Roll�l�i�x�P�ʁj
        void SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);
		/// @brief ��]�̉��Z�iYaw/Pitch/Roll�F�x�P�ʁj
		/// @param yawDeltaDegrees      Yaw���Z�l�i�x�P�ʁj
		/// @param pitchDeltaDegrees    Pitch���Z�l�i�x�P�ʁj
		/// @param rollDeltaDegrees     Roll���Z�l�i�x�P�ʁj
        void AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees = 0.0f);

        //============================================================
        // Scale
        //============================================================
        /// @brief �X�P�[���̐ݒ�
        /// @param scale �X�P�[��
        void SetScale(const Vector3& scale);
        /// @brief �X�P�[���̋ψ�ݒ�
        /// @param uniformScale �ψ�X�P�[���l
        void SetUniformScale(float uniformScale);
        /// @brief �X�P�[���̉��Z
        /// @param delta ���Z�X�P�[���l
		void AddScale(const Vector3& deltaScale);
        /// @brief �X�P�[���̎擾
        /// @return �X�P�[���l�̎Q��
        const Vector3& GetScale() const;

        //============================================================
        // Matrix / Direction
        //============================================================
		/// @brief ���[���h�s��̎擾
		/// @return ���[���h�s��̎Q��
        const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

		/// @brief �E�����x�N�g���̎擾
		/// @return �E�����x�N�g��
        Vector3 GetRight() const;
		/// @brief ������x�N�g���̎擾
		/// @return ������x�N�g��
        Vector3 GetUp() const;
		/// @brief �O�����x�N�g���̎擾
		/// @return �O�����x�N�g��
        Vector3 GetForward() const;

    private:
		/// @brief ���[���h�s��č\�z�t���O�𗧂Ă�
        void MarkDirty();
		/// @brief ���[���h�s����č\�z����
        void RebuildWorldIfDirty() const;

		/// @brief �I�C���[�p�i�x�P�ʁj�����]�N�H�[�^�j�I�����쐬����
		/// @param eulerDegrees �I�C���[�p�i�x�P�ʁj
		/// @return ��]�N�H�[�^�j�I��
        static DirectX::XMVECTOR MakeRotationQuaternion(const Vector3& eulerDegrees);

    };

} // namespace Engine
