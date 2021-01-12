#include "Animation.h"
#include <iostream>
#include <algorithm>    // std::find
#include <fstream>
#include <string> 
#include <io.h>

using namespace std;

Animation::Animation() {

}

Animation::Animation(TimeLine* tl, Button* rec, Button* star, Button* stop, Button* save, Button* clear) {
	timeLine = tl;
	record_btn = rec;
	start_btn = star;
	stop_btn = stop;
	save_btn = save;
	clear_btn = clear;

	timeLine->InitVAOandVBO();
	record_btn->InitVAOandVBO();
	start_btn->InitVAOandVBO();
	stop_btn->InitVAOandVBO();
	save_btn->InitVAOandVBO();
	clear_btn->InitVAOandVBO();
}

void Animation::InitFinish() {
	timeLine->InitFinish();
	record_btn->InitFinish();
	start_btn->InitFinish();
	stop_btn->InitFinish();
	save_btn->InitFinish();
	clear_btn->InitFinish();
}

void Animation::Render(Shader shader, ARAPTool* a) {
	timeLine->Render(shader, a);
	record_btn->Render(shader);
	start_btn->Render(shader);
	stop_btn->Render(shader);
	save_btn->Render(shader);
	clear_btn->Render(shader);
}

int Animation::Click(int state, int x, int y) {
	if (state == GLUT_UP) return false;
	bool record_flag = false;
	bool start_flag = false;
	bool stop_flag = false;
	bool save_flag = false;
	bool clear_flag = false;
	bool timeLine_flag = false;

	record_flag = record_btn->Click(x, y);
	start_flag = start_btn->Click(x, y);
	stop_flag = stop_btn->Click(x, y);
	save_flag = save_btn->Click(x, y);
	clear_flag = clear_btn->Click(x, y);


	if(!record_flag && !start_flag && !stop_flag && !save_flag && !clear_flag)
		timeLine_flag = timeLine->Click(x, y);

	if (record_flag) {
		animState = AnimState::RECORDING;

		return 1;
	}else if (start_flag) {
		animState = AnimState::PLAYING;
		timeLine->Play(true);
		return 2;
	}else if (stop_flag) {
		animState = AnimState::NONE;
		timeLine->Play(false);
		return 3;
	}
	else if (save_flag) {
		animationList.push_back(timeLine->SaveAnimation(animationList.size()));
		cout << "Save Animation!\n";
		return 2;
	}
	else if (clear_flag) {
		cout << "Clear timeLine!\n";
		timeLine->Clear();
		return 3;
	}
	else if (timeLine_flag) {
		return 4;
	}
	return 0;
	//return record_flag | start_flag | stop_flag | timeLine_flag;
}

void Animation::SetKeyFrame(vector<CtrlPoint> cps, bool init) {
	timeLine->SetKeyFrame(cps, init);
}

void Animation::AnimationParser() {
	cout << "AnimationParser!\n";
	string path = "../Assets/AnimationList/";
	for (int i = 0; i < 2; i++) {
		ifstream fin(path + std::to_string(i) + ".txt");
		string str;
		int keyPointNum;
		int key_controlpointNum;

		fin >> str;	// 動畫有幾個keyframes
		keyPointNum = stoi(str);
		fin >> str; // 每個key有幾個control point
		key_controlpointNum = stoi(str);

		int state = 0;
		map<float, AnimControlPoint*> frames;
		vector<float> indexs;

		for (int k = 0; k < keyPointNum; k++) {
			fin >> str;	// time

			vector<CtrlPoint> keyPoints;
			int time = stoi(str);

			indexs.push_back(time);
			for (int m = 0; m < key_controlpointNum; m++) {
				fin >> str;
				int idx = stoi(str);

				fin >> str;
				int x = stoi(str);

				fin >> str;
				int y = stoi(str);

				CtrlPoint cp;
				cp.idx = idx;
				cp.p = OMT::Point(x, y, 0);
				keyPoints.push_back(cp);
			}
			AnimControlPoint* ap = new AnimControlPoint(keyPoints);
			frames[time] = ap;
		}
		fin.close();

		AnimationData* ad = new AnimationData(frames, indexs);
		animationList.push_back(ad);
	}
	timeLine->SetAnimation(animationList[animIndex]->keyframes, animationList[animIndex]->framesIndex);
}

void Animation::OnAnimationListChange(int index) {
	if (index > animationList.size()) {
		cout << "Animation Index Out of range!\n";
		return;
	}
	animIndex = index;
	timeLine->SetAnimation(animationList[animIndex]->keyframes, animationList[animIndex]->framesIndex);
}

