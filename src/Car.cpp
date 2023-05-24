#include "Car.h"
int Car::sNextID = 0;
Car::Car(Vector3 pos)
{
	mTransform.mPosition = pos;
	mModel = make_shared<Model>(mModelPath);
	mEntityID = sNextID++;
}

void Car::Draw(ShaderPtr shader)
{
	// slot must >= 4, the first 3 slots are used
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mTexture->GetID());
	shader->SetMat4("uModel", GetModelMatrix());
	shader->SetInt("uEntityID", mEntityID);
	shader->SetBool("uUseAlbedo", false);
	shader->SetBool("uUseNormal", true);
	shader->SetBool("uUseRoughness", true);
	shader->SetFloat2("UVScale", Vector2::ONE);
	if (mSelected) {
		shader->SetFloat("uEmission", 1.0f);
	}
	for (auto mesh : mModel->GetMesh()) {
		shader->SetFloat3("uAlbedo", mesh->mMaterial.Albedo);
		shader->SetFloat("uMetallic", mesh->mMaterial.Metallic);
		shader->SetFloat("uRoughness", mesh->mMaterial.Roughness);
		if (!mSelected) shader->SetFloat("uEmission", mesh->mMaterial.Emission);

		glBindVertexArray(mesh->VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void Car::Update(float delta)
{
	mTransform.mPosition += mVelocity * GetForward();
}

Wall::Wall(const string& path)
{
	mModel = make_shared<Model>(path);
}

void Wall::Draw(ShaderPtr shader)
{
	glActiveTexture(GL_TEXTURE4);
	if (mAlbedoTexture) glBindTexture(GL_TEXTURE_2D, mAlbedoTexture->GetID());
	glActiveTexture(GL_TEXTURE5);
	if (mNormalTexture) glBindTexture(GL_TEXTURE_2D, mNormalTexture->GetID());
	glActiveTexture(GL_TEXTURE6);
	if (mRoughnessTexture) glBindTexture(GL_TEXTURE_2D, mRoughnessTexture->GetID());


	shader->SetMat4("uModel", GetModelMatrix());
	shader->SetInt("uEntityID", -1);
	shader->SetBool("uUseAlbedo", !mAlbedoTexture);
	shader->SetBool("uUseNormal", !mNormalTexture);
	shader->SetBool("uUseRoughness", !mRoughnessTexture);
	shader->SetFloat2("UVScale", mUVScale);
	for (auto mesh : mModel->GetMesh()) {
		shader->SetFloat3("uAlbedo", mesh->mMaterial.Albedo);
		shader->SetFloat("uMetallic", mesh->mMaterial.Metallic);
		shader->SetFloat("uRoughness", mesh->mMaterial.Roughness);
		shader->SetFloat("uEmission", mesh->mMaterial.Emission);
		glBindVertexArray(mesh->VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
