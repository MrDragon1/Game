#include "Camera.h"

#include "Engine.h"

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
	: mFOV(fov),
	mAspectRatio(aspectRatio),
	mNearClip(nearClip),
	mFarClip(farClip),
	mProjection(Math::Perspective(Math::Radians(fov), aspectRatio, nearClip, farClip)) {
	UpdateView();
}

void Camera::UpdateProjection() {
	auto v = Engine::GetEngine()->GetViewportSize();
	mAspectRatio = v.x / v.y;
	mProjection = Math::Perspective(Math::Radians(mFOV), mAspectRatio, mNearClip, mFarClip);
}

void Camera::UpdateView() {
	// mYaw = mPitch = 0.0f; // Lock the camera's rotation
	mViewMatrix = Math::LookAt(mPosition, mPosition + GetForwardDirection(), GetUpDirection());
}

float Camera::RotationSpeed() const { return 0.8f; }

float Camera::ZoomSpeed() const { return 0.7f; }

Vector3 Camera::MouseSpeed() const { return Vector3{ 0.03f, 0.03f, 0.03f }; };

void Camera::UpdatePosition(CameraMovement move) {
	if (mModifying) {
		auto pos = Engine::GetEngine()->GetMousePosition();
		Vector2 mouse = { pos.first, pos.second };
		Vector2 delta = (mouse - mInitialMousePosition) * 0.003f;

		switch (move)
		{
		case Camera::Forward:
			mPosition += GetForwardDirection() * MouseSpeed().y;
			break;
		case Camera::Backward:
			mPosition -= GetForwardDirection() * MouseSpeed().y;
			break;
		case Camera::Left:
			mPosition -= GetRightDirection() * MouseSpeed().x;
			break;
		case Camera::Right:
			mPosition += GetRightDirection() * MouseSpeed().x;
			break;
		case Camera::Up:
			mPosition -= GetUpDirection() * MouseSpeed().z;
			break;
		case Camera::Down:
			mPosition += GetUpDirection() * MouseSpeed().z;
			break;
		default:
			break;
		}

		MouseRotate(delta);
		Engine::GetEngine()->SetMousePosition(mInitialMousePosition);
	}

	UpdateView();
}


void Camera::OnRightButtonPressed() {
	mModifying = true;
	auto pos = Engine::GetEngine()->GetMousePosition();
	mInitialMousePosition = { pos.first,pos.second };
	Engine::GetEngine()->SetMouseInvisible();
}

void Camera::OnRightButtonReleased() {
	mModifying = false;
	Engine::GetEngine()->SetMouseVisible();
}

void Camera::OnMouseScroll(float offset) {
	MouseZoom(offset);
	UpdateView();
}

void Camera::MouseRotate(const Vector2& delta) {
	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	mYaw += yawSign * delta.x * RotationSpeed();
	mPitch += delta.y * RotationSpeed();
}

void Camera::MouseZoom(float delta) {
	mFOV -= delta * ZoomSpeed();
	mFOV = std::clamp(mFOV, 10.0f, 60.0f);
	UpdateProjection();
}

Vector3 Camera::GetUpDirection() const { return Math::Rotate(GetOrientation(), Vector3(0.0f, 1.0f, 0.0f)); }

Vector3 Camera::GetRightDirection() const { return Math::Rotate(GetOrientation(), Vector3(1.0f, 0.0f, 0.0f)); }

Vector3 Camera::GetForwardDirection() const { return Math::Rotate(GetOrientation(), Vector3(0.0f, 0.0f, -1.0f)); }

Quaternion Camera::GetOrientation() const { return Quaternion(Vector3(-mPitch, -mYaw, 0.0f)); }
