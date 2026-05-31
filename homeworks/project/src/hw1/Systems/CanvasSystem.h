#pragma once

#include <UECS/World.h>
#include "UGM/point.h"
#include "../Components/CanvasData.h"
#include <_deps/imgui/imgui.h>

static const double DOUBLE_TOLERANCE = 1e-8;
static const double MIN_X = 0.0;
static const double MAX_X = 1.0;

struct CanvasSystem {
	static void OnUpdate(Ubpa::UECS::Schedule& schedule);
  static void polynomialInterpolation(const std::vector<Ubpa::pointf2>& input, std::vector<Ubpa::pointf2>& output);
  static void gaussBasicFuncInterpolation(const std::vector<Ubpa::pointf2>& input, double sigma, std::vector<Ubpa::pointf2>& output);
  static void polynomialFitting(const std::vector<Ubpa::pointf2>& input, int degree, std::vector<Ubpa::pointf2>& output);
  static void ridgeRegressionFitting(const std::vector<Ubpa::pointf2>& input, double lambda, int degree, std::vector<Ubpa::pointf2>& output);
  static void sysStatus(CanvasData* data);

  static void drawCurve(const std::vector<Ubpa::pointf2>& points, ImDrawList* draw_list, const ImVec2& origin, ImU32 col);
  static void getNormalizedParameter(const std::vector<Ubpa::pointf2>& points, double& minVal, double& maxVal);
};