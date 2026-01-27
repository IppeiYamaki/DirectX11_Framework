#pragma once

#include <DirectXMath.h>
#include <list>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    class Transform final : public Component {
    private:
		Vector3 m_position;                                 /// ローカル座標での位置
		Vector3 m_rotation;                                 /// ローカル座標での回転（オイラー角：度単位）
		Vector3 m_scale;                                    /// ローカル座標でのスケール

        Transform* m_parent;                                /// 親Transformへのポインタ
        std::list<Transform*> m_children;                   /// 子Transformのリスト

		mutable DirectX::XMFLOAT4X4 m_localToWorldMatrix{}; /// ローカルからワールドへの変換行列
		mutable DirectX::XMFLOAT4X4 m_worldToLocalMatrix{}; /// ワールドからローカルへの変換行列
		mutable bool                m_isDirty = true;       /// 変換行列への更新が必要かどうかのフラグ

		//============================================================
		// 正規化
		//============================================================
		/// @brief 角度を0～360度の範囲に正規化
		/// @param angle 正規化する角度（度単位）
		/// @return 正規化後の角度（度単位）
		static float NormalizeAngle(float angle);

		/// @brief Vector3の各軸の角度を0～360度の範囲に正規化
		/// @param angles 正規化する角度ベクトル（度単位）
		/// @return 正規化後の角度ベクトル（度単位）
		static Vector3 NormalizeAngle(const Vector3& angles);

		//============================================================
		// 子/親の設定・取得
		//============================================================
		/// @brief 子Transformをリストに追加
		/// @param child 追加する子Transform
		/// @note GameObject::SetParent() 経由で内部から呼ばれる想定
		void AddChild(Transform* child);

		/// @brief 子Transformをリストから削除
		/// @param child 削除する子Transform
		/// @note GameObject::SetParent() 経由で内部から呼ばれる想定
		void RemoveChild(Transform* child);

		/// @brief 変換行列の再計算が必要なことをマークする
		/// @brief マークを子にも伝播する
		void SetDirty();

    public:
        Transform() = default;
        ~Transform() override = default;

        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;

        //============================================================
        // Position
        //============================================================
		/// @brief ローカル座標での位置の設定
        /// @param position 位置
        void SetPosition(const Vector3& position);
        /// @brief ローカル座標での位置の設定（個別設定）
		/// @param x X座標
		/// @param y Y座標
		/// @param z Z座標
		void SetPosition(float x, float y, float z);

		/// @brief ローカル座標での位置の加算
        /// @param delta 加算位置
        void AddPosition(const Vector3& delta);
		/// @brief ローカル座標での位置の加算（個別指定）
		/// @param deltaX X座標加算値
		/// @param deltaY Y座標加算値
		/// @param deltaZ Z座標加算値
		void AddPosition(float deltaX, float deltaY, float deltaZ);

		/// @brief ローカル座標での位置の取得
		/// @return ローカル座標での位置の参照
        const Vector3& GetPosition() const;

		/// @brief ワールド座標での位置の設定
		/// @param worldPosition ワールド座標での位置
		/// @note 親がいる場合はローカル座標に変換して設定する
		void SetWorldPosition(const Vector3& worldPosition);
		/// @brief ワールド座標での位置の設定（個別指定）
		/// @param x ワールドX座標
		/// @param y ワールドY座標
		/// @param z ワールドZ座標
		void SetWorldPosition(float x, float y, float z);

        
		/// @brief ワールド座標での位置の加算
		/// @param deltaWorldPosition ワールド座標での加算位置
		void AddWorldPosition(const Vector3& deltaWorldPosition);
		/// @brief ワールド座標での位置の取得
		/// @return ワールド座標での位置
		const Vector3 GetWorldPosition() const;


        //============================================================
        // Rotation (Euler degrees)
        // x = pitch, y = yaw, z = roll
        //============================================================
        /// @brief 回転の設定（オイラー角：度単位）
        /// @param eulerDegrees オイラー角（度単位）
        void SetRotationEulerDegrees(const Vector3& eulerDegrees);
		/// @brief 回転の加算（オイラー角：度単位）
		/// @param deltaDegrees 加算オイラー角（度単位）
        void AddRotationEulerDegrees(const Vector3& deltaDegrees);
        /// @brief 回転の取得（オイラー角：度単位）
        /// @return オイラー角（度単位）の参照
        const Vector3& GetRotationEulerDegrees() const;

		//============================================================
		//camera用の Yaw/Pitch/Roll 指定関数

        /// @brief 回転の設定（Yaw/Pitch/Roll：度単位）
        /// @param yawDegrees           Yaw値（度単位）
        /// @param pitchDegrees         Pitch値（度単位）
        /// @param rollDegrees          Roll値（度単位）
        void SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);
		/// @brief 回転の加算（Yaw/Pitch/Roll：度単位）
		/// @param yawDeltaDegrees      Yaw加算値（度単位）
		/// @param pitchDeltaDegrees    Pitch加算値（度単位）
		/// @param rollDeltaDegrees     Roll加算値（度単位）
        void AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees = 0.0f);

        //============================================================
        // Scale
        //============================================================
        /// @brief スケールの設定
        /// @param scale スケール
        void SetScale(const Vector3& scale);
        /// @brief スケールの均一設定
        /// @param uniformScale 均一スケール値
        void SetUniformScale(float uniformScale);
        /// @brief スケールの加算
        /// @param delta 加算スケール値
		void AddScale(const Vector3& deltaScale);
        /// @brief スケールの取得
        /// @return スケール値の参照
        const Vector3& GetScale() const;

        //============================================================
        // Matrix / Direction
        //============================================================
		/// @brief ワールド行列の取得
		/// @return ワールド行列の参照
        const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

		/// @brief 右方向ベクトルの取得
		/// @return 右方向ベクトル
        Vector3 GetRight() const;
		/// @brief 上方向ベクトルの取得
		/// @return 上方向ベクトル
        Vector3 GetUp() const;
		/// @brief 前方向ベクトルの取得
		/// @return 前方向ベクトル
        Vector3 GetForward() const;

    private:
		/// @brief ワールド行列再構築フラグを立てる
        void MarkDirty();
		/// @brief ワールド行列を再構築する
        void RebuildWorldIfDirty() const;

		/// @brief オイラー角（度単位）から回転クォータニオンを作成する
		/// @param eulerDegrees オイラー角（度単位）
		/// @return 回転クォータニオン
        static DirectX::XMVECTOR MakeRotationQuaternion(const Vector3& eulerDegrees);

    };

} // namespace Engine
