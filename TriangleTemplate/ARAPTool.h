#pragma once
#include "Common.h"
#include "GUA_OM.h"
#include "Shader.h"
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <vector>

enum CtrlOP
{
	Add,
	Remove
};

struct CtrlPoint
{
	int idx;
	OMT::Point p;
};

class ARAPTool
{
public:
	ARAPTool(Tri_Mesh* mesh2D);
	~ARAPTool();

	std::vector<CtrlPoint> GetCtrlPoint();
	Tri_Mesh* GetMesh();
	bool LoadModel();
	void LoadToShader();
	void Render(Shader shader);
	void OnMotion(int x, int y, int ctrl_index);
	void OnMouse(int x, int y, CtrlOP op);
	void SetCtrlPoints(std::vector<CtrlPoint> pointsToSet);
	int GetVertex(int x, int y);


private:
	Tri_Mesh* mesh;

	Eigen::SparseMatrix<double> BigG, Gprime, B;
	Eigen::SparseMatrix<double> BigH, Hprime, D;
	std::vector< Eigen::MatrixXd* > F, K, invF;

	bool beingDragged;

	std::vector<int> ctrlPoints;
	std::vector<int> flags;
	std::vector<OMT::Point> saveLastFlagPosition;
	std::vector< std::vector<OMT::Vec2d> > fittedVertices;
	std::vector<OMT::Vec2d> baseVertices;

	void getGeachTri(int* v_id, Eigen::MatrixXd& G);
	void preCompG();
	void preStep1();
	void step1();

	void getFeachTri(int* v_id, Eigen::MatrixXd& K, Eigen::Matrix4d& F);
	void preCompF();
	void getHeachTri(int* v_id, Eigen::MatrixXd& H);
	void preCompH();
	void preStep2();
	void step2();

	void deform();

	GLuint vao;
	GLuint ebo;
	GLuint vboVertices, vboNormal, vboTexCoord;

	// the original image scale
	float xScale;
	float yScale;

	bool hasLoaded = false;
};

