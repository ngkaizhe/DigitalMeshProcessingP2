#pragma once
#include "Common.h"
#include "Shader.h"
#include "ARAPTool.h"
#include <vector>
#include <map>

using namespace std;

extern float imageScale;
extern int ScreenWidth;
extern int ScreenHeight;

enum btnType{ RECORD, START, STOP, TIMELINE};
enum AnimState {NONE, RECORDING, PLAYING};

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
	AnimControlPoint(vector<CtrlPoint> cps);

	vector<CtrlPoint> keyPoints;
};


class TimeLine :public Button {
public:

	TimeLine();
	TimeLine(float w, float h, float sp);
	TimeLine(glm::vec2 c, float w, float h, float sp);

	void SetKeyTime();
	void BindKey();
	void CheckIndex();

	bool Click(int x, int y);
	void SetKeyFrame(vector<CtrlPoint> cps);

	void Render(Shader shader, ARAPTool* a);
	void Play(bool f);
	void SetSpeed(float sp);
private:
	GLuint line_vao;
	GLuint key_vao;

	map<float, AnimControlPoint*> keyframes;
	vector<float> framesIndex;
	float key_time;
	float speed;
	bool isplay = false;
	int key_index = 0;
};

class Animation
{
public:
	Animation();
	Animation(TimeLine* tl, Button* rec, Button* star, Button* stop);

	void Render(Shader shader, ARAPTool* a);
	int Click(int state, int x, int y);
	void SetKeyFrame(vector<CtrlPoint> cps);

	TimeLine* timeLine;
	Button* record_btn;
	Button* start_btn;
	Button* stop_btn;

	AnimState animState = AnimState::NONE;
	std::vector<CtrlPoint> lastCPs;
};

