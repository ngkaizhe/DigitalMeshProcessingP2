#pragma once
#include"Common.h"

#include<string>
#include<vector>

#include"Shader.h"
using namespace std;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	string type;
	string path;  // we store the path of the texture to compare with other textures
};

struct Material {
	Material() {
		ambient = glm::vec3(0.0f);
		diffuse = glm::vec3(0.0f);
		specular = glm::vec3(0.0f);
		shininess = 0.0f;
	}

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float shininess;
};

class Mesh
{
public:
	// Mesh Data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// or texture are not used, so probably use mat
	Material material;

	// Function
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, Material material);
	void Draw(Shader shader);
private:
	// Render data
	unsigned int VAO, VBO, EBO;
	// Function
	void setupMesh();
};

