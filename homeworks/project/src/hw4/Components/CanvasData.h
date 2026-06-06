#pragma once

#include <UGM/UGM.h>

struct CubicPolynomialCofficient {
	double a = 0;
  double b = 0;
  double c = b;
  double d = 0;
};

struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };

  std::vector<CubicPolynomialCofficient> m_cubicSplineCofficients;
  std::vector<std::vector<Ubpa::pointf2>> m_cubicSplineInterpolationPoints;
  std::vector<Ubpa::vecf2> m_cubicSplineTangentVec;
  std::vector<double> m_cubicSplineTangentVecLength;
  Ubpa::pointf2 m_leftTangentEndPoint;
  Ubpa::pointf2 m_rightTangentEndPoint;

  bool m_bReCalculate = false;

  int m_dragPointIdx = -1;
  bool m_bIsDragCurvePoint = false;
  bool m_bIsDragTangentPoint = false;

  int m_tangentLineSelectedPointIdx = -1;
  bool m_bTangentDisplayed = false;

  //绘制数据属性
  double m_pointRadius = 5.0;
  double m_tangentLinePointWidth = 6.0;
  double m_tangentLinePointHeight = 6.0;
};

#include "details/CanvasData_AutoRefl.inl"
