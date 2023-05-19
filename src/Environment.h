#pragma once
#include "Model.h"
#include "math/Matrix.h"
#include "Shader.h"
class Environment {
public:
	Environment();

	void Init(int width, int height);

	void Draw();

	void Resize(int width, int height);

	TextureCubePtr GetRadiance() { return mEnvironmentCubeMap; }
	TextureCubePtr GetIrradianvce() { return mReflectionCubeMap; }

private:
	GLuint mFramebuffer = 0;

	ShaderPtr mEquirectangularToCubeMapShader;
	std::string mEquirectangularToCubeMapShaderPath = "asset/shader/EquirectangularToCubeMap";
	ShaderPtr mPrefilterShader;
	std::string mPrefilterShaderPath = "asset/shader/Prefilter";
	ShaderPtr mConvolutionShader;
	std::string mConvolutionShaderPath = "asset/shader/IrradianceConvolution";

	TextureCubePtr mEnvironmentCubeMap;
	TextureCubePtr mReflectionCubeMap;

	Texture2DPtr mTexture;
	TextureCubePtr mTextureCube;
	std::string mPath = "asset/texture/input.png";

	ModelPtr mCube;
};

