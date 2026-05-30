#pragma once

#include <UECS/World.h>
#include "UGM/point.h"
#include "../Components/CanvasData.h"

static const double DOUBLE_TOLERANCE = 1e-8;

struct CanvasSystem {
	static void OnUpdate(Ubpa::UECS::Schedule& schedule);
  static void polynomialInterpolation(const std::vector<Ubpa::pointf2>& input, std::vector<Ubpa::pointf2>& output);
  static void gaussBasicFuncInterpolation(const std::vector<Ubpa::pointf2>& input, double sigma, std::vector<Ubpa::pointf2>& output);
  static void polynomialFitting(const std::vector<Ubpa::pointf2>& input, int degree, std::vector<Ubpa::pointf2>& output);
  static void ridgeRegressionFitting(const std::vector<Ubpa::pointf2>& input, double lambda, int degree, std::vector<Ubpa::pointf2>& output);
  static void sysStatus(CanvasData* data);
};