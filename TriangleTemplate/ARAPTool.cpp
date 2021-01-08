#include "ARAPTool.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

ARAPTool::ARAPTool(Tri_Mesh* mesh2D)
{
	mesh = mesh2D;

	beingDragged = false;
	flags.resize(mesh->n_vertices(), 0);

	saveLastFlagPosition.resize(mesh->n_vertices(), OMT::Point(0, 0, 0));

	baseVertices.reserve(mesh->n_vertices());
	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		OMT::Point p = mesh->point(*v_it);
		baseVertices.push_back(OMT::Vec2d(p[0], p[1]));
	}

	preCompG();
	preCompF();
	preCompH();

	totalCtrlPoint = 0;
}

ARAPTool::~ARAPTool()
{
}

std::vector<CtrlPoint> ARAPTool::GetCtrlPoint()
{
	std::vector<CtrlPoint> pointData;

	for (int i = 0; i < ctrlPoints.size(); ++i)
	{
		CtrlPoint ctrlPoint;
		ctrlPoint.idx = ctrlPoints[i];
		ctrlPoint.p = mesh->point(mesh->vertex_handle(ctrlPoints[i]));

		pointData.push_back(ctrlPoint);
	}

	return pointData;
}

void ARAPTool::getGeachTri(int* v_id, Eigen::MatrixXd& G)
{
	//reset G
	G.setZero();
	Eigen::MatrixXd A(2, 6);

	for (int i = 0; i < 3; ++i)
	{
		//reset A	
		A.setZero();

		int i0 = i, i1 = (i + 1) % 3, i2 = (i + 2) % 3;
		OMT::Point v0 = mesh->point(mesh->vertex_handle(v_id[i0]));
		OMT::Point v1 = mesh->point(mesh->vertex_handle(v_id[i1]));
		OMT::Point v2 = mesh->point(mesh->vertex_handle(v_id[i2]));

		OMT::Vec2d xdir(v1[0] - v0[0], v1[1] - v0[1]);
		OMT::Vec2d ydir(-xdir[1], xdir[0]);
		OMT::Vec2d v2dir(v2[0] - v0[0], v2[1] - v0[1]);
		double det = xdir[0] * ydir[1] - xdir[1] * ydir[0];
		double x01 = (v2dir[0] * ydir[1] - v2dir[1] * ydir[0]) / det;
		double y01 = (xdir[0] * v2dir[1] - xdir[1] * v2dir[0]) / det;

		//for v0'
		A.coeffRef(0, i0 * 2) = 1 - x01;
		A.coeffRef(1, i0 * 2) = -y01;
		A.coeffRef(0, i0 * 2 + 1) = y01;
		A.coeffRef(1, i0 * 2 + 1) = 1 - x01;

		//for v1'
		A.coeffRef(0, i1 * 2) = x01;
		A.coeffRef(1, i1 * 2) = y01;
		A.coeffRef(0, i1 * 2 + 1) = -y01;
		A.coeffRef(1, i1 * 2 + 1) = x01;

		//for v2'
		A.coeffRef(0, i2 * 2) = -1;
		A.coeffRef(1, i2 * 2) = 0;
		A.coeffRef(0, i2 * 2 + 1) = 0;
		A.coeffRef(1, i2 * 2 + 1) = -1;

		G += A.transpose() * A;

		{
			std::ofstream outFile("G/GG" + std::to_string(i) + ".csv");
			if (outFile.is_open())
			{
				for (int row = 0; row < G.rows(); ++row)
				{
					for (int col = 0; col < G.cols(); ++col)
					{
						outFile << std::fixed << std::setprecision(8) << G.coeff(row, col) << ",";
					}

					outFile << std::endl;
				}
			}
		}

		{
			std::ofstream outFile("G/GA" + std::to_string(i) + ".csv");
			if (outFile.is_open())
			{
				for (int row = 0; row < A.rows(); ++row)
				{
					for (int col = 0; col < A.cols(); ++col)
					{
						outFile << std::fixed << std::setprecision(8) << A.coeff(row, col) << ",";
					}

					outFile << std::endl;
				}
			}
		}
	}
}

void ARAPTool::preCompG()
{
	int nv = flags.size();
	BigG = Eigen::SparseMatrix<double>(nv * 2, nv * 2);
	BigG.setZero();

	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		Eigen::MatrixXd G(6, 6);
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		getGeachTri(v_id, G);

		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				BigG.coeffRef(v_id[j] * 2, v_id[k] * 2) += G.coeff(j * 2, k * 2);
				BigG.coeffRef(v_id[j] * 2 + 1, v_id[k] * 2) += G.coeff(j * 2 + 1, k * 2);
				BigG.coeffRef(v_id[j] * 2, v_id[k] * 2 + 1) += G.coeff(j * 2, k * 2 + 1);
				BigG.coeffRef(v_id[j] * 2 + 1, v_id[k] * 2 + 1) += G.coeff(j * 2 + 1, k * 2 + 1);
			}
		}
	}
}

void ARAPTool::preStep1()
{
	int nv = flags.size(), cur_free = 0, cur_ctrl = 0;
	std::vector<int> vert_map(nv, 0);

	for (int i = 0; i < nv; ++i)
	{
		if (flags[i] == 0)
		{
			vert_map[i] = cur_free++;
		}
		else
		{
			vert_map[i] = cur_ctrl++;
		}
	}

	if (cur_free == 0 || cur_ctrl == 0)
	{
		return;
	}

	Eigen::SparseMatrix<double> G00(cur_free * 2, cur_free * 2),
		G01(cur_free * 2, cur_ctrl * 2),
		G10(cur_ctrl * 2, cur_free * 2),
		G11(cur_ctrl * 2, cur_ctrl * 2);


	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		int m[3] = { vert_map[v_id[0]], vert_map[v_id[1]], vert_map[v_id[2]] };

		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				Eigen::SparseMatrix<double>* GG = NULL;

				if (!flags[v_id[j]] && !flags[v_id[k]])
				{
					GG = &G00;
				}
				else if (!flags[v_id[j]] && flags[v_id[k]])
				{
					GG = &G01;
				}
				else if (flags[v_id[j]] && !flags[v_id[k]])
				{
					GG = &G10;
				}
				else
				{
					GG = &G11;
				}

				GG->coeffRef(m[j] * 2, m[k] * 2) = BigG.coeff(v_id[j] * 2, v_id[k] * 2);
				GG->coeffRef(m[j] * 2 + 1, m[k] * 2) = BigG.coeff(v_id[j] * 2 + 1, v_id[k] * 2);
				GG->coeffRef(m[j] * 2, m[k] * 2 + 1) = BigG.coeff(v_id[j] * 2, v_id[k] * 2 + 1);
				GG->coeffRef(m[j] * 2 + 1, m[k] * 2 + 1) = BigG.coeff(v_id[j] * 2 + 1, v_id[k] * 2 + 1);
			}
		}
	}

	Eigen::SparseMatrix<double> G00_T = G00.transpose();
	Eigen::SparseMatrix<double> G10_T = G10.transpose();

	Gprime = G00 + G00_T;
	B = G01 + G10_T;
}

void ARAPTool::step1()
{
	//compute q

	std::vector<double> qVector;
	Eigen::VectorXd q, nBq;

	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();
		if (!flags[id])
		{
			continue;
		}

		OMT::Point p = mesh->point(*v_it);
		qVector.push_back(-p[0]);
		qVector.push_back(-p[1]);
	}

	q = Eigen::VectorXd::Map(&qVector[0], qVector.size());
	nBq = B * q;

	Eigen::SparseMatrix<double> Gprime_T = Gprime.transpose();
	Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > ldlt(Gprime_T * Gprime);
	Eigen::VectorXd u = ldlt.solve(Gprime_T * nBq);

	int ui = 0;
	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();
		if (flags[id])
		{
			continue;
		}

		OMT::Point p = mesh->point(*v_it);
		p[0] = u[ui++];
		p[1] = u[ui++];

		mesh->set_point(*v_it, p);
	}
}

