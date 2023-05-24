#include "Engine.h"
Engine* Engine::sInstance = nullptr;

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message,
	const void* userParam) {
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;  // ignore these non-significant error codes

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		std::cout << "Source: API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		std::cout << "Source: Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		std::cout << "Source: Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		std::cout << "Source: Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		std::cout << "Source: Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		std::cout << "Source: Other";
		break;
	}
	std::cout << std::endl;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "Type: Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "Type: Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "Type: Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "Type: Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "Type: Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		std::cout << "Type: Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		std::cout << "Type: Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		std::cout << "Type: Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "Type: Other";
		break;
	}
	std::cout << std::endl;

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "Severity: high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "Severity: medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "Severity: low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "Severity: notification";
		break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

Engine::Engine()
{
	if (!sInstance)
	{
		sInstance = this;
		Init();
	}
	else INFO("Engine already exists");
}

Engine::~Engine()
{
	Clear();
	sInstance = nullptr;
}

void Engine::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mWindow = glfwCreateWindow(mWidth, mHeight, "Game", nullptr, nullptr);
	if (mWindow == nullptr)
	{
		INFO("Failed to create GLFW window")
			glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(mWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		INFO("Failed to initialize GLAD")
			return;
	}


	glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
		GetEngine()->OnViewportChange(window, width, height);
		});

	glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
		GetEngine()->OnMouseScroll(yoffset);
		});

	glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
		switch (action) {
		case GLFW_PRESS: {
			GetEngine()->OnMousePress(button);
			break;
		}
		case GLFW_RELEASE: {
			GetEngine()->OnMouseRelease(button);
			break;
		}
		}
		});

	// enable OpenGL debug context if context allows for debug context
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	PrepareFramebuffer();
	PrepareShader();
	PrepareScene();

	mTextRenderer = make_shared<class TextRenderer>(mWidth, mHeight);
}

void Engine::Clear()
{
	glfwTerminate();
}

bool Engine::IsEnd()
{
	return glfwWindowShouldClose(mWindow);
}

void Engine::Run()
{
	GLfloat lastFrame = 0, deltaTime;
	while (!IsEnd()) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		UpdateScene(deltaTime);

		OnEvent();

		ShadowPass();
		MainPass();

		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	}
}

void Engine::OnViewportChange(GLFWwindow* window, int width, int height)
{
	mWidth = width;
	mHeight = height;

	glViewport(0, 0, width, height);
	mCamera->SetViewportSize();

	mTextRenderer->OnResize(width, height);

	PrepareFramebuffer();
}

void Engine::OnMouseScroll(float offset)
{
	mCamera->OnMouseScroll(offset);
}

void Engine::OnMousePress(int button)
{
	if (button == 0) // left
	{
		mIsPressed["left"] = true;
	}
	if (button == 1) // right
	{
		mIsPressed["right"] = true;
		mCamera->OnRightButtonPressed();
	}
}

void Engine::OnMouseRelease(int button)
{
	if (button == 0) // left
	{
		mIsPressed["left"] = false;
		if (!mPlay)
		{
			int id = ReadPixel();
			if (id >= 0) //除车辆外的物体id都为-1
			{
				INFO("Selece car with id {}", id);
				if (mPlayer1.mCarID == -1)  mPlayer1.mCarID = id;
				else if (mPlayer2.mCarID == -1)  mPlayer2.mCarID = id;
			}
			else { //选择其他物体 则清空车辆选择
				INFO("Deselect");
				mPlayer1.mCarID = -1;
				mPlayer2.mCarID = -1;
			}

			for (auto& car : mCars)
			{
				car->mSelected = car->mEntityID == mPlayer1.mCarID || car->mEntityID == mPlayer2.mCarID;
			}
		}
	}
	if (button == 1) // right
	{
		mIsPressed["right"] = false;
		mCamera->OnRightButtonReleased();
	}
}

