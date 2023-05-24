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
	float mTheta = 0; //ºÍ-zµÄ¼Ð½Ç

	Transform() = default;

	Matrix4 getMatrix() const {
		Matrix4 temp = Math::Translate(Matrix4::IDENTITY, mPosition) * Math::Rotate(Matrix4::IDENTITY, mTheta,Vector3(0,1,0)) * Math::Scale(Matrix4::IDENTITY, mScale);
		return temp;
	}
};

class Car {
public:
	Car(Vector3 pos = Vector3(0));

	void Draw(ShaderPtr shader);

	void Update(float delta);

	Matrix4 GetModelMatrix() { return mTransform.getMatrix(); }
	Vector3 GetForward() { return Vector3(-sin(mTransform.mTheta), 0, -cos(mTransform.mTheta)); }
	Vector3 GetRight() { return Math::Cross(GetForward(), GetUp()); }
	Vector3 GetUp() { return Vector3(0, 1, 0); }

	ModelPtr mModel;
	Texture2DPtr mTexture;
	int mEntityID;
	bool mSelected = false;
	Transform mTransform;
	float mVelocity = 0;


private:
	const string mModelPath = "asset/model/car.obj";
	static int sNextID;
};


class Wall {
public:
	Wall(const string& path);

	void Draw(ShaderPtr shader);
	Matrix4 GetModelMatrix() { return mTransform.getMatrix(); }

	Transform mTransform;
	Vector2 mUVScale = Vector2(1, 1);
	ModelPtr mModel;
	Texture2DPtr mAlbedoTexture, mNormalTexture, mRoughnessTexture;
};

class Player {
public:
	int mScore = 0;
	int mCarID = -1;
};
