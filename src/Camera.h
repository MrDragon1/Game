#pragma once
#include "Defines.h"
#include "math/Matrix.h"
using CameraPtr = shared_ptr<class Camera>;

class Camera {
public:
	enum CameraMovement
	{
		Forward, Backward, Left, Right, Up, Down,None,
	};

	Camera() = default;
	Camera(float fov, float aspectRatio, float nearClip, float farClip);

	void UpdatePosition(CameraMovement move);

	inline void SetViewportSize() {
		UpdateProjection();
	}

	const Matrix4& GetViewMatrix() const { return mViewMatrix; }
	Matrix4 GetViewProjection() const { return mProjection * mViewMatrix; }

	Vector3 GetUpDirection() const;
	Vector3 GetRightDirection() const;
	Vector3 GetForwardDirection() const;
	const Vector3& GetPosition() const { return mPosition; }
	Quaternion GetOrientation() const;

	float GetPitch() const { return mPitch; }
	float GetYaw() const { return mYaw; }

	float GetFOV() const { return mFOV; }
	float GetAspectRatio() const { return mAspectRatio; }
	float GetNearPlane() const { return mNearClip; }
	float GetFarPlane() const { return mFarClip; }
	const Matrix4& GetProjection() const { return mProjection; }

	void OnMouseScroll(float offset);
	void OnRightButtonPressed();
	void OnRightButtonReleased();

private:
	void UpdateProjection();
	void UpdateView();

	void MouseRotate(const Vector2& delta);
	void MouseZoom(float delta);

	float RotationSpeed() const;
	float ZoomSpeed() const;
	Vector3 MouseSpeed() const;

private:
	Matrix4 mProjection = Matrix4::IDENTITY;
	float mFOV = 45.0f, mAspectRatio = 1.778f, mNearClip = 0.1f, mFarClip = 1000.0f;

	Matrix4 mViewMatrix;
	Vector3 mPosition = { 0.0f, 0.0f, 3.0f };

	Vector2 mInitialMousePosition = { 0.0f, 0.0f };

	float mPitch = 0.0f, mYaw = 0.0f;

	bool mModifying = false;
};

