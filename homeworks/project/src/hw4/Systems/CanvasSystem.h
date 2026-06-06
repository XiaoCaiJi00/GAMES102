#pragma once

#include <UECS/World.h>
#include <UGM/vec.h>
#include "../Components/CanvasData.h"
#include <_deps/imgui/imgui.h>

struct CanvasSystem {
	static void OnUpdate(Ubpa::UECS::Schedule& schedule);
  static void calCubicSplineCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& coefficient);
  static void getPointTangentInfo(const Ubpa::pointf2& points, const CubicPolynomialCofficient& coefficient, Ubpa::vecf2& tangentVec);
  static void calAllPointTangentInfo(const std::vector<Ubpa::pointf2>& points, std::vector<Ubpa::vecf2>& tangentVec, const std::vector<CubicPolynomialCofficient>& coefficient);
	static void calCubicSplineInterpolationPoints(const std::vector<Ubpa::pointf2>& points, const std::vector<CubicPolynomialCofficient>& coefficient, std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints);
	static void calCubicSplineInterpolationPointsForSegment(const Ubpa::pointf2& p0, const Ubpa::pointf2& p1, const CubicPolynomialCofficient& coefficient, std::vector<Ubpa::pointf2>& interpolationPoints);
	static void calCofficientForSegment(const Ubpa::pointf2& p0, const Ubpa::pointf2& p1, double t0, double t1, CubicPolynomialCofficient& coefficient);
	static void calDerivativeByHandle(const Ubpa::pointf2& p0, const Ubpa::pointf2& handlePoint, double& t, double length);

	static void drawCubicSpline(const std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints, const ImVec2& origin, ImDrawList* draw_list);
  static void drawBothTangentLine(const std::vector<Ubpa::vecf2>& tangentVec, const std::vector<Ubpa::pointf2>& points, int pointIdx, const std::vector<double>& vecLength, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight);
  static void drawSingleTangentLine(const Ubpa::vecf2 tangentVec, const Ubpa::pointf2& point, double length, const ImVec2& origin, bool isRightPoint, ImDrawList* draw_list, double rectWidth, double rectHeight);
  
};