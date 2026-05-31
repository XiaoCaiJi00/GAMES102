#pragma once

#include <UGM/UGM.h>

struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	bool m_bCurPolynomialInterpolation = false;
	bool m_bPrePolynomialInterpolation = false;
	bool m_bCurGuassBasicInterpolation = false;
	bool m_bPreGuassBasicInterpolation = false;
	bool m_bCurPolynomialFitting = false;
	bool m_bPrePolynomialFitting = false;
	bool m_bCurRidgeRegressionFitting = false;
	bool m_bPreRidgeRegressionFitting = false;
	bool m_bReCalculate = false;

	std::vector<Ubpa::pointf2> m_polynomialInterpolationPoints;
	std::vector<Ubpa::pointf2> m_guassBasicInterpolationPoints;
	std::vector<Ubpa::pointf2> m_polynomialFittingPoints;
	std::vector<Ubpa::pointf2> m_ridgeRegressionFittingPoints;
	double m_gaussSigma = 0.5;
	double m_ridgeRegressionlambda = 0.1;
  int m_polynomialFittingDegree = 3;
  int m_RidgeRegressionFittingDegree = 3;

};

#include "details/CanvasData_AutoRefl.inl"
