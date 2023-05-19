#pragma once
#include "Defines.h"
#include "Model.h"
#include "Shader.h"
using CarPtr = shared_ptr<class Car>;

class Car{
public:
	Car();

	void Draw(ShaderPtr shader);

	Matrix4 GetModelMatrix();
private:
	ModelPtr mModel;
	Texture2DPtr mTexture;

	Vector2 mPosition;
	float mVelocity;
	Vector2 mDirection;
	const string mModelPath = "asset/model/cube.obj";
	const string mTexturePath = "asset/texture/test.png";
};

