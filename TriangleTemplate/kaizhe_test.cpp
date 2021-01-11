#include"Common.h"
#include"Shader.h"
#include "GUA_OM.h"
#include"stb_image.h"

Shader textureShader1;
Tri_Mesh mesh;
unsigned int texture;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;

unsigned int VAO1;

glm::mat4 modelMat;

float xScale1 = 482;
float yScale1 = 604;

void My_Init1()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// mesh
	if (!ReadFile("../Assets/Model/test.obj", &mesh)) {
		std::cerr << "read error\n";
	}

	// shader 
	textureShader1 = Shader("../Assets/shaders/texture_vertex.vs.glsl", "../Assets/shaders/texture_fragment.fs.glsl");
	// image
	// texture part
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load("../Assets/pictures/gingerbread_man_texture.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// vertices
	vertices.reserve(mesh.n_vertices());
	for (Tri_Mesh::VIter vi = mesh.vertices_begin(); vi != mesh.vertices_end(); vi++) {
		Tri_Mesh::Point v1 = mesh.point(*vi);
		vertices.push_back(glm::vec3(v1[0], v1[1], v1[2]));
	}

	// uv
	uvs.reserve(mesh.n_vertices());
	for (Tri_Mesh::VIter vi = mesh.vertices_begin(); vi != mesh.vertices_end(); vi++) {
		Tri_Mesh::Point v1 = mesh.point(*vi);
		uvs.push_back(glm::vec2(v1[0]/ xScale1, 1 - (v1[1] / yScale1)));
	}

	// faces indices
	std::vector<unsigned int> faces_indices;
	faces_indices.reserve(mesh.n_faces() * 3);
	for (Tri_Mesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
	{
		for (Tri_Mesh::FaceVertexIter fv_it = mesh.fv_begin(*f_it); fv_it != mesh.fv_end(*f_it); ++fv_it)
		{
			faces_indices.push_back(fv_it->idx());
		}
	}


	GLuint vboVertices, vboUV, ebo;
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &vboVertices);
	glGenBuffers(1, &vboUV);
	glGenBuffers(1, &ebo);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	// uv
	glBindBuffer(GL_ARRAY_BUFFER, vboUV);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	// faces
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * faces_indices.size(), &faces_indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	modelMat = glm::mat4(1.0);
	modelMat = glm::scale(modelMat, glm::vec3(1 / xScale1 * 2 * 0.5, -1 / yScale1 * 2 * 0.5, 1));
	modelMat = glm::translate(modelMat, glm::vec3(-xScale1 / 2.0, -yScale1 / 2.0, 0));
}

// GLUT callback. Called to draw the scene.
void My_Display1()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	textureShader1.use();
	textureShader1.setUniformMatrix4fv("model", modelMat);


	// set the texture value for the texture shader
	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glBindVertexArray(VAO1);
	glDrawElements(GL_TRIANGLES, mesh.n_faces() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	
	///////////////////////////	
	glutSwapBuffers();
}

void My_Reshape1(int width, int height)
{
	glViewport(0, 0, width, height);
}

int main1(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);

	glutCreateWindow("Framework");

	glewInit();

	My_Init1();

	//Register GLUT callback functions
	////////////////////
	glutDisplayFunc(My_Display1);
	glutReshapeFunc(My_Reshape1);

	glutMainLoop();

	return 0;
}