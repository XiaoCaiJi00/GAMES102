#include "CanvasSystem.h"


#include <Eigen/Dense>
#include <spdlog/spdlog.h>
#include <algorithm>

using namespace Ubpa;

void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {
	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;

		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

			// 左侧分组
			ImGui::BeginChild("LeftGroup", ImVec2(800, 150), true);  // 宽度200px，自动高度，带边框
			ImGui::Checkbox("polynomial interpolation", &data->m_bCurPolynomialInterpolation);
			ImGui::Checkbox("gauss basic function", &data->m_bCurGuassBasicInterpolation);
			ImGui::SameLine();
			ImGui::Text("sigma");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputDouble("##sigma", &data->m_gaussSigma, 0.1, 1.0, "%.3f"))
			{
				data->m_gaussSigma = std::max(0.001, data->m_gaussSigma);
			}
			ImGui::SameLine();
			if (ImGui::Button("Apply##gaussInterpolation"))
			{
				data->m_bReCalculate = true;
			}
			ImGui::Checkbox("polynomial fitting", &data->m_bCurPolynomialFitting);
			ImGui::SameLine();
			ImGui::Text("degree");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputInt("##degree", &data->m_polynomialFittingDegree, 1, 5))
			{
				data->m_polynomialFittingDegree = std::max(3, data->m_polynomialFittingDegree);
			}
			ImGui::SameLine();
			if (ImGui::Button("Apply##polyfit"))
			{
				data->m_bReCalculate = true;
			}

			ImGui::Checkbox("ridgeRegression fitting", &data->m_bCurRidgeRegressionFitting);
			ImGui::SameLine();
			ImGui::Text("ridge degree");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputInt("##ridge degree", &data->m_RidgeRegressionFittingDegree, 1, 5))
			{
				data->m_RidgeRegressionFittingDegree = std::max(1, data->m_RidgeRegressionFittingDegree);
			}
			ImGui::SameLine();
			ImGui::Text("ridge lambda");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::InputDouble("##ridge lambda", &data->m_ridgeRegressionlambda, 0.1, 1, "%.3f"))
			{
				data->m_ridgeRegressionlambda = std::max(0.1, data->m_ridgeRegressionlambda);
			}
			ImGui::SameLine();
			if (ImGui::Button("Apply##ridgefit"))
			{
				data->m_bReCalculate = true;
			}
			ImGui::EndChild();

			// 右侧分组（使用 SameLine() 实现并排）
			ImGui::SameLine();
			ImGui::BeginChild("RightGroup", ImVec2(200, 150), true);  // 自动填充剩余宽度
			ImGui::Text("parameterization type：");
			ImGui::RadioButton("uniform parameterize", &data->m_curParamterType, ParameterType::kUniformParameterization);
			ImGui::RadioButton("chord length parameterize", &data->m_curParamterType, ParameterType::kChordParameterization);
			ImGui::RadioButton("centripetal parameterize", &data->m_curParamterType, ParameterType::kCentripetalParameterization);
			ImGui::EndChild();

			if (data->m_preParamterType != data->m_curParamterType)
			{
				data->m_bReCalculate = true;
			}
			if (!data->m_bPreGuassBasicInterpolation && data->m_bCurGuassBasicInterpolation)
			{
				data->m_bReCalculate = true;
			}
			if (!data->m_bPrePolynomialInterpolation && data->m_bCurPolynomialInterpolation)
			{
        data->m_bReCalculate = true;
			}
			if (!data->m_bPrePolynomialFitting && data->m_bCurPolynomialFitting)
			{
				data->m_bReCalculate = true;
      }
			if (!data->m_bPreRidgeRegressionFitting && data->m_bCurRidgeRegressionFitting)
			{
				data->m_bReCalculate = true;
      }
			// Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
			// Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
			// To use a child window instead we could use, e.g:
			//      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
			//      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
			//      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
			//      ImGui::PopStyleColor();
			//      ImGui::PopStyleVar();
			//      [...]
			//      ImGui::EndChild();

			// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
			if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
			if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
			ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

			// Draw border and background color
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
			draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

			// This will catch our interactions
			ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			const bool is_hovered = ImGui::IsItemHovered(); // Hovered
			const bool is_active = ImGui::IsItemActive();   // Held
			const ImVec2 origin(canvas_p0.x + data->scrolling[0], canvas_p0.y + data->scrolling[1]); // Lock scrolled origin
			const pointf2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

			// Add first and second point
			if (is_hovered  && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				data->points.push_back(mouse_pos_in_canvas);
				data->m_bReCalculate = true;
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
			const float mouse_threshold_for_pan = data->opt_enable_context_menu ? -1.0f : 0.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
			{
				data->scrolling[0] += io.MouseDelta.x;
				data->scrolling[1] += io.MouseDelta.y;
			}

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (ImGui::MenuItem("Remove one", NULL, false, data->points.size() > 0)) 
				{ 
					data->points.resize(data->points.size() - 1); 
					data->m_bReCalculate = true;
				}
				if (ImGui::MenuItem("Remove all", NULL, false, data->points.size() > 0))
				{ 
					data->points.clear(); 
					data->m_bReCalculate = true; 
				}
				ImGui::EndPopup();
			}
			if (data->m_bReCalculate)
			{
				std::vector<pointf2> xPoints(data->points.size());
				std::vector<pointf2> yPoints(data->points.size());
				std::vector<double> tParam;
				parameterization(data->points, tParam, data->m_curParamterType);
				for (int i = 0; i < tParam.size(); i++)
				{
					xPoints[i][0] = tParam[i];
          xPoints[i][1] = data->points[i][0];
					yPoints[i][0] = tParam[i];
					yPoints[i][1] = data->points[i][1];
				}
				if (data->m_bCurPolynomialInterpolation)
				{
          data->m_polynomialInterpolationPoints.clear();
					std::vector<pointf2> polynomialInterpolationXPoints;
					std::vector<pointf2> polynomialInterpolationYPoints;
					polynomialInterpolation(xPoints, polynomialInterpolationXPoints);
					polynomialInterpolation(yPoints, polynomialInterpolationYPoints);
					for (int i = 0; i < polynomialInterpolationXPoints.size(); i++)
					{
						double x = polynomialInterpolationXPoints[i][1];
						double y = polynomialInterpolationYPoints[i][1];
						data->m_polynomialInterpolationPoints.push_back(pointf2(x, y));
          }
				}
				if (data->m_bCurGuassBasicInterpolation)
				{
					data->m_guassBasicInterpolationPoints.clear();
          std::vector<pointf2> guassBasicInterpolationXPoints;
					std::vector<pointf2> guassBasicInterpolationYPoints;
          gaussBasicFuncInterpolation(xPoints, data->m_gaussSigma, guassBasicInterpolationXPoints);
					gaussBasicFuncInterpolation(yPoints, data->m_gaussSigma, guassBasicInterpolationYPoints);
					for (int i = 0; i < guassBasicInterpolationXPoints.size(); i++)
					{
						double x = guassBasicInterpolationXPoints[i][1];
						double y = guassBasicInterpolationYPoints[i][1];
						data->m_guassBasicInterpolationPoints.push_back(pointf2(x, y));
					}
				}
				if (data->m_bCurPolynomialFitting)
				{

					data->m_polynomialFittingPoints.clear();
					std::vector<pointf2> polynomialFittingXPoints;
					std::vector<pointf2> polynomialFittingYPoints;
					polynomialFitting(xPoints, data->m_polynomialFittingDegree, polynomialFittingXPoints);
					polynomialFitting(yPoints, data->m_polynomialFittingDegree, polynomialFittingYPoints);
					for (int i = 0; i < polynomialFittingXPoints.size(); i++)
					{
						double x = polynomialFittingXPoints[i][1];
						double y = polynomialFittingYPoints[i][1];
						data->m_polynomialFittingPoints.push_back(pointf2(x, y));
					}
				}
				if (data->m_bCurRidgeRegressionFitting)
				{
					data->m_ridgeRegressionFittingPoints.clear();
					std::vector<pointf2> ridgeRegressionFittingXPoints;
					std::vector<pointf2> ridgeRegressionFittingYPoints;
					ridgeRegressionFitting(xPoints, data->m_ridgeRegressionlambda, data->m_RidgeRegressionFittingDegree, ridgeRegressionFittingXPoints);
					ridgeRegressionFitting(yPoints, data->m_ridgeRegressionlambda, data->m_RidgeRegressionFittingDegree, ridgeRegressionFittingYPoints);
					for (int i = 0; i < ridgeRegressionFittingXPoints.size(); i++)
					{
						double x = ridgeRegressionFittingXPoints[i][1];
						double y = ridgeRegressionFittingYPoints[i][1];
						data->m_ridgeRegressionFittingPoints.push_back(pointf2(x, y));
					}
				}
				data->m_bReCalculate = false;
			}
			

			sysStatus(data);

			// Draw grid + all lines in the canvas
			draw_list->PushClipRect(canvas_p0, canvas_p1, true);
			if (data->opt_enable_grid)
			{
				const float GRID_STEP = 64.0f;
				for (float x = fmodf(data->scrolling[0], GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
				for (float y = fmodf(data->scrolling[1], GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
			}
			for (int n = 0; n < data->points.size(); ++n)
			{
				draw_list->AddCircleFilled(ImVec2(origin.x + data->points[n][0], origin.y + data->points[n][1]), 5.0, IM_COL32(255, 0, 0, 255));
			}
			if (data->m_bCurPolynomialInterpolation && !data->m_polynomialInterpolationPoints.empty())
			{
				drawCurve(data->m_polynomialInterpolationPoints, draw_list, origin, IM_COL32(255, 0, 0, 255));
			}
			if (data->m_bCurGuassBasicInterpolation && !data->m_guassBasicInterpolationPoints.empty())
			{
				drawCurve(data->m_guassBasicInterpolationPoints, draw_list, origin, IM_COL32(0, 255, 0, 255));
			}
			if (data->m_bCurPolynomialFitting && !data->m_polynomialFittingPoints.empty())
			{
				drawCurve(data->m_polynomialFittingPoints, draw_list, origin, IM_COL32(0, 0, 255, 255));
			}
			if (data->m_bCurRidgeRegressionFitting && !data->m_ridgeRegressionFittingPoints.empty())
			{
				drawCurve(data->m_ridgeRegressionFittingPoints, draw_list, origin, IM_COL32(255, 255, 0, 255));
			}
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}

//多项式幂级函数插值
void CanvasSystem::polynomialInterpolation(const std::vector<Ubpa::pointf2>& input, std::vector<Ubpa::pointf2>& output)
{
	output.clear();
  int pointNum = input.size();
	if (pointNum < 2)
	{
		return;
	}

  Eigen::MatrixXd A(pointNum, pointNum);
	Eigen::VectorXd b(pointNum);
	for (int i = 0; i < pointNum; i++)
	{
    double x = 1;
		for (int j = 0; j < pointNum; j++)
		{
			A(i, j) = x;
			x *= input[i][0];
		}
    b(i) = input[i][1];
	}
	/*auto singularValues = A.singularValues();
	double sigmaMax = singularValues(0);
	double sigmaMin = singularValues(singularValues.size() - 1);
	double cond = sigmaMax / sigmaMin;
	spdlog::info("condition number: {:d}", cond);*/

	auto dec = A.colPivHouseholderQr();
	spdlog::info("info: {:d}, rand: {:d}", dec.info(), dec.rank());
	if (dec.info() != Eigen::Success)
	{
		return;
	}
	//Eigen::JacobiSVD<Eigen::MatrixXd> dec(A, Eigen::ComputeThinU | Eigen::ComputeThinV);

  Eigen::VectorXd coefficient = dec.solve(b);
	double step = 0.0001;
  double xPos = input[0][0];
	int idx = 1;
	while(xPos < 1 + DOUBLE_TOLERANCE)
	{
		double y = 0;
		for (int j = 0; j < pointNum; j++)
		{
			y += coefficient(j) * pow(xPos, j);
		}
    output.emplace_back(xPos, y);
		xPos += step;
	}
}

//高斯基函数插值
void CanvasSystem::gaussBasicFuncInterpolation(const std::vector<Ubpa::pointf2>& input, double sigma, std::vector<Ubpa::pointf2>& output)
{
	output.clear();
	if (input.size() < 2)
	{
		return;
	}
	
	int pointNum = input.size();
	Eigen::MatrixXd A(pointNum + 1, pointNum + 1);
	Eigen::VectorXd b(pointNum + 1);
	for (int i = 0; i < pointNum; ++i)
	{
		A(i, 0) = 1;
    double xi = input[i][0] ;
		for (int j = 1; j < pointNum + 1; ++j)
		{
      double xj = input[j - 1][0];
			A(i, j) = exp(-pow(xi - xj, 2) / 2.0 / pow(sigma, 2));
		}
    b(i) = input[i][1];
	}

	//增加约束  初始点input[0]的导数为0
	A(pointNum, 0) = 0;
  double x0 = input[0][0];
	for (int j = 1; j < pointNum + 1; ++j)
	{
    double xj = input[j - 1][0];
		A(pointNum, j) = -(x0 - xj) / pow(sigma, 2) * exp(-pow(x0 - xj, 2) / 2.0 / pow(sigma, 2));
	}
	b(pointNum) = 0;

	auto dec = A.colPivHouseholderQr();
	if (dec.info() != Eigen::Success)
	{
		spdlog::info("info: {:d}, rand: {:d}", dec.info(), dec.rank());
		return;
	}

	Eigen::VectorXd coefficient = dec.solve(b);

  double step = 0.0001;

  double xPos = input[0][0];
  int idx = 1;
	while (xPos < 1 + DOUBLE_TOLERANCE)
	{
		double y = coefficient(0);
		for (int j = 1; j < pointNum + 1; ++j)
		{
      double xj = input[j - 1][0];
			y += coefficient(j) * exp(-pow(xPos - xj, 2) / 2.0 / pow(sigma, 2));
		}
		output.emplace_back(xPos, y);
		xPos += step;
	}
}

void CanvasSystem::polynomialFitting(const std::vector<Ubpa::pointf2>& input, int degree, std::vector<Ubpa::pointf2>& output)
{
	output.clear();
	if (input.size() < 2)
	{
		return;
	}
  int pointNum = input.size();
  Eigen::MatrixXd A(pointNum, degree + 1);
  Eigen::VectorXd b(pointNum);

  for (int i = 0; i < pointNum; i++)
	{
		double x = 1;
		for (int j = 0; j < degree + 1; j++)
		{
			A(i, j) = x;
			x *= input[i][0];
		}
		b(i) = input[i][1];
	}
  Eigen::VectorXd coefficient = A.colPivHouseholderQr().solve(b);
  double step = 0.0001;
  double xPos = input[0][0];
  int idx = 1;
	while (xPos < 1 + DOUBLE_TOLERANCE)
	{
		double y = 0;
		for (int j = 0; j <= degree; j++)
		{
			y += coefficient(j) * pow(xPos, j);
		}
		output.emplace_back(xPos, y);
		xPos += step;
	}
}

void CanvasSystem::ridgeRegressionFitting(const std::vector<Ubpa::pointf2>& input, double lambda, int degree, std::vector<Ubpa::pointf2>& output)
{
	if (degree < 1 || lambda < 0)
	{
		return;
	}
	output.clear();
	int pointCount = input.size();
	if (pointCount < 2)
	{
		return;
	}
  Eigen::MatrixXd A(pointCount + degree + 1, degree + 1);
  Eigen::VectorXd b(pointCount + degree + 1);

	for (int i = 0; i < pointCount; i++)
	{
		A(i, 0) = 1;
		b(i) = input[i][1];
		for (int j = 1; j < degree + 1; j++)
		{
			A(i, j) = pow(input[i][0], j);
		}
	}
	for (int i = 0; i < degree + 1; i++)
	{
		for (int j = 0; j < degree + 1; j++)
		{
			A(pointCount + i, j) = (i == j ? sqrt(lambda) : 0);
		}
		b(pointCount + i) = 0;
	}
  Eigen::VectorXd coefficient = A.colPivHouseholderQr().solve(b);
  double step = 0.00011;
  double xPos = input[0][0];
  int idx = 1;
	while (xPos < 1 + DOUBLE_TOLERANCE)
	{
		double y = 0;
		for (int j = 0; j < degree + 1; j++)
		{
			y += coefficient(j) * pow(xPos, j);
		}
		output.emplace_back(xPos, y);
		xPos += step;
	}
}

void CanvasSystem::parameterization(const std::vector<Ubpa::pointf2>& input, std::vector<double>& tParam, int type)
{
	switch (type)
	{
		case ParameterType::kUniformParameterization:
			uniformParameterization(input, tParam);
			break;
		case ParameterType::kChordParameterization:
			chordLengthParameterization(input, tParam);
			break;
		case ParameterType::kCentripetalParameterization:
			centripetalParameterization(input, tParam);
			break;
		default:
			break;
	}
}

void CanvasSystem::uniformParameterization(const std::vector<Ubpa::pointf2>& input, std::vector<double>& tParam)
{
	tParam.clear();
	int n = input.size();
	if (n < 2)
	{
		return;
	}
	tParam.resize(n);
	tParam[0] = 0;
	tParam[n - 1] = 1;
	double step = 1.0 / (n - 1);
	for (int i = 1; i < n - 1; ++i)
	{
		tParam[i] = tParam[i - 1] + step;
	}
}

void CanvasSystem::chordLengthParameterization(const std::vector<Ubpa::pointf2>& input, std::vector<double>& tParam)
{
  tParam.clear();
	int n = input.size();
	if (n < 2)
	{
		return;
	}
	tParam.resize(n);
  tParam[0] = 0;
	for (int i = 1; i < n; ++i)
	{
		double length = sqrt(pow(input[i][0] - input[i - 1][0], 2) + pow(input[i][1] - input[i -1][1], 2));
		tParam[i] = tParam[i - 1] + length;
	}
	for (int i = 1; i < n; ++i)
	{
		tParam[i] /= tParam[n - 1];
	}
}

void CanvasSystem::centripetalParameterization(const std::vector<Ubpa::pointf2>& input, std::vector<double>& tParam)
{
	tParam.clear();
	int n = input.size();
	if (n < 2)
	{
		return;
	}
	tParam.resize(n);
	tParam[0] = 0;
	for (int i = 1; i < n; ++i)
	{
		double length = sqrt(pow(input[i][0] - input[i - 1][0], 2) + pow(input[i][1] - input[i - 1][1], 2));
		tParam[i] = tParam[i - 1] + sqrt(length);
	}
	for (int i = 1; i < n; ++i)
	{
		tParam[i] /= tParam[n - 1];
	}
}

void CanvasSystem::sysStatus(CanvasData* data)
{
	if (data == nullptr)
	{
		return;
	}
	data->m_bPrePolynomialInterpolation = data->m_bCurPolynomialInterpolation;
	data->m_bPreGuassBasicInterpolation = data->m_bCurGuassBasicInterpolation;
  data->m_bPrePolynomialFitting = data->m_bCurPolynomialFitting;
  data->m_bPreRidgeRegressionFitting = data->m_bCurRidgeRegressionFitting;
	data->m_preParamterType = data->m_curParamterType;
}

void CanvasSystem::drawCurve(const std::vector<Ubpa::pointf2>& points, ImDrawList* draw_list, const ImVec2& origin, ImU32 col)
{
	for (int n = 1; n < points.size(); ++n)
	{
		draw_list->AddLine(ImVec2(origin.x + points[n - 1][0], origin.y + points[n - 1][1]),
			ImVec2(origin.x + points[n][0], origin.y + points[n][1]),
			col, 2.0f);
	}
}