void ARAPTool::getFeachTri(int* v_id, Eigen::MatrixXd& K, Eigen::Matrix4d& F)
{
	K.setZero();
	K.diagonal().setOnes();

	OMT::Point v0 = mesh->point(mesh->vertex_handle(v_id[0]));
	OMT::Point v1 = mesh->point(mesh->vertex_handle(v_id[1]));
	OMT::Point v2 = mesh->point(mesh->vertex_handle(v_id[2]));

	OMT::Vec2d xdir(v1[0] - v0[0], v1[1] - v0[1]);
	OMT::Vec2d ydir(-xdir[1], xdir[0]);
	OMT::Vec2d v2dir(v2[0] - v0[0], v2[1] - v0[1]);
	double det = xdir[0] * ydir[1] - xdir[1] * ydir[0];
	double x01 = (v2dir[0] * ydir[1] - v2dir[1] * ydir[0]) / det;
	double y01 = (xdir[0] * v2dir[1] - xdir[1] * v2dir[0]) / det;

	K.coeffRef(4, 0) = 1 - x01;
	K.coeffRef(5, 0) = -y01;
	K.coeffRef(4, 1) = y01;
	K.coeffRef(5, 1) = 1 - x01;

	K.coeffRef(4, 2) = x01;
	K.coeffRef(5, 2) = y01;
	K.coeffRef(4, 3) = -y01;
	K.coeffRef(5, 3) = x01;

	F = K.transpose() * K;
}

void ARAPTool::preCompF()
{
	int nt = mesh->n_faces();

	F.reserve(nt);
	invF.reserve(nt);
	K.reserve(nt);

	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		Eigen::Matrix4d tmpF, tmpInvF;
		Eigen::MatrixXd tmpK(6, 4);
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		getFeachTri(v_id, tmpK, tmpF);

		tmpInvF = tmpF.inverse();

		F.push_back(new Eigen::MatrixXd(tmpF));
		invF.push_back(new Eigen::MatrixXd(tmpInvF));
		K.push_back(new Eigen::MatrixXd(tmpK));
	}
}

void ARAPTool::getHeachTri(int* v_id, Eigen::MatrixXd& H)
{
	//reset H
	H.setZero();

	Eigen::MatrixXd tI(2, 6);
	tI.setZero();

	for (int i = 0; i < 3; ++i)
	{
		//reset tI
		tI.setZero();
		int i0 = i, i1 = (i + 1) % 3;

		//for v0'
		tI.coeffRef(0, i0 * 2) = -1;
		tI.coeffRef(1, i0 * 2 + 1) = -1;

		//for v1'
		tI.coeffRef(0, i1 * 2) = 1;
		tI.coeffRef(1, i1 * 2 + 1) = 1;

		H += tI.transpose() * tI;
	}
}

void ARAPTool::preCompH()
{
	int nv = flags.size();
	BigH = Eigen::SparseMatrix<double>(nv * 2, nv * 2);
	BigH.setZero();

	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		Eigen::MatrixXd H(6, 6);
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		getHeachTri(v_id, H);

		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (H.coeff(j * 2, k * 2) != 0)
				{
					BigH.coeffRef(v_id[j] * 2, v_id[k] * 2) += H.coeff(j * 2, k * 2);
				}
				if (H.coeff(j * 2 + 1, k * 2) != 0)
				{
					BigH.coeffRef(v_id[j] * 2 + 1, v_id[k] * 2) += H.coeff(j * 2 + 1, k * 2);
				}
				if (H.coeff(j * 2, k * 2 + 1) != 0)
				{
					BigH.coeffRef(v_id[j] * 2, v_id[k] * 2 + 1) += H.coeff(j * 2, k * 2 + 1);
				}
				if (H.coeff(j * 2 + 1, k * 2 + 1) != 0)
				{
					BigH.coeffRef(v_id[j] * 2 + 1, v_id[k] * 2 + 1) += H.coeff(j * 2 + 1, k * 2 + 1);
				}
			}
		}
	}
}

void ARAPTool::preStep2()
{
	int nt = mesh->n_faces();
	fittedVertices = std::vector< std::vector<OMT::Vec2d> >(nt, std::vector<OMT::Vec2d>(3, OMT::Vec2d(0, 0)));
	// Compute Hprime and D

	// The map from vertex index to index in the matrix (then multiply by 2)
	int nv = flags.size(), cur_free = 0, cur_ctrl = 0;
	std::vector<int> vert_map(nv, 0);

	for (int i = 0; i < nv; ++i)
	{
		if (flags[i] == 0)
		{
			vert_map[i] = cur_free++;
		}
		else
		{
			vert_map[i] = cur_ctrl++;
		}
	}

	if (cur_free == 0 || cur_ctrl == 0)
	{
		return;
	}

	Eigen::SparseMatrix<double> H00(cur_free * 2, cur_free * 2),
		H01(cur_free * 2, cur_ctrl * 2),
		H10(cur_ctrl * 2, cur_free * 2),
		H11(cur_ctrl * 2, cur_ctrl * 2);

	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		int m[3] = { vert_map[v_id[0]], vert_map[v_id[1]], vert_map[v_id[2]] };

		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				Eigen::SparseMatrix<double>* HH = NULL;

				if (!flags[v_id[j]] && !flags[v_id[k]])
				{
					HH = &H00;
				}
				else if (!flags[v_id[j]] && flags[v_id[k]])
				{
					HH = &H01;
				}
				else if (flags[v_id[j]] && !flags[v_id[k]])
				{
					HH = &H10;
				}
				else
				{
					HH = &H11;
				}

				HH->coeffRef(m[j] * 2, m[k] * 2) = BigH.coeff(v_id[j] * 2, v_id[k] * 2);
				HH->coeffRef(m[j] * 2 + 1, m[k] * 2) = BigH.coeff(v_id[j] * 2 + 1, v_id[k] * 2);
				HH->coeffRef(m[j] * 2, m[k] * 2 + 1) = BigH.coeff(v_id[j] * 2, v_id[k] * 2 + 1);
				HH->coeffRef(m[j] * 2 + 1, m[k] * 2 + 1) = BigH.coeff(v_id[j] * 2 + 1, v_id[k] * 2 + 1);
			}
		}
	}

	Eigen::SparseMatrix<double> H00_T = H00.transpose();
	Eigen::SparseMatrix<double> H10_T = H10.transpose();

	Hprime = H00 + H00_T;
	D = H01 + H10_T;
}

void ARAPTool::step2()
{
	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		int f_id = f_it->idx();
		Eigen::MatrixXd _K(*K[f_id]);
		Eigen::MatrixXd _Kt = _K.transpose();

		OMT::Point v0 = mesh->point(mesh->vertex_handle(v_id[0]));
		OMT::Point v1 = mesh->point(mesh->vertex_handle(v_id[1]));
		OMT::Point v2 = mesh->point(mesh->vertex_handle(v_id[2]));

		Eigen::VectorXd vprime(6);
		vprime << v0[0], v0[1], v1[0], v1[1], v2[0], v2[1];

		Eigen::VectorXd _C_tmp = _Kt * vprime;
		Eigen::MatrixXd _invF(*invF[f_id]);

		vprime = _invF * _C_tmp;

		Eigen::VectorXd vfit = _K * vprime;
		fittedVertices[f_id][0][0] = vfit[0];
		fittedVertices[f_id][0][1] = vfit[1];
		fittedVertices[f_id][1][0] = vfit[2];
		fittedVertices[f_id][1][1] = vfit[3];
		fittedVertices[f_id][2][0] = vfit[4];
		fittedVertices[f_id][2][1] = vfit[5];

		OMT::Vec2d center = (fittedVertices[f_id][0] + fittedVertices[f_id][1] + fittedVertices[f_id][2]) / 3.0;

		double scale = (fittedVertices[f_id][0] - fittedVertices[f_id][1]).norm() +
			(fittedVertices[f_id][1] - fittedVertices[f_id][2]).norm() +
			(fittedVertices[f_id][2] - fittedVertices[f_id][0]).norm();

		scale /= (baseVertices[v_id[0]] - baseVertices[v_id[1]]).norm() +
			(baseVertices[v_id[1]] - baseVertices[v_id[2]]).norm() +
			(baseVertices[v_id[2]] - baseVertices[v_id[0]]).norm();

		for (int j = 0; j < 3; ++j)
		{
			OMT::Vec2d v = fittedVertices[f_id][j] - center;
			v /= scale;
			fittedVertices[f_id][j] = center + v;

		}

	}

	//return;

	// The second sub-step of step2

	// Compute q
	Eigen::VectorXd q, Dq_plus_f0;

	int nv = mesh->n_vertices(), cur_free = 0, cur_ctrl = 0;
	std::vector<int> vert_map(nv, 0);
	std::vector<double> qVector;

	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();
		if (flags[id] == 0)
		{
			vert_map[id] = cur_free++;
		}
		else
		{
			vert_map[id] = cur_ctrl++;

			OMT::Point p = mesh->point(*v_it);
			qVector.push_back(p[0]);
			qVector.push_back(p[1]);

		}
	}

	q = Eigen::VectorXd::Map(&qVector[0], qVector.size());

	if (cur_free == 0 || cur_ctrl == 0)
	{
		return;
	}

	Dq_plus_f0 = D * q;

	// Compute f0 and f1
	Eigen::VectorXd f(nv * 2), f0(cur_free * 2), f1(cur_ctrl * 2);
	f.setZero();

	for (OMT::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		int v_id[3];
		int index = 0;
		for (OMT::FVIter fv_it = mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
		{
			v_id[index++] = fv_it->idx();
		}

		int f_id = f_it->idx();
		std::vector<OMT::Vec2d>& fitted = fittedVertices[f_id];

		for (int i = 0; i < 3; ++i)
		{
			int j = (i + 1) % 3;
			OMT::Vec2d vij_f = fitted[j] - fitted[i];
			f[2 * v_id[i]] += -2 * vij_f[0];
			f[2 * v_id[i] + 1] += -2 * vij_f[1];
			f[2 * v_id[j]] += 2 * vij_f[0];
			f[2 * v_id[j] + 1] += 2 * vij_f[1];
		}
	}

	// Map them into f0, f1
	for (int i = 0; i < nv; ++i)
	{
		if (flags[i] == 0)
		{
			f0[vert_map[i] * 2] = f[i * 2];
			f0[vert_map[i] * 2 + 1] = f[i * 2 + 1];
		}
		else
		{
			f1[vert_map[i] * 2] = f[i * 2];
			f1[vert_map[i] * 2 + 1] = f[i * 2 + 1];
		}
	}

	// Compute Dq_plus_f0, and negate it
	for (int i = 0; i < Dq_plus_f0.innerSize(); ++i)
	{
		Dq_plus_f0[i] -= f0[i];
		Dq_plus_f0[i] = -Dq_plus_f0[i];
	}

	Eigen::SparseMatrix<double> Hprime_T = Hprime.transpose();
	Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > ldlt(Hprime_T * Hprime);
	Eigen::VectorXd u = ldlt.solve(Hprime_T * Dq_plus_f0);

	int ui = 0;
	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();
		if (flags[id])
		{
			continue;
		}

		OMT::Point p = mesh->point(*v_it);
		p[0] = u[ui++];
		p[1] = u[ui++];

		mesh->set_point(*v_it, p);
	}
}

void ARAPTool::deform()
{
	int nv = flags.size();
	std::vector<int> ctrl_verts;
	for (int i = 0; i < nv; ++i)
	{
		if (flags[i])
		{
			ctrl_verts.push_back(i);
		}
	}

	if (ctrl_verts.size() >= 2 && ctrl_verts.size() < nv)
	{
		step1();
		step2();

		return;
	}

	if (ctrl_verts.size() == nv)
	{

	}
	else if (ctrl_verts.size() == 1)
	{
		OMT::Point p = mesh->point(mesh->vertex_handle(ctrl_verts[0]));
		OMT::Vec2d v(p[0], p[1]);
		OMT::Vec2d diff = v - baseVertices[ctrl_verts[0]];

		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			int id = v_it->idx();
			OMT::Point _p = mesh->point(*v_it);

			_p[0] = baseVertices[id][0] + diff[0];
			_p[1] = baseVertices[id][1] + diff[1];

			mesh->set_point(*v_it, _p);
		}
	}
	else
	{
		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			int id = v_it->idx();
			OMT::Point _p = mesh->point(*v_it);

			_p[0] = baseVertices[id][0];
			_p[1] = baseVertices[id][1];

			mesh->set_point(*v_it, _p);
		}
	}

	step2();
}

void ARAPTool::OnMotion(int x, int y, int ctrl_index)
{
	OMT::Point point = mesh->point(mesh->vertex_handle(ctrl_index));

	point[0] = x;
	point[1] = y;
	mesh->set_point(mesh->vertex_handle(ctrl_index), point);
	deform();

	if (true/*detect normal*/)
	{
		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			int id = v_it->idx();
			saveLastFlagPosition[id] = mesh->point(*v_it);
		}
	}
	else
	{
		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			int id = v_it->idx();

			mesh->set_point(*v_it, saveLastFlagPosition[id]);
		}
	}
}

