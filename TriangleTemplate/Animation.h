#pragma once
#include "Common.h"
#include "Shader.h"
#include "ARAPTool.h"
#include <vector>

using namespace std;

extern float imageScale;
extern int ScreenWidth;
extern int ScreenHeight;

enum btnType{ RECORD, START, STOP, TIMELINE};

class Button {

	//model info
	typedef struct
	{
		GLuint vao;
		GLuint vbo;
		GLuint vboTex;
		GLuint ebo;

		GLuint p_normal;
		int materialId;
		int indexCount;
		glm::mat4 model;
	} Shape;

public:
	Button();
	Button(float w, float h, btnType t);
	Button(glm::vec2 c, float w, float h, btnType t);

	void InitVAOandVBO();
	bool Collider(int x, int y);
	virtual void Render(Shader shader);
	virtual bool Click(int x, int y);

	Shape m_shape;
	btnType type;
	glm::vec2 center;
	float width;
	float height;
};

class AnimControlPoint {
public:
	AnimControlPoint();
	AnimControlPoint(glm::vec2 p, vector<glm::vec2>cps);

	glm::vec2 ui_pos;
	vector<glm::vec2> mesh_cps;
};


class TimeLine :public Button {
public:

	TimeLine();
	TimeLine(float w, float h, float sp);
	TimeLine(glm::vec2 c, float w, float h, float sp);

	void SetKeyTime();
	bool Click(int x, int y);
	void AddControlPoint(AnimControlPoint cp);
	void Render(Shader shader);
	void Play(bool f);
	void SetSpeed(float sp);
private:
	GLuint line_vao;
	vector<AnimControlPoint> cps;
	float linePos;
	float speed;
	bool isplay = false;
};

class Animation
{
public:
	Animation();
	Animation(TimeLine* tl, Button* rec, Button* star, Button* stop);

	void Render(Shader shader);
	int Click(int state, int x, int y);
	void SetKeyFrame(std::vector<CtrlPoint> );

	TimeLine* timeLine;
	Button* record_btn;
	Button* start_btn;
	Button* stop_btn;
};