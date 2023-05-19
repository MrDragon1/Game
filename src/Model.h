#pragma once
#include "Defines.h"
#include "Utils.h"
#include "math/Matrix.h"
#include "Shader.h"
using MeshPtr = shared_ptr<class Mesh>;
using ModelPtr = shared_ptr<class Model>;
using Texture2DPtr = shared_ptr<class Texture2D>;
using TextureCubePtr = shared_ptr<class TextureCube>;

struct Vertex {
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoords;
	Vector3 Tangent;
	Vector3 Bitangent;
};

class Texture2D {
public:
	Texture2D(const string& path) { mID = Utils::LoadTexture(path); }
	Texture2D(int width, int height);

	void Bind(int slot);
	unsigned int GetID() { return mID; }
private:
	unsigned int mID;
};

class TextureCube {
public:
	TextureCube(int width, int height);

	void GenerateMipmaps();

	void Bind(int slot);
	unsigned int GetID() { return mID; }
private:
	unsigned int mID;

};


class Mesh {
public:
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices)
	{
		this->mVertices = vertices;
		this->mIndices = indices;
		mMaterial.Albedo = Vector3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);

		InitMesh();
	}

	unsigned int VBO, EBO;
	vector<Vertex> mVertices;
	vector<unsigned int> mIndices;
	unsigned int VAO;

	struct Material {
		Vector3 Albedo = Vector3(1);
		float Metallic = 0.1;
		float Roughness = 1;
		float Emission = 0;
	}mMaterial;

	void InitMesh();
};

class Model {
public:
	Model(string const& path) { LoadModel(path); };
	vector<MeshPtr>& GetMesh() { return mMesh; }
private:
	void LoadModel(string const& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	MeshPtr ProcessMesh(aiMesh* mesh, const aiScene* scene);
private:
	vector<MeshPtr> mMesh;
};