void ARAPTool::OnMouse(int x, int y, CtrlOP op)
{
	double minDist = 9999;
	int minIdx = -1;

	if (op == CtrlOP::Add)
	{
		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			OMT::Point p = mesh->point(*v_it);
			double dist = std::sqrt((p[0] - x) * (p[0] - x) + (p[1] - y) * (p[1] - y));

			if (dist > minDist)
			{
				continue;
			}

			int id = v_it->idx();
			if (flags[id] == 0)
			{
				minDist = dist;
				minIdx = id;
			}
		}

		if (minIdx == -1)
		{
			return;
		}

		flags[minIdx] = 1;
		preStep1();
		preStep2();

		ctrlPoints.clear();

		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			int id = v_it->idx();

			if (!flags[id])
			{
				continue;
			}

			ctrlPoints.push_back(id);
		}


	}
	else if (op == CtrlOP::Remove)
	{
		for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			OMT::Point p = mesh->point(*v_it);
			double dist = std::sqrt((p[0] - x) * (p[0] - x) + (p[1] - y) * (p[1] - y));

			if (dist > minDist)
			{
				continue;
			}

			int id = v_it->idx();
			if (flags[id] == 1)
			{
				minDist = dist;
				minIdx = id;
			}
		}

		if (minIdx == -1)
		{
			return;
		}

		flags[minIdx] = 0;
		preStep1();
		preStep2();

		//ctrlPoints.clear();

	}
	else
	{

	}
}

void ARAPTool::ReBind() {
	totalCtrlPoint = 0;

	std::vector<glm::vec3> vertices;
	vertices.reserve(mesh->n_vertices());
	for (Tri_Mesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();

		if (flags[id] > 0)
		{
			Tri_Mesh::Point p = mesh->point(*v_it);
			glm::vec3 v1 = glm::vec3(p[0], p[1], 0);
			vertices.push_back(v1);

			totalCtrlPoint++;
		}
	}

	GLuint vboVertices;
	glGenVertexArrays(1, &ctrl_point_vao);
	glGenBuffers(1, &vboVertices);

	// bind the vao
	glBindVertexArray(ctrl_point_vao);

	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ARAPTool::Render(Shader shader)
{
	if (mesh != NULL)
	{
		ReBind();

		xScale = 482;
		yScale = 604;

		glm::mat4 modelMat = glm::mat4(1.0);
		// normalize the flip y axis
		modelMat = glm::scale(modelMat, glm::vec3(1 / xScale, -1 / yScale, 1));
		// set the model matrix value
		shader.setUniformMatrix4fv("model", modelMat);

		// draw mesh with line and triangle
		mesh->Render(shader);
		
		// draw control point
		if (totalCtrlPoint > 0) {
			glPointSize(8.0);
			shader.setUniform3fv("color", glm::vec3(0, 0, 1));
			glBindVertexArray(ctrl_point_vao);
			glDrawArrays(GL_POINTS, 0, totalCtrlPoint);
			glBindVertexArray(0);
		}
	}
}

Tri_Mesh* ARAPTool::GetMesh() {
	return mesh;
}

int ARAPTool::GetVertex(int x, int y)
{
	double minDist = 9999;
	int minIdx = -1;
	for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		OMT::Point p = mesh->point(*v_it);
		double dist = std::sqrt((p[0] - x) * (p[0] - x) + (p[1] - y) * (p[1] - y));

		if (dist > minDist)
		{
			continue;
		}

		int id = v_it->idx();

		if (flags[id] == 1)
		{
			minDist = dist;
			minIdx = id;
		}
	}

	return minIdx;
}

void ARAPTool::SetCtrlPoints(std::vector<CtrlPoint> pointsToSet)
{
	int changed = false;
	for (int i = 0; i < pointsToSet.size(); ++i)
	{
		int idx = pointsToSet[i].idx;
		if (flags[idx] == 1)
		{
			changed = true;
			mesh->set_point(mesh->vertex_handle(idx), pointsToSet[i].p);
		}
	}

	if (changed)
	{
		deform();
	}

	/*for (OMT::VIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		int id = v_it->idx();
		saveLastFlagPosition[id] = mesh->point(v_it.handle());
	}*/
}