#include "Car.h"

Car::Car()
{
	mModel = make_shared<Model>(mModelPath);
	mTexture = make_shared<class Texture2D>(mTexturePath);
}

void Car::Draw(ShaderPtr shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture->GetID());

	shader->SetMat4("model", GetModelMatrix());
	mModel->Draw();
}

Matrix4 Car::GetModelMatrix()
{
	return Matrix4(1.0f);
}
