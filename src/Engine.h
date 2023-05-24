#pragma once

#include "Car.h"
#include "Shader.h"
#include "Camera.h"
#include "Environment.h"
#include "TextRenderer.h"

class Engine {
public:
	Engine();

	~Engine();

	static Engine* GetEngine() {
		return sInstance;
	}

	void Init();
	void Clear();
	bool IsEnd();
	void Run();

	void OnViewportChange(GLFWwindow* window, int width, int height);
	void OnMouseScroll(float offset);
	void OnMousePress(int button);
	void OnMouseRelease(int button);
	void OnEvent();

	void UpdateScene(GLfloat delta);
	void ShadowPass();
	void MainPass();

	void PrepareFramebuffer();
	void PrepareShader();
	void PrepareScene();

	std::pair<float, float> GetMousePosition();
	void SetMousePosition(const Vector2& pos);
	void SetMouseVisible();
	void SetMouseInvisible();
	Vector2 GetViewportSize() { return Vector2(mWidth, mHeight); }
	int ReadPixel();
	float ReadDepth();
	/// Scene
private:
	vector<CarPtr> mCars;
	vector<WallPtr> mWalls;
	CameraPtr mCamera;
	Vector3 mMove = Vector3(0, 0, 0);
	Vector3 mLightPos = Vector3(0, 5, 0);

	Player mPlayer1, mPlayer2;
	bool mPlay = false;
	Environment mEnvironment;

	unordered_map<string, bool> mIsPressed;
	/// Window
private:
	size_t mWidth = 800;
	size_t mHeight = 600;
	GLFWwindow* mWindow;
	static Engine* sInstance;

	/// Render 
	TextRendererPtr mTextRenderer;
private:
	const GLuint mShadowMapWidth = 1024, mShadowMapHeight = 1024;
	GLuint mMainFBO = 0, mShadowFBO = 0;
	GLuint mShadowMap = 0, mColorAttachment = 0, mIDAttachment = 0, mDepthAttachment = 0;
	ShaderPtr mShadowShader, mMainShader, mPresentShader, mSkyboxShader;
	GLuint mPresentVAO, mPresentVBO;
	ModelPtr mCube;
	Texture2DPtr mLUT;
};

