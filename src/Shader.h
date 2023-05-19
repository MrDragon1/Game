#pragma once
#include "Defines.h"
#include "math/Matrix.h"
#include "glad/glad.h"

using ShaderPtr = shared_ptr<class Shader>;
class Shader {
public:
	Shader(const string& vertexPath, const string& fragmentPath);
	void Use();
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetMat3(const std::string& name, const Matrix3& value);
	void SetMat4(const std::string& name, const Matrix4& value);
	
private:
	GLuint mID;
	void CheckCompileErrors(GLuint shader, std::string type);
};