void Engine::OnEvent()
{
	if (mIsPressed["left"])
	{
		if (mPlay && mPlayer2.mCarID >= 0) // Player2 车辆操控
		{
			CarPtr car = mCars[mPlayer2.mCarID];
			auto pos = GetMousePosition();
			float depth = ReadDepth();

			Vector2 mousePosWorld;
			{
				Vector3 winPos = Vector3(pos.first, mHeight - pos.second, depth);
				auto posWorld = Math::Unproject(winPos, mCamera->GetViewMatrix(), mCamera->GetProjection(), Vector4(0, 0, mWidth, mHeight));
				mousePosWorld = Vector2(posWorld.x, posWorld.z);
			}


			Vector2 carPosWorld = Vector2(car->mTransform.mPosition.x, car->mTransform.mPosition.z);

			Vector2 delta = mousePosWorld - carPosWorld;
			// INFO("delta: {}, {}", delta.x, delta.y);
			float dis = Math::Length(delta);
			float angle = atan2(-delta.x, -delta.y);

			// car->mTransform.mTheta = angle;

			// INFO("{},{}", angle / Math::PI * 180, car->mTransform.mTheta / Math::PI * 180);
			float theta = angle - car->mTransform.mTheta;
			if (theta > -Math::PI && theta < Math::PI) {
				car->mTransform.mTheta += theta * 0.05f;
			}
			else if (theta < -Math::PI) {
				car->mTransform.mTheta += (2 * Math::PI + theta) * 0.05f;
			}
			else if (theta > Math::PI) {
				car->mTransform.mTheta += (theta - 2 * Math::PI) * 0.05f;
			}
			car->mTransform.mTheta = fmod(car->mTransform.mTheta, Math::PI * 2);

			{
				float speed_change = Math::Clamp(log(dis), 0, 1) * 0.1f;
				car->mVelocity += (speed_change - car->mVelocity) * 0.1f;
			}
		}
	}



	if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(mWindow, true);
	if (glfwGetKey(mWindow, GLFW_KEY_F3) == GLFW_PRESS) {
		if (mPlayer1.mCarID >= 0 || mPlayer2.mCarID >= 0) mPlay = true;
	}
	if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		mPlay = false;

	if (true) { //暂停模式自由移动视角
		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Forward);
		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Backward);
		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Left);
		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Right);
		if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Up);
		if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
			mCamera->UpdatePosition(Camera::CameraMovement::Down);

		mCamera->UpdatePosition(Camera::CameraMovement::None);
	}
	{
		if (mPlayer1.mCarID >= 0) //Player1 车辆操控
		{
			CarPtr car = mCars[mPlayer1.mCarID];
			float mspeed = 0.001f, rspeed = 0.01f;
			if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS)
				car->mVelocity += mspeed;
			if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
				car->mVelocity -= mspeed;
			if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
				car->mTransform.mTheta += rspeed;
			if (glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
				car->mTransform.mTheta -= rspeed;
		}
	}

}

void Engine::UpdateScene(GLfloat delta)
{
	if (!mPlay) return;

	mLightPos.x = cos(glfwGetTime() * 0.5) * 3.0;
	mLightPos.z = sin(glfwGetTime() * 0.5) * 3.0;

	for (auto& car : mCars)
	{
		car->Update(delta);
	}

}

