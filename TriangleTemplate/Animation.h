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

enum btnType{ RECORD, START, STOP, SAVE, CLEAR, TIMELINE};
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
		// texture
		unsigned int texture;
		unsigned int texture1;

		glm::mat4 model;
	} Shape;

public:
	Button();
	Button(float w, float h, btnType t);
	Button(glm::vec2 c, float w, float h, btnType t);

	void InitVAOandVBO();
	void InitFinish();
	bool Collider(int x, int y);
	virtual void Render(Shader shader);
	virtual bool Click(int x, int y);

	Shape m_shape;
	btnType type;
	glm::vec2 center;

	float width;
	float height;
	bool keep = false;
	bool clickf = false;
	bool initf = false;
	int total_cd = 8;
	int cd = 8;
};

class AnimControlPoint {
public:
	AnimControlPoint();
	AnimControlPoint(vector<CtrlPoint> cps);

	vector<CtrlPoint> keyPoints;
};

class AnimationData {
public:
	AnimationData();
	AnimationData(map<float, AnimControlPoint*> frames, vector<float> indexs);

	map<float, AnimControlPoint*> keyframes;
	vector<float> framesIndex;
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
	void SetKeyFrame(vector<CtrlPoint> cps, bool init);
	void SetAnimation(map<float, AnimControlPoint*> frames, vector<float> indexs);
	void Clear();
	//void AnimationParser(string path);
	AnimationData* SaveAnimation(int index);

	void Render(Shader shader, Shader textureShader, ARAPTool* a);
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
	Animation(TimeLine* tl, Button* rec, Button* star, Button* stop, Button* save_btn, Button* clear_btn);

	void InitFinish();
	void Render(Shader normalShader, Shader textureShader, ARAPTool* a);
	int Click(int state, int x, int y);
	void SetKeyFrame(vector<CtrlPoint> cps, bool init = false);
	void AnimationParser();
	void OnAnimationListChange(int index);
	vector<glm::vec2> GetCpsPos();

	TimeLine* timeLine;
	Button* record_btn;
	Button* start_btn;
	Button* stop_btn;
	Button* save_btn;
	Button* clear_btn;

	int animIndex = 0;
	vector<AnimationData*> animationList;
	AnimState animState = AnimState::NONE;
	std::vector<CtrlPoint> lastCPs;
};