vector<glm::vec2> Animation::GetCpsPos() {
	AnimationData* ad = animationList[animIndex];
	vector<CtrlPoint> cps = ad->keyframes[ad->framesIndex[0]]->keyPoints;
	vector<glm::vec2> pos;
	for (int i = 0; i < cps.size(); i++) {
		pos.push_back(glm::vec2(cps[i].p[0], cps[i].p[1]));
	}
	return pos;
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

void Button::InitVAOandVBO() {
	float screenWidth = ScreenWidth / 2.0;
	float screenHeight = ScreenHeight / 2.0;

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
// Positions   // TexCoords
(center.x - width - screenWidth) / screenWidth,  (center.y + height - screenHeight)/ screenHeight,  0.0f, 1.0f,
(center.x - width - screenWidth) / screenWidth, (center.y - height - screenHeight) / screenHeight,  0.0f, 0.0f,
 (center.x + width - screenWidth) / screenWidth, (center.y - height - screenHeight) / screenHeight,  1.0f, 0.0f,

(center.x - width - screenWidth) / screenWidth,  (center.y + height - screenHeight) / screenHeight,  0.0f, 1.0f,
 (center.x + width - screenWidth) / screenWidth, (center.y - height - screenHeight) / screenHeight,  1.0f, 0.0f,
 (center.x + width - screenWidth) / screenWidth, (center.y + height - screenHeight) / screenHeight,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_shape.vao);
	glBindVertexArray(m_shape.vao);

	glGenBuffers(3, &m_shape.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
}

void Button::InitFinish() {
	initf = true;
}

bool Button::Collider(int x, int y) {
	
	y = ScreenHeight - y;

	if (x <= center.x + width && x >= center.x - width && y <= center.y + height && y >= center.y - height) {
		return true;
	}
	return false;
}

void Button::Render(Shader shader) {
	if (!initf)
		return;
	//ReBind();
	glm::mat4 modelMat = glm::mat4(1.0);
	// set the model matrix value
	shader.setUniformMatrix4fv("model", modelMat);

	glBindVertexArray(m_shape.vao);
	shader.use();
	switch (type) {
	case btnType::RECORD:
		shader.setUniform3fv("color", glm::vec3(1.0, 0.0, 0.0));
		break;
	case btnType::START:
		shader.setUniform3fv("color", glm::vec3(0, 0.96, 0.49));
		break;
	case btnType::STOP:
		shader.setUniform3fv("color", glm::vec3(1.0, 0.0, 1.0));
		break;
	case btnType::SAVE:
		shader.setUniform3fv("color", glm::vec3(0.0, 0.0, 1.0));
		break;
	case btnType::CLEAR:
		shader.setUniform3fv("color", glm::vec3(0.0, 0.0, 0.0));
		break;
	case btnType::TIMELINE:
		shader.setUniform3fv("color", glm::vec3(0.9, 0.9, 0.9));
		break;
	}
	//shader.setUniform3fv("color", glm::vec3(1.0, 0.96, 0.49));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}

bool Button::Click(int x, int y) {
	string btnN = "";

	switch (type) {
	case btnType::RECORD:
		btnN = "record";
		break;
	case btnType::START:
		btnN = "start";
		break;
	case btnType::STOP:
		btnN = "stop";
		break;
	case btnType::TIMELINE:
		btnN = "timeLine";
		break;
	}

	if (Collider(x, y)) {
		cout << "btn " << btnN << "\n";
		return true;
		//Do click
	}
	return false;
}

AnimControlPoint::AnimControlPoint() {

}

AnimControlPoint::AnimControlPoint(vector<CtrlPoint> cps) {
	keyPoints = cps;
}

TimeLine::TimeLine() {
	center = glm::vec2(0, 0);
	width = 1.0;
	height = 1.0;
	key_time = 50;
	speed = 10.0;
	type = btnType::TIMELINE;
}

TimeLine::TimeLine(float w, float h, float sp = 10.0) {
	center = glm::vec2(0, 0);
	width = w;
	height = h;
	key_time = 50;
	speed = sp;
	type = btnType::TIMELINE;
}

TimeLine::TimeLine(glm::vec2 c, float w, float h, float sp = 10.0) {
	center = c;
	width = w;
	height = h;
	key_time = 50;
	speed = sp;
	type = btnType::TIMELINE;
}

void TimeLine::SetKeyTime() {
	float screenWidth = ScreenWidth / 2.0;
	float screenHeight = ScreenHeight / 2.0;

	// rebind line_vao starts
	// vao
	glGenVertexArrays(1, &line_vao);
	// faces
	glGenBuffers(1, &m_shape.ebo);
	// vertices
	glGenBuffers(1, &m_shape.vbo);

	vector<glm::vec3> edges;
	glm::vec3 lt = glm::vec3((key_time - screenWidth) / screenWidth, (center.y + height - screenHeight) / screenHeight, 0.0);
	glm::vec3 lb = glm::vec3((key_time - screenWidth) / screenWidth, (center.y - height - screenHeight) / screenHeight, 0.0);
	edges.push_back(lt);
	edges.push_back(lb);

	std::vector<unsigned int> line_indices;
	line_indices.reserve(2);
	line_indices.push_back(0);
	line_indices.push_back(1);


	glBindVertexArray(line_vao);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * edges.size(), &edges[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// lines
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shape.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * line_indices.size(), &line_indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// rebind line_vao ends
}

void TimeLine::BindKey() {

	std::vector<glm::vec3> vertices;
	vertices.reserve(framesIndex.size());
	float scrW = ScreenWidth / 2.0;
	float scrH = ScreenHeight / 2.0;

	for (int i = 0; i < framesIndex.size(); i++) {
		glm::vec3 v1 = glm::vec3((framesIndex[i] - scrW)/ scrW, (center.y- scrH) / scrH, 0);
		vertices.push_back(v1);
	}

	glGenVertexArrays(1, &key_vao);
	glGenBuffers(1, &m_shape.vbo);

	// bind the vao
	glBindVertexArray(key_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void TimeLine::CheckIndex() {
	for (int i = 0; i < framesIndex.size(); i++) {
		if (key_time > framesIndex[i]) {
			key_index = i;
		}
	}
}

bool TimeLine::Click(int x, int y) {
	if (Collider(x, y)) {
		cout << "Click " << "timeLine" << " button!\n";
		key_time = x;
		return true;
	}
	return false;
}

void TimeLine::Play(bool f) {
	cout << "Play " << f << "\n";

	isplay = f;
}

void TimeLine::Render(Shader shader, ARAPTool* arap) {
	if (!initf)
		return;

	if (isplay) {
		key_time += speed;
		if (key_time >= ScreenWidth) {
			key_time -= ScreenWidth;
			key_index = 0;
		}
	}

	if (framesIndex.size() > 0) {
		CheckIndex();
		AnimControlPoint* AP = keyframes[framesIndex[key_index]];
	//	cout << "AP keyPoints size : " << AP->keyPoints.size() << "\n";
		for (int i = 0; i < AP->keyPoints.size(); i++) {
			int x = AP->keyPoints[i].p[0];
			int y = AP->keyPoints[i].p[1];
			if ((key_index + 1) < framesIndex.size()) {
				AnimControlPoint* next_AP = keyframes[framesIndex[key_index + 1]];
				float w = (key_time - framesIndex[key_index]) / (framesIndex[key_index + 1] - framesIndex[key_index]);
				x = AP->keyPoints[i].p[0] * (1.0 - w) + next_AP->keyPoints[i].p[0] * w;
				y = AP->keyPoints[i].p[1] * (1.0 - w) + next_AP->keyPoints[i].p[1] * w;
			}
			arap->OnMotion(x, y, AP->keyPoints[i].idx);
		}
	}

	

	glm::mat4 modelMat = glm::mat4(1.0);
	// set the model matrix value
	shader.setUniformMatrix4fv("model", modelMat);

	glBindVertexArray(m_shape.vao);
	shader.use();
	shader.setUniform3fv("color", glm::vec3(1.0, 1.0, 1.0));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	if (framesIndex.size() > 0) {
		glPointSize(8.0);
		shader.setUniform3fv("color", glm::vec3(1.0, 0.0, 0.0));
		glBindVertexArray(key_vao);
		glDrawArrays(GL_POINTS, 0, framesIndex.size());
		glBindVertexArray(0);
	}

	SetKeyTime();

	shader.setUniform3fv("color", glm::vec3(1.0, 0, 0));
	glBindVertexArray(line_vao);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void TimeLine::SetKeyFrame(vector<CtrlPoint> cps, bool init = false) {
	if (init)
		key_time = 0;
	AnimControlPoint* ap = new AnimControlPoint(cps);
	keyframes[key_time] = ap;

	std::vector<float>::iterator it;
	it = std::find(framesIndex.begin(), framesIndex.end(), key_time);
	if (it == framesIndex.end()) {
		framesIndex.push_back(key_time);
		sort(framesIndex.begin(), framesIndex.end());
	}
	BindKey();
}

void TimeLine::SetAnimation(map<float, AnimControlPoint*> frames, vector<float> indexs) {
	keyframes = frames;
	framesIndex = indexs;
	BindKey();
}

void TimeLine::Clear() {

	framesIndex.clear();
}

AnimationData* TimeLine::SaveAnimation(int index) {
	string str = std::to_string(index);
	ofstream fout("../Assets/AnimationList/" + str + ".txt");
	fout << framesIndex.size() << " " << keyframes[framesIndex[0]]->keyPoints.size() << "\n";
	for (int i = 0; i < framesIndex.size(); i++) {
		fout << framesIndex[i] << "\n";
		vector<CtrlPoint> keyPoints = keyframes[framesIndex[i]]->keyPoints;
		for (int j = 0; j < keyPoints.size(); j++) {
			fout << keyPoints[j].idx << " " << keyPoints[j].p[0] << " " << keyPoints[j].p[1] << "\n";
		}
	}
	fout.close();
	AnimationData* ad = new AnimationData(keyframes, framesIndex);
	return ad;
}

void TimeLine::SetSpeed(float sp) {
	speed = sp;
}

AnimationData::AnimationData() {

}

AnimationData::AnimationData(map<float, AnimControlPoint*> frames, vector<float> indexs) {
	keyframes = frames;
	framesIndex = indexs;
}
