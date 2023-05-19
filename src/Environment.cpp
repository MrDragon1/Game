#include "Environment.h"

Environment::Environment()
{

}

void Environment::Init(int width, int height)
{
	mCube = make_shared<class Model>("asset/model/cube.obj");


	mEquirectangularToCubeMapShader = make_shared<class Shader>(mEquirectangularToCubeMapShaderPath + ".vs", mEquirectangularToCubeMapShaderPath + ".fs");
	mPrefilterShader = make_shared<class Shader>(mPrefilterShaderPath + ".vs", mPrefilterShaderPath + ".fs");
	mConvolutionShader = make_shared<class Shader>(mConvolutionShaderPath + ".vs", mConvolutionShaderPath + ".fs");

	mEnvironmentCubeMap = make_shared<TextureCube>(32, 32);

	mReflectionCubeMap = make_shared<TextureCube>(512, 512);
	mReflectionCubeMap->GenerateMipmaps();

	mTextureCube = make_shared<TextureCube>(512, 512);

	Resize(width, height);
}


void Environment::Draw()
{
	mTexture = make_shared<Texture2D>(mPath);

	Matrix4 captureProjection = Math::Perspective(Math::Radians(90.0f), 1.0f, 0.1f, 10.0f);
	Matrix4 captureViews[] = { Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)),
								Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)),
								Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
								Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),
								Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f)),
								Math::LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f)) };

	glDepthFunc(GL_LEQUAL);
	mEquirectangularToCubeMapShader->Use();
	mEquirectangularToCubeMapShader->SetInt("u_EquirectangularMap0", 0);
	mEquirectangularToCubeMapShader->SetMat4("u_Projection", captureProjection);

	// Generate Environment CubeMap
	Resize(512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
	for (int i = 0; i < 6; i++) {
		mTexture->Bind(0);
		mEquirectangularToCubeMapShader->SetMat4("u_View", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mTextureCube->GetID(), 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto mesh : mCube->GetMesh()) {
			glBindVertexArray(mesh->VAO);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	// Convolution
	mConvolutionShader->Use();
	mConvolutionShader->SetInt("u_EnvironmentMap", 0);
	mConvolutionShader->SetMat4("u_Projection", captureProjection);

	Resize(32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
	for (int i = 0; i < 6; i++) {
		mTextureCube->Bind(0);
		mConvolutionShader->SetMat4("u_View", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mEnvironmentCubeMap->GetID(), 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto mesh : mCube->GetMesh()) {
			glBindVertexArray(mesh->VAO);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	mEquirectangularToCubeMapShader->Use();
	mEquirectangularToCubeMapShader->SetInt("u_EquirectangularMap0", 0);
	mEquirectangularToCubeMapShader->SetMat4("u_Projection", captureProjection);

	// Generate Reflection CubeMap
	// here, we reuse the m_TextureCube

	// Generate Prefilter CubeMap
	mPrefilterShader->Use();
	mPrefilterShader->SetInt("u_EnvironmentMap", 0);
	mPrefilterShader->SetMat4("u_Projection", captureProjection);

	Resize(512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
	unsigned int maxMipLevels = 10;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(512 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(512 * std::pow(0.5, mip));

		// RenderCommand::SetViewport(0, 0, mipWidth, mipHeight);
		Resize(mipWidth, mipHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		mPrefilterShader->SetFloat("u_Roughness", roughness);
		for (int i = 0; i < 6; i++) {
			mTextureCube->Bind(0);
			mPrefilterShader->SetMat4("u_View", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mReflectionCubeMap->GetID(), mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (auto mesh : mCube->GetMesh()) {
				glBindVertexArray(mesh->VAO);
				glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDepthFunc(GL_LESS);
}

void Environment::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
	if (mFramebuffer)
	{
		glDeleteFramebuffers(1, &mFramebuffer);
	}
	glGenFramebuffers(1, &mFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

	//auto mColorAttachment0 = make_shared<Texture2D>(width, height);
	//auto mColorAttachment1 = make_shared<Texture2D>(width, height);

	GLuint mDepthAttachment;
	glGenTextures(1, &mDepthAttachment);
	glBindTexture(GL_TEXTURE_2D, mDepthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorAttachment0->GetID(), 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColorAttachment1->GetID(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, mDepthAttachment, 0);

	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("Error Environment Pass");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
