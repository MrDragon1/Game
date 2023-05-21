#pragma once
#include "Defines.h"
#include "Model.h"
#include "Shader.h"
using CarPtr = shared_ptr<class Car>;
using WallPtr = shared_ptr<class Wall>;

class Transform {
public:
	Vector3 mPosition{ Vector3::ZERO };
	Vector3 mScale{ Vector3::ONE };
	Quaternion mRotation{ Quaternion::IDENTITY };

	Transform() = default;
	Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale) : mPosition{ position }, mScale{ scale }, mRotation{ rotation } {}

	Matrix4 getMatrix() const {
		Matrix4 temp = Math::Translate(Matrix4::IDENTITY, mPosition) * Matrix4(mRotation) * Math::Scale(Matrix4::IDENTITY, mScale);
		return temp;
	}
};

class Car {
public:
	Car(Vector3 pos = Vector3(0));

	void Draw(ShaderPtr shader);

	Matrix4 GetModelMatrix() { return mTransform.getMatrix(); }

	ModelPtr mModel;
	Texture2DPtr mTexture;
	int mEntityID;
	bool mSelected = false;
	Transform mTransform;
	float mVelocity;
	Vector2 mDirection;
	const string mModelPath = "asset/model/car.obj";
	const string mTexturePath = "asset/texture/test.png";


	static int sNextID;
};


class Wall {
public:
	Wall();

	void Draw(ShaderPtr shader);


	Matrix4 GetModelMatrix() { return mTransform.getMatrix(); }

	Transform mTransform;

	ModelPtr mModel;
	Texture2DPtr mTexture;
	const string mModelPath = "asset/model/cube.obj";
	const string mTexturePath = "asset/texture/test.png";
};
