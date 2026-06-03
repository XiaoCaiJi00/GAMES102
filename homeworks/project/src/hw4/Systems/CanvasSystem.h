#pragma once

#include <UECS/World.h>
#include <UGM/vec.h>
#include "../Components/CanvasData.h"
#include <_deps/imgui/imgui.h>

struct CanvasSystem {
	static void OnUpdate(Ubpa::UECS::Schedule& schedule);
  static void calCubicSplineCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& cofficient);
  static void getPointTangentInfo(const Ubpa::pointf2& points, Ubpa::vecf2& tangentVec, const CubicPolynomialCofficient& cofficient);
  static void calAllPointTangentInfo(const std::vector<Ubpa::pointf2>& points, std::vector<Ubpa::vecf2>& tangentVec, const std::vector<CubicPolynomialCofficient>& cofficient);
  static void drawTangentLine(const Ubpa::vecf2 tangentVec, const Ubpa::pointf2& point, double length, const ImVec2& origin, bool isLeftPoint, ImDrawList* draw_list, double rectWidth, double rectHeight);
};