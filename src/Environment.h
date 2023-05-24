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
	TextureCubePtr GetIrradiance() { return mReflectionCubeMap; }
	Texture2DPtr mTexture;
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


	TextureCubePtr mTextureCube;
	std::string mPath = "asset/texture/Ridgecrest_Road_Ref.hdr";

	ModelPtr mCube;
};