void Engine::ShadowPass()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLfloat aspect = (GLfloat)mShadowMapWidth / (GLfloat)mShadowMapHeight;
	GLfloat near = 0.1f;
	GLfloat far = 25.0f;
	Matrix4 shadowProj = Math::Perspective(Math::Radians(90.0f), aspect, near, far);
	std::vector<Matrix4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(-1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(0.0, -1.0, 0.0), Vector3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(0.0, 0.0, 1.0), Vector3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * Math::LookAt(mLightPos, mLightPos + Vector3(0.0, 0.0, -1.0), Vector3(0.0, -1.0, 0.0)));

	glViewport(0, 0, mShadowMapWidth, mShadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mShadowShader->Use();
	for (GLuint i = 0; i < 6; ++i)
		mShadowShader->SetMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	mShadowShader->SetFloat3("uLightPos", mLightPos);
	mShadowShader->SetFloat("uFarPlane", far);

	for (auto& car : mCars)
	{
		car->Draw(mShadowShader);
	}
	for (auto& wall : mWalls)
	{
		wall->Draw(mShadowShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::MainPass()
{
	glViewport(0, 0, mWidth, mHeight);

	// Draw scene
	glBindFramebuffer(GL_FRAMEBUFFER, mMainFBO);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.1f, 0.5f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mMainShader->Use();
	mMainShader->SetMat4("uViewProjection", mCamera->GetViewProjection());
	mMainShader->SetFloat3("uCamPos", mCamera->GetPosition());
	mMainShader->SetFloat("uFarPlane", 25.0f); // shadow far plane
	mMainShader->SetFloat3("uLightPos", mLightPos);

	mLUT->Bind(0);
	mEnvironment.GetRadiance()->Bind(1);
	mEnvironment.GetIrradiance()->Bind(2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mShadowMap);

	for (auto& car : mCars)
	{
		car->Draw(mMainShader);
	}

	for (auto& wall : mWalls)
	{
		wall->Draw(mMainShader);
	}
	// Skybox 
	{
		glDepthFunc(GL_LEQUAL);
		mSkyboxShader->Use();
		mSkyboxShader->SetInt("u_SkyboxTexture", 0);
		mSkyboxShader->SetMat4("u_Projection", mCamera->GetProjection());
		mSkyboxShader->SetMat4("u_View", mCamera->GetViewMatrix());
		mEnvironment.GetIrradiance()->Bind(0);

		for (auto mesh : mCube->GetMesh()) {
			glBindVertexArray(mesh->VAO);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->mIndices.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		glDepthFunc(GL_LESS);
	}

	// Present to screen 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		mPresentShader->Use();
		glBindVertexArray(mPresentVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorAttachment);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		mTextRenderer->RenderText("Hello World", 25.0f, 25.0f, 0.5f, Vector3(1, 0, 0));
	}
}

void Engine::PrepareFramebuffer()
{
	if (mShadowFBO) glDeleteFramebuffers(1, &mShadowFBO);
	if (mMainFBO) glDeleteFramebuffers(1, &mMainFBO);
	if (mColorAttachment) glDeleteTextures(1, &mColorAttachment);
	if (mIDAttachment) glDeleteTextures(1, &mIDAttachment);
	if (mDepthAttachment) glDeleteTextures(1, &mDepthAttachment);
	if (mShadowMap) glDeleteTextures(1, &mShadowMap);

	/// Shadow Pass 
	glGenFramebuffers(1, &mShadowFBO);

	glGenTextures(1, &mShadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mShadowMap);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			mShadowMapWidth, mShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("Error Shadow Pass");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/// Main Pass
	glGenFramebuffers(1, &mMainFBO);

	glGenTextures(1, &mColorAttachment);
	glBindTexture(GL_TEXTURE_2D, mColorAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(1, &mIDAttachment);
	glBindTexture(GL_TEXTURE_2D, mIDAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, mWidth, mHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(1, &mDepthAttachment);
	glBindTexture(GL_TEXTURE_2D, mDepthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, mWidth, mHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, mMainFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mIDAttachment, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachment, 0);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("Error Main Pass");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::PrepareShader()
{
	mShadowShader = make_shared<class Shader>("asset/shader/shadow.vs", "asset/shader/shadow.fs", "asset/shader/shadow.gs");
	mMainShader = make_shared<class Shader>("asset/shader/pbr.vs", "asset/shader/pbr.fs");
	mPresentShader = make_shared<class Shader>("asset/shader/present.vs", "asset/shader/present.fs");
	mSkyboxShader = make_shared<class Shader>("asset/shader/skybox.vs", "asset/shader/skybox.fs");
	mLUT = make_shared<class Texture2D>("asset/texture/BRDF_LUT.tga");
	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &mPresentVAO);
	glGenBuffers(1, &mPresentVBO);
	glBindVertexArray(mPresentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mPresentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Engine::PrepareScene()
{
	mCamera = make_shared<class Camera>(45.0f, float(mWidth) / float(mHeight), 0.1f, 100.0f);
	mCube = make_shared<class Model>("asset/model/cube.obj");

	mEnvironment.Init(mWidth, mHeight);
	mEnvironment.Draw();

	mCars.push_back(make_shared<class Car>());
	mCars.push_back(make_shared<class Car>(Vector3(1, 0, 1)));
	mCars.push_back(make_shared<class Car>(Vector3(1, 0, -1)));
	mCars.push_back(make_shared<class Car>(Vector3(-1, 0, -1)));
	mCars.push_back(make_shared<class Car>(Vector3(-1, 0, 1)));


	auto& wall = mWalls.emplace_back(make_shared<class Wall>());
	wall->mTransform.mPosition = Vector3(0, -0.15, 0);
	wall->mTransform.mScale = Vector3(20, 0.01, 20);
	for (auto& mesh : wall->mModel->GetMesh()) {
		mesh->mMaterial.Albedo = Vector3(0.8);
		mesh->mMaterial.Roughness = 0.2f;
		mesh->mMaterial.Metallic = 0.1f;
	}
}

std::pair<float, float> Engine::GetMousePosition()
{
	double xpos, ypos;
	glfwGetCursorPos(mWindow, &xpos, &ypos);
	return { (float)xpos, (float)ypos };
}

void Engine::SetMousePosition(const Vector2& pos)
{
	glfwSetCursorPos(mWindow, pos.x, pos.y);
}

void Engine::SetMouseVisible()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Engine::SetMouseInvisible()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

int Engine::ReadPixel()
{
	auto pos = GetMousePosition();
	glBindFramebuffer(GL_FRAMEBUFFER, mMainFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	int pixelData;
	glReadPixels(pos.first, mHeight - pos.second, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return pixelData;
}

float Engine::ReadDepth()
{
	auto pos = GetMousePosition();
	glBindFramebuffer(GL_FRAMEBUFFER, mMainFBO);
	glReadBuffer(GL_DEPTH_ATTACHMENT);
	float pixelData;
	glReadPixels(pos.first, mHeight - pos.second, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixelData);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return pixelData;
}
