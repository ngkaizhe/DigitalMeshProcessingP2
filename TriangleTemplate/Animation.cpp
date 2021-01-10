#include "Animation.h"

Animation::Animation() {

}

Animation::Animation(TimeLine* tl, Button* rec, Button* star, Button* stop) {
	timeLine = tl;

	record_btn = rec;
	start_btn = star;
	stop_btn = stop;
}

void Animation::Render() {
	timeLine->Render();
	record_btn->Render();
	start_btn->Render();
	stop_btn->Render();
}


Button::Button() {

}

Button::Button(float w, float h, btnType t) {
	center = glm::vec2(0, 0);
	width = w;
	height = h;
	type = t;
}

Button::Button(glm::vec2 c, float w, float h, btnType t) {
	center = c;
	width = w;
	height = h;
	type = t;
}

bool Button::Collider(int x, int y) {
	if (x <= center.x + width && x >= center.x - width && y <= center.y + height && y >= center.y - height) {
		return true;
	}
	return false;
}

void Button::Render() {
	float z = -1.0;
	switch (type) {
	case btnType::Record:
		glColor3f(1.0, 0.0, 0.1);
		z = -2.0;
		break;
	case btnType::Start:
		z = -2.0;
		glColor3f(0.1, 0.0, 1.0);
		break;
	case btnType::Stop:
		z = -2.0;
		glColor3f(0.1, 1.0, 0.1);
		break;
	default:
		glColor3f(1.0, 1.0, 1.0);
		break;
	}

	glBegin(GL_QUADS);                      // Draw A Quad
		glVertex3f(center.x - width, center.y + height, z);              // Top Left
		glVertex3f(center.x + width, center.y + height, z);              // Top Right
		glVertex3f(center.x + width, center.y - height, z);              // Bottom Right
		glVertex3f(center.x - width, center.y - height, z);              // Bottom Left
	glEnd();
}

void Button::Click(int x, int y) {
	if (Collider(x, y)) {
		//Do click
	}
}

AnimControlPoint::AnimControlPoint() {

}


AnimControlPoint::AnimControlPoint(glm::vec2 p, vector<glm::vec2>cps) {
	ui_pos = p;
	mesh_cps = cps;
}

TimeLine::TimeLine() {
	center = glm::vec2(0, 0);
	width = 1.0;
	height = 1.0;
	linePos = 0;
	speed = 1.0;
}

TimeLine::TimeLine(float w, float h, float sp = 1.0) {
	center = glm::vec2(0, 0);
	width = w;
	height = h;
	linePos = 0;
	speed = sp;
}

TimeLine::TimeLine(glm::vec2 c, float w, float h, float sp = 1.0) {
	center = c;
	width = w;
	height = h;
	linePos = 0;
	speed = sp;
}

void TimeLine::Click(int x, int y) {
	if (Collider(x, y)) {
		linePos = x;
	}
}

void TimeLine::AddControlPoint(AnimControlPoint cp) {
	cps.push_back(cp);
}

void TimeLine::SetSpeed(float sp) {
	speed = sp;
}

