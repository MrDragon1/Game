#pragma once

#include "Car.h"
#include "Shader.h"
#include "Camera.h"
#include "Environment.h"
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
	void OnKeyEvent();


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
	void ReadPixel();
/// Scene
private:
	vector<CarPtr> mCars;
	// vector<Wall*> mWalls;
	CameraPtr mCamera;
	Vector3 mMove = Vector3(0,0,0);

	Environment mEnvironment;
/// Window
private:
	const size_t mWidth = 800;
	const size_t mHeight = 600;
	GLFWwindow* mWindow;
	static Engine* sInstance;

/// Render 
private:
	const GLuint mShadowMapWidth = 1024, mShadowMapHeight = 1024;
	GLuint mMainFBO,mShadowFBO;
	GLuint mDepthMap, mColorAttachment, mIDAttachment, mDepthAttachment;
	ShaderPtr mShadowShader, mMainShader, mPresentShader,mSkyboxShader;
	GLuint mPresentVAO, mPresentVBO;
	ModelPtr mCube;
};

