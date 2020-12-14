#include"stdio.h"
#include"Common.h"
#include "ViewManager.h"
#include"Shader.h"
#include"Model.h"

#define MENU_Entry1 1
#define MENU_Entry2 2
#define MENU_EXIT   3

using namespace glm;
using namespace std;


float			aspect;
ViewManager		m_camera;

Shader shader;
Model model;

glm::mat4 translationMatrix[4];
glm::vec3 relativePos[4];


void My_Init()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	shader = Shader("../Assets/shaders/vertex.vs.glsl", "../Assets/shaders/fragment.fs.glsl");
	model = Model("../Assets/objects/nanosuit/nanosuit.obj");

	// init all translationMatrix
	for (int i = 0; i < 4; i++)
		translationMatrix[i] = mat4();

	relativePos[0] = vec3(0, 0, 0);
	relativePos[1] = vec3(1, 0, 0);
	relativePos[2] = vec3(1, 0, 0);
	relativePos[3] = vec3(1, 0, 0);

}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Update shaders' input variable
	///////////////////////////	
	shader.use();
	
	// set view matrix
	shader.setUniformMatrix4fv("view", m_camera.GetViewMatrix() * m_camera.GetModelMatrix());
	// set projection matrix
	shader.setUniformMatrix4fv("projection", m_camera.GetProjectionMatrix(aspect));

	// do something here
	// set model matrix
	shader.setUniformMatrix4fv("model", mat4(1.0));
	// draw model
	model.Draw(shader);

	///////////////////////////	

	glFlush();
	glutSwapBuffers();
}

//Call to resize the window
void My_Reshape(int width, int height)
{
	aspect = width * 1.0f / height;
	m_camera.SetWindowSize(width, height);
	glViewport(0, 0, width, height);
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
	m_camera.mouseEvents(button, state, x, y);

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		}
		else if (state == GLUT_UP)
		{
			printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		printf("Mouse %d is pressed\n", button);
	}
	printf("%d %d %d %d\n", button, state, x, y);
}

//Keyboard event
void My_Keyboard(unsigned char key, int x, int y)
{
	m_camera.keyEvents(key);
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
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

//Menu event
void My_Menu(int id)
{
	switch (id)
	{
	case MENU_Entry1:
		printf("Entry1 is selected.\n");
		break;
	case MENU_Entry2:
		printf("Entry2 is selected.\n");
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}


void My_Mouse_Moving(int x, int y) {
	m_camera.mouseMoveEvent(x, y);
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
	glutInitWindowSize(600, 600);
	glutCreateWindow("Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif

	//Print debug information 
	ViewManager::DumpInfo();
	////////////////////

	//Call custom initialize function
	My_Init();

	//Define Menu
	////////////////////
	int menu_main = glutCreateMenu(My_Menu);
	int menu_entry = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Entry", menu_entry);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_entry);
	glutAddMenuEntry("Print Entry1", MENU_Entry1);
	glutAddMenuEntry("Print Entry2", MENU_Entry2);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	////////////////////

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