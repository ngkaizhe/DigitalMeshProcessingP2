#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include"Mesh.h"
#include"Shader.h"
#include<string>

using namespace std;

class Model
{
public:
	Model() {

	}

	// Function
	Model(string path) {
		loadModel(path);
	}
	void Draw(Shader shader);
private:
	// Model data
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;
	// Function
	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
	Material loadMaterialMaterial(aiMaterial* mat);
};

