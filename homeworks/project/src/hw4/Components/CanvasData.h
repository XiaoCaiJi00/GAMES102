#pragma once

#include <UGM/UGM.h>

struct CubicPolynomialCofficient {
	double a;
  double b;
  double c;
  double d;
};

struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };

  std::vector<CubicPolynomialCofficient> m_cubicSplineCofficients;
  std::vector<Ubpa::pointf2> m_cubicSplineInterpolationPoints;
  std::vector<Ubpa::vecf2> m_cubicSplineTangentVec;
  std::vector<double> m_cubicSplineTangentVecLength;

  int m_dragPointIdx = -1;
  bool m_bIsDragging = false;

  int m_tangentLineSelectedPointIdx = -1;
  bool m_bTangentDisplayed = false;

  //绘制数据属性
  double m_pointRadius = 5.0;
  double m_tangentLinePointWidth = 3.0;
  double m_tangentLinePointHeight = 3.0;
};

#include "details/CanvasData_AutoRefl.inl"
