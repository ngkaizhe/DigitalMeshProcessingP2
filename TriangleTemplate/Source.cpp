#include"stdio.h"
#include"Common.h"
#include "ViewManager.h"
#include"Shader.h"
#include <AntTweakBar.h>

#include "ARAPTool.h"
#include "vavImage.h"
#include "TriangulationCgal.h"
#include "GUA_OM.h"

using namespace glm;
using namespace std;

float			aspect;

Shader shader;

unsigned int VAO;

vavImage* ImageEdge = NULL;	   //find contour
TriangulationCgal* Triangulate = NULL;	   //Delaunay triangulation	

// rigid tool
ARAPTool* Arap = NULL;
// the mesh we used
Tri_Mesh* test_1 = NULL;

Triangles m_triangles;
std::vector<Vector2> vertices;
int flag = -1;
int MouseX;
int MouseY;
int offsetX = 330;
int offsetY = 0;

struct Mode_Display {
	bool openImg;
	bool triangulation;
};

Mode_Display Current_Display;

// selection part
enum PictureSelectionMode
{
	Gingerman,
	Gingerman2,
};
PictureSelectionMode pictureSelectionMode = Gingerman;

TwEnumVal pictureSelectionModeEV[] = {
	{Gingerman, "Ginger Man"},
	{Gingerman2, "Ginger Man2"},
};
TwType pictureSelectionModeType;

void LoadImg(string path);
void Triangulation();

int search(Vector2 pData, std::vector<Vector2>& vertices)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i][0] == pData[0] && vertices[i][1] == pData[1])
			return i;
}
	vertices.push_back(pData);
	return -1;
}

void SetupGUI() {
#ifdef _MSC_VER
	TwInit(TW_OPENGL, NULL);
#else
	TwInit(TW_OPENGL_CORE, NULL);
#endif
	TwGLUTModifiersFunc(glutGetModifiers);
	TwBar* bar = TwNewBar("Project2");
	TwDefine(" 'Project2' size='230 90' ");
	TwDefine(" 'Project2' fontsize='3' color='96 216 224'");


	 //Defining season enum type
	 pictureSelectionModeType = TwDefineEnum("SelectionModeType", pictureSelectionModeEV, 2);
	 //Adding season to bar
	 TwAddVarRW(bar, "SelectionMode", pictureSelectionModeType, &pictureSelectionMode, NULL);
}

void My_Init()
{
	ImageEdge = new vavImage;
	test_1 = new Tri_Mesh;

	LoadImg("../Assets/pictures/gingerbread_man.bmp");
	Triangulation();

	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	shader = Shader("../Assets/shaders/vertex.vs.glsl", "../Assets/shaders/fragment.fs.glsl");
}

void LoadImg(string path) {
	Current_Display.openImg = 1;

	ImageEdge->ReadImage(path);
	*ImageEdge = (ImageEdge->CannyEdge());

	std::cout << "Load img :" << ImageEdge->GetHeight() << "*" << ImageEdge->GetWidth() << std::endl;
}

void Triangulation() {//CGAL Delaunay Triangulation
	if (Current_Display.openImg)
	{
		Current_Display.triangulation = 1;
		Current_Display.openImg = 0;
		Triangulate = new TriangulationCgal;
	//	Vector2s meshPointset;
		Vector2s ContourPoint = ImageEdge->GetContour();

		for (int i = 0; i < ContourPoint.size(); i += 15)
		{
			Triangulate->AddPoint(ContourPoint[i][0], ContourPoint[i][1]);
		}

		Triangulate->DelaunayMesher2();

		Triangles Tris = Triangulate->GetTriangles();
		for (int i = 0; i < Tris.size(); i++)
		{
			if (!ImageEdge->IsinsidePoint(Tris[i].m_Points[0][0], Tris[i].m_Points[0][1],
				Tris[i].m_Points[1][0], Tris[i].m_Points[1][1],
				Tris[i].m_Points[2][0], Tris[i].m_Points[2][1]))
				continue;
			m_triangles.push_back(Tris[i]);
		}
		std::cout << "m_triangles.size() = " << m_triangles.size() << std::endl;

		std::vector<OMT::VertexHandle> face_vhandles;
		std::vector<OMT::VertexHandle> vecVH;
		int idx[3];
		for (int i = 0; i < m_triangles.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				Vector2 point2D = m_triangles[i].m_Points[j];
				if (search(point2D, vertices) == -1)
				{
					idx[j] = vecVH.size();
					vecVH.push_back(test_1->add_vertex(OMT::Point(point2D[0], point2D[1], 0)));
				}
				else
					idx[j] = search(point2D, vertices);
			}
			face_vhandles.clear();
			face_vhandles.push_back(vecVH[idx[0]]);
			face_vhandles.push_back(vecVH[idx[1]]);
			face_vhandles.push_back(vecVH[idx[2]]);
			test_1->add_face(face_vhandles);
		}

		// init our rigid model from the triangulated mesh
		Arap = new ARAPTool(test_1);
		
		// save the mesh to test.obj
		if (SaveFile("../Assets/Model/test.obj", Arap->GetMesh()))
			std::cout << "Success to save test.obj" << std::endl;
		else
			std::cout << "Failed to save test.obj" << std::endl;

	}
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Arap->Render(shader);

/*	if (Current_Display.triangulation)
	{
		Arap->Render();
	}*/
	// draw gui
	TwDraw();
	///////////////////////////	
	glutSwapBuffers();
}

//Call to resize the window
void My_Reshape(int width, int height)
{
	aspect = width * 1.0f / height;
	glViewport(0, 0, width, height);

	// Send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

//Timer event
void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(16, My_Timer, val);
}

//Mouse event
void My_Mouse(int button, int state, int x, int y)
{
	if (!TwEventMouseButtonGLUT(button, state, x, y)) {

		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				MouseX = x;
				MouseY = y;
				flag = Arap->GetVertex(x - offsetX, y - offsetY);						 //get control point ID

				printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
			}
			else if (state == GLUT_UP)
			{

				if(x == MouseX && MouseY == y)
					Arap->OnMouse(x - offsetX, y - offsetY, CtrlOP::Add);
				printf("Mouse %d is released at (%d, %d)\n", button, x, y);
			}
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			Arap->OnMouse(x - offsetX, y - offsetY, CtrlOP::Remove);
			printf("Mouse %d is pressed\n", button);
		}

		printf("%d %d %d %d\n", button, state, x, y);
	}
}

//Keyboard event
void My_Keyboard(unsigned char key, int x, int y)
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	}
}

//Special key event
void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}


void My_Mouse_Moving(int x, int y) {
	if (!TwEventMouseMotionGLUT(x, y)) {
		if (flag != -1)
		{
			Arap->OnMotion(x- offsetX, y- offsetY, flag);//0.008s
		}
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	//Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 600);
	glutCreateWindow("Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	// setup for antweakbar
	SetupGUI();

	//Print debug information 
	ViewManager::DumpInfo();
	////////////////////

	//Call custom initialize function
	My_Init();

	//Register GLUT callback functions
	////////////////////
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(16, My_Timer, 0);
	glutPassiveMotionFunc(My_Mouse_Moving);
	glutMotionFunc(My_Mouse_Moving);
	////////////////////

	// Enter main event loop.
	glutMainLoop();

	return 0;
}