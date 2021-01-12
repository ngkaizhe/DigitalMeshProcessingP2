#include"stdio.h"
#include"Common.h"
#include"Shader.h"
#include <AntTweakBar.h>
#include"ViewManager.h"
#include "Animation.h"
#include "ARAPTool.h"
#include "vavImage.h"
#include "TriangulationCgal.h"
#include "GUA_OM.h"


using namespace glm;
using namespace std;

float			aspect;
Shader normalShader;
Shader textureShader;

unsigned int VAO;

// screen size
int ScreenHeight = 800;
int ScreenWidth = 600;

// picture size
int pictureHeight;
int pictureWidth;

// pixel space to clip space
glm::mat4 pixelToClip;

// image scale
float imageScale = 0.5;

vavImage* ImageEdge = NULL;	   //find contour
TriangulationCgal* Triangulate = NULL;	   //Delaunay triangulation	


//Anim tool
Animation* anim = NULL;
// rigid tool
ARAPTool* Arap = NULL;
// the mesh we used
Tri_Mesh* test_1 = NULL;

Triangles m_triangles;
std::vector<Vector2> vertices;
int flag = -1;
int animIndex = 0;
int MouseX;
int MouseY;

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

enum AnimationSelectionMode
{
	Walk,
	Jump,
};
AnimationSelectionMode animSelectionMode = Walk;


TwEnumVal pictureSelectionModeEV[] = {
	{Gingerman, "Ginger Man"},
	{Gingerman2, "Ginger Man2"},
};

TwEnumVal animSelectionModeEV[] = {
	{Walk, "Walk"},
	{Jump, "Jump"},
};

TwType pictureSelectionModeType;
TwType animSelectionModeType;

void LoadImg(string path);
void Triangulation();

void DumpInfo(void);
glm::vec2 CalculateScreenSpaceToPixelSpace(glm::vec2 ScreenSpaceValue);

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
	 animSelectionModeType = TwDefineEnum("AnimSelectionModeType", animSelectionModeEV, 2);

	 //Adding season to bar
	 TwAddVarRW(bar, "SelectionMode", pictureSelectionModeType, &pictureSelectionMode, NULL);
	 TwAddVarRW(bar, "Animation List", animSelectionModeType, &animSelectionMode, NULL);

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

	normalShader = Shader("../Assets/shaders/vertex.vs.glsl", "../Assets/shaders/fragment.fs.glsl");
	textureShader = Shader("../Assets/shaders/texture_vertex.vs.glsl", "../Assets/shaders/texture_fragment.fs.glsl");

	int hpos = 100;
	int btn_hPos = 180;
	int btn_w = 10;
	int btn_h = 10;
	// init animation tool
	TimeLine* timeline = new TimeLine(glm::vec2(300, hpos), 300, 100, 1.0);
	Button* record_btn = new Button(glm::vec2(520, btn_hPos), btn_w, btn_h, btnType::RECORD);
	Button* start_btn = new Button(glm::vec2(550, btn_hPos), btn_w, btn_h, btnType::START);
	Button* stop_btn = new Button(glm::vec2(580, btn_hPos), btn_w, btn_h, btnType::STOP);
	Button* save_btn = new Button(glm::vec2(20, btn_hPos), btn_w, btn_h, btnType::SAVE);
	Button* clear_btn = new Button(glm::vec2(50, btn_hPos), btn_w, btn_h, btnType::CLEAR);

	anim = new Animation(timeline, record_btn, start_btn, stop_btn, save_btn, clear_btn);
	anim->AnimationParser();
	Arap->SetControlPoints(anim->GetCpsPos());
	anim->InitFinish();
}

void LoadImg(string path) {
	Current_Display.openImg = 1;

	ImageEdge->ReadImage(path);
	*ImageEdge = (ImageEdge->CannyEdge());

	std::cout << "Load img :" << ImageEdge->GetHeight() << "*" << ImageEdge->GetWidth() << std::endl;

	pictureHeight = ImageEdge->GetWidth();
	pictureWidth = ImageEdge->GetHeight();
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
		Arap = new ARAPTool(test_1, pictureHeight, pictureWidth);
		
		// save the mesh to test.obj
		if (SaveFile("../Assets/Model/test.obj", Arap->GetMesh()))
			std::cout << "Success to save test.obj" << std::endl;
		else
			std::cout << "Failed to save test.obj" << std::endl;

	}

}

glm::vec2 CalculateScreenSpaceToPixelSpace(glm::vec2 ScreenSpaceValue) {
	// set to homogenous vector
	glm::vec4 homogenous_vec4 = glm::vec4(ScreenSpaceValue.x, ScreenSpaceValue.y, 0, 1);

	// move the screen space to clip space which
	// x in range [-1, 1]
	// y in range [-1, 1]
	glm::mat4 toClipSpaceMat = glm::mat4(1.0);
	// normalize the flip y axis
	toClipSpaceMat = glm::scale(toClipSpaceMat, glm::vec3(1.0 / ScreenWidth * 2, -1.0 / ScreenHeight * 2, 1));
	// move to middle
	toClipSpaceMat = glm::translate(toClipSpaceMat, glm::vec3(-ScreenWidth / 2.0, -ScreenHeight / 2.0, 0));
	homogenous_vec4 = toClipSpaceMat * homogenous_vec4;

	// move the clip space to pixel space
	glm::mat4 inverseMat = glm::inverse(pixelToClip);
	homogenous_vec4 = inverseMat * homogenous_vec4;

	return glm::vec2(homogenous_vec4.x, homogenous_vec4.y);
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	if (animIndex != animSelectionMode) {
		animIndex = animSelectionMode;
		anim->OnAnimationListChange(animIndex);
	}

	Arap->Render(normalShader, textureShader);

/*	if (Current_Display.triangulation)
	{
		Arap->Render();
	}*/
	// draw gui
	anim->Render(normalShader, textureShader, Arap);
	TwDraw();
	///////////////////////////	
	glutSwapBuffers();
}

//Call to resize the window
void My_Reshape(int width, int height)
{
	ScreenWidth = width;
	ScreenHeight = height;

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
		int f = anim->Click(state, x, y);
		if (f > 0) {
			switch (f) {
			case 1:		//click record btn
				anim->SetKeyFrame(Arap->GetCtrlPoint(), true);
				break;
			case 2:		//click start btn
				break;
			case 3:		//click stop btn
				break;
			}
			return;
		}
		glm::vec2 pixelSpaceValue = CalculateScreenSpaceToPixelSpace(glm::vec2(x, y));

		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				MouseX = x;
				MouseY = y;

				flag = Arap->GetVertex(pixelSpaceValue.x, pixelSpaceValue.y);						 //get control point ID

				printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
			}
			else if (state == GLUT_UP)
			{
				if (x == MouseX && MouseY == y && anim->animState == AnimState::NONE)
					Arap->OnMouse(pixelSpaceValue.x, pixelSpaceValue.y, CtrlOP::Add);

				flag = -1;
				printf("Mouse %d is released at (%d, %d)\n", button, x, y);
			}
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			if (state == GLUT_UP && anim->animState == AnimState::NONE)
			{
				Arap->OnMouse(pixelSpaceValue.x, pixelSpaceValue.y, CtrlOP::Remove);
				printf("Mouse %d is pressed\n", button);
			}
		}
		
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
		glm::vec2 pixelSpaceValue = CalculateScreenSpaceToPixelSpace(glm::vec2(x, y));
		
		if (flag != -1)
		{
			Arap->OnMotion(pixelSpaceValue.x, pixelSpaceValue.y, flag);//0.008s
			if (anim->animState == AnimState::RECORDING) {
				anim->SetKeyFrame(Arap->GetCtrlPoint());
			}
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

	glutInitWindowSize(ScreenWidth, ScreenHeight);

	glutCreateWindow("Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	// setup for antweakbar
	SetupGUI();

	//Print debug information 
	DumpInfo();
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

// Print OpenGL context related information.
void DumpInfo(void)
{
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}