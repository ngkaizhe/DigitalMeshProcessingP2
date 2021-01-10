#pragma once
#include "Common.h"
#include <vector>

using namespace std;

enum btnType{ Record, Start, Stop};

class Button {
public:
	Button();
	Button(float w, float h, btnType t);
	Button(glm::vec2 c, float w, float h, btnType t);

	bool Collider(int x, int y);
	virtual void Render();
	virtual void Click(int x, int y);

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

	void Click(int x, int y);
	void AddControlPoint(AnimControlPoint cp);
//	void Draw();

	void SetSpeed(float sp);
private:
	vector<AnimControlPoint> cps;
	float linePos;
	float speed;
};

class Animation
{
public:
	Animation();
	Animation(TimeLine* tl, Button* rec, Button* star, Button* stop);

	void Render();
	void Click(int x, int y);

	TimeLine* timeLine;
	Button* record_btn;
	Button* start_btn;
	Button* stop_btn;
};