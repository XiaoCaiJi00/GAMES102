#pragma once

#include <UECS/World.h>
#include <UGM/vec.h>
#include "../Components/CanvasData.h"
#include <_deps/imgui/imgui.h>

struct CanvasSystem {
	static void OnUpdate(Ubpa::UECS::Schedule& schedule);
  static void calCubicSplineParamCofficient(const std::vector<Ubpa::pointf2>& points, const std::vector<double>& tParams, std::vector<CubicPolynomialCofficient>& xCoefficient, std::vector<CubicPolynomialCofficient>& yCoefficient);
  static void calCubicSplineSingleCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& coefficient);
	static void uniformParameterization(const std::vector<Ubpa::pointf2>& input, std::vector<double>& tParam);
	static void getPointTangentInfo(const Ubpa::pointf2& points, const CubicPolynomialCofficient& coefficient, Ubpa::vecf2& tangentVec);
  static void calAllPointTangentInfo(const std::vector<Ubpa::pointf2>& points, std::vector<Ubpa::vecf2>& tangentVec, const std::vector<CubicPolynomialCofficient>& coefficient);
	static void calCubicSplineInterpolationPoints(const std::vector<double>& tParams, const std::vector<CubicPolynomialCofficient>& xCoefficient, const std::vector<CubicPolynomialCofficient>& yCoefficient, std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints);
	static void calCubicSplineInterpolationPointsForSegment(double param1, double param2, const CubicPolynomialCofficient& xCoefficient, const CubicPolynomialCofficient& yCoefficient, std::vector<Ubpa::pointf2>& interpolationPoints);
	static void calCofficientForSegment(const Ubpa::pointf2& p0, const Ubpa::pointf2& p1, double t0, double t1, CubicPolynomialCofficient& coefficient);
	static void calDerivativeByHandle(const Ubpa::pointf2& p0, const Ubpa::pointf2& handlePoint, double& t);
	static void calTangentEndPoint(const std::vector<Ubpa::vecf2>& tangentVec, const Ubpa::pointf2& points, int pointIdx, const std::vector<double>& vecLength, Ubpa::pointf2& tangentEndPoint, bool isLeftTangent);
	static double calDerivativeOnParam(double tParam, const CubicPolynomialCofficient& coefficient);
	static void	calTangentHandle(double tParam, const CubicPolynomialCofficient& coefficient, const CubicPolynomialCofficient& yCoefficient, Ubpa::vecf2& handleVec, bool isLeft);
	static void calLeftCoefficientByHandle(const std::vector<Ubpa::pointf2>& points, int idx, const std::vector<double>& params, const Ubpa::vecf2& handle, CubicPolynomialCofficient& xCoefficient, CubicPolynomialCofficient& yCoefficient);
	static void calRightCoefficientByHandle(const std::vector<Ubpa::pointf2>& points, int idx, const std::vector<double>& params, const Ubpa::vecf2& handle, CubicPolynomialCofficient& xCoefficient, CubicPolynomialCofficient& yCoefficient);

	static void drawCubicSpline(const std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints, const ImVec2& origin, ImDrawList* draw_list);
  //static void drawBothTangentLine(const std::vector<Ubpa::vecf2>& tangentVec, const std::vector<Ubpa::pointf2>& points, int pointIdx, const std::vector<double>& vecLength, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight);
  static void drawSingleTangentLine(const Ubpa::pointf2& point, const Ubpa::pointf2& endPoint, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight);
  
};