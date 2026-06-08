#include "CanvasSystem.h"
#include <Eigen/Dense>


using namespace Ubpa;

static const double INIT_TANGENT_VEC_LENGTH = 30.0;
static const double TANGENT_VEC_SCALE = 10.0;

void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {
	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;

		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

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

			if (is_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				for (int i = 0; i < data->m_points.size(); i++)
				{
					if ((mouse_pos_in_canvas - data->m_points[i]).norm() < data->m_pointRadius)
					{
						data->m_dragPointIdx = i;
						data->m_bIsDragCurvePoint = true;
						data->m_tangentLineSelectedPointIdx = i;
						data->m_bTangentDisplayed = true;
						break;
					}
				}
				if (!data->m_bIsDragCurvePoint && data->m_bTangentDisplayed)
				{
					if (data->m_bIsLeftTangentDisplayed)
					{
						if ((mouse_pos_in_canvas - data->m_leftTangentEndPoint).norm() < 40)
						{
							data->m_bIsDragLeftTangentPoint = true;
						}
						else if (data->m_bIsRightTangentDisplayed && (mouse_pos_in_canvas - data->m_rightTangentEndPoint).norm() < data->m_pointRadius)
						{
							data->m_bIsDragRightTangentPoint = true;
						}
						else
						{
							//data->m_bIsDragLeftTangentPoint = false;
							//data->m_bIsDragRightTangentPoint = false;
						}
					}
				}
			}

			// Add first and second point
			if (is_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
        if (data->m_bIsDragCurvePoint)
				{
					data->m_bIsDragCurvePoint = false;
				}
				else if (data->m_bIsDragLeftTangentPoint || data->m_bIsDragRightTangentPoint)
				{
					data->m_bIsDragLeftTangentPoint = data->m_bIsDragRightTangentPoint = false;
				}
				else
				{
					data->m_points.push_back(mouse_pos_in_canvas);
					data->m_bReCalculate = true;
				}
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
			const float mouse_threshold_for_pan = data->opt_enable_context_menu ? -1.0f : 0.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
			{
				data->scrolling[0] += io.MouseDelta.x;
				data->scrolling[1] += io.MouseDelta.y;
			}
      if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, mouse_threshold_for_pan))
      {
				if (data->m_bIsDragCurvePoint)
				{
					data->m_points[data->m_dragPointIdx] = mouse_pos_in_canvas;
					data->m_bReCalculate = true;
				}
				else if (data->m_bIsDragLeftTangentPoint)
				{
					data->m_leftTangentEndPoint = mouse_pos_in_canvas;
					int idx = data->m_tangentLineSelectedPointIdx;
					Ubpa::vecf2 handle = data->m_points[idx] - data->m_leftTangentEndPoint;
					calLeftCoefficientByHandle(data->m_points, idx, data->m_tParams, handle,
																	data->m_cubicSplineXCofficients[idx - 1],
																	data->m_cubicSplineYCofficients[idx - 1]);
					calCubicSplineInterpolationPointsForSegment(data->m_tParams[idx - 1], data->m_tParams[idx],
																											data->m_cubicSplineXCofficients[idx - 1], data->m_cubicSplineYCofficients[idx - 1],
																											data->m_cubicSplineInterpolationPoints[idx - 1]);

					if (idx < data->m_points.size() - 1)
					{
						calRightCoefficientByHandle(data->m_points, idx, data->m_tParams, handle, data->m_cubicSplineXCofficients[idx], data->m_cubicSplineYCofficients[idx]);
						calCubicSplineInterpolationPointsForSegment(data->m_tParams[idx], data->m_tParams[idx + 1],
																												data->m_cubicSplineXCofficients[idx], data->m_cubicSplineYCofficients[idx],
																												data->m_cubicSplineInterpolationPoints[idx]);
					}
 
				}
				else if (data->m_bIsDragRightTangentPoint)
				{
					data->m_rightTangentEndPoint = mouse_pos_in_canvas;
					int idx = data->m_tangentLineSelectedPointIdx;
					Ubpa::vecf2 handle = data->m_rightTangentEndPoint - data->m_points[idx];
					calRightCoefficientByHandle(data->m_points, idx, data->m_tParams, handle, data->m_cubicSplineXCofficients[idx], data->m_cubicSplineYCofficients[idx]);
					calCubicSplineInterpolationPointsForSegment(data->m_tParams[idx], data->m_tParams[idx + 1],
						data->m_cubicSplineXCofficients[idx], data->m_cubicSplineYCofficients[idx],
						data->m_cubicSplineInterpolationPoints[idx]);
					if (idx > 0)
					{
						calLeftCoefficientByHandle(data->m_points, idx, data->m_tParams, handle,
																			 data->m_cubicSplineXCofficients[idx - 1],
																			 data->m_cubicSplineYCofficients[idx - 1]);
					calCubicSplineInterpolationPointsForSegment(data->m_tParams[idx - 1], data->m_tParams[idx],
																											data->m_cubicSplineXCofficients[idx - 1], data->m_cubicSplineYCofficients[idx - 1],
																											data->m_cubicSplineInterpolationPoints[idx - 1]);
					}
				}
				else
				{

				}
      }
      

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (ImGui::MenuItem("Remove one", NULL, false, data->m_points.size() > 0)) 
				{ 
					data->m_points.resize(data->m_points.size() - 1); 
					data->m_bReCalculate = true;
				}
				if (ImGui::MenuItem("Remove all", NULL, false, data->m_points.size() > 0)) 
				{ 
					data->m_points.clear(); 
					data->m_bReCalculate = true;
				}
				if (ImGui::MenuItem("hide tangentLine", NULL, false, data->m_points.size() > 0)) { data->m_bTangentDisplayed = false; }
				ImGui::EndPopup();
			}

			if (data->m_bReCalculate)
			{
				data->m_bTangentDisplayed = false;
				uniformParameterization(data->m_points, data->m_tParams);
				calCubicSplineParamCofficient(data->m_points, data->m_tParams, data->m_cubicSplineXCofficients, data->m_cubicSplineYCofficients);
				calCubicSplineInterpolationPoints(data->m_tParams, data->m_cubicSplineXCofficients, data->m_cubicSplineYCofficients, data->m_cubicSplineInterpolationPoints);
			}


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
      for (int n = 0; n < data->m_points.size(); ++n)
      {
        draw_list->AddCircleFilled(ImVec2(origin.x + data->m_points[n][0], origin.y + data->m_points[n][1]), data->m_pointRadius, IM_COL32(255, 0, 0, 255));
      }

			drawCubicSpline(data->m_cubicSplineInterpolationPoints, origin, draw_list);
			
			// Draw tangent line if needed
			if (data->m_bTangentDisplayed &&  data->m_tangentLineSelectedPointIdx < data->m_points.size())
			{
				int idx = data->m_tangentLineSelectedPointIdx;
				if (data->m_tangentLineSelectedPointIdx > 0)
				{
					data->m_bIsLeftTangentDisplayed = true;
					Ubpa::vecf2 handleVec;
					calTangentHandle(data->m_tParams[idx], data->m_cubicSplineXCofficients[idx - 1], data->m_cubicSplineYCofficients[idx - 1], handleVec, true);
					data->m_leftTangentEndPoint = data->m_points[idx] + handleVec;
					drawSingleTangentLine(data->m_points[idx], data->m_leftTangentEndPoint, origin, draw_list, data->m_tangentLinePointWidth, data->m_tangentLinePointHeight);
				}
				if (data->m_tangentLineSelectedPointIdx < data->m_points.size() - 1)
				{
					data->m_bIsRightTangentDisplayed = true;
					Ubpa::vecf2 handleVec;
					calTangentHandle(data->m_tParams[idx], data->m_cubicSplineXCofficients[idx], data->m_cubicSplineYCofficients[idx], handleVec, false);
					data->m_rightTangentEndPoint = data->m_points[idx] + handleVec;
					drawSingleTangentLine(data->m_points[idx], data->m_rightTangentEndPoint, origin, draw_list, data->m_tangentLinePointWidth, data->m_tangentLinePointHeight);
				}

			}
			data->m_bReCalculate = false;
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}

void CanvasSystem::calCubicSplineParamCofficient(const std::vector<Ubpa::pointf2>& points, const std::vector<double>& tParams, std::vector<CubicPolynomialCofficient>& xCoefficient, std::vector<CubicPolynomialCofficient>& yCoefficient)
{
	int n = points.size();
	if (n < 2)
	{
		return;
	}
	std::vector<Ubpa::pointf2> xParamPoints(n);
	for (int i = 0; i < n; ++i)
	{
		xParamPoints[i][0] = tParams[i];
		xParamPoints[i][1] = points[i][0];
	}
	calCubicSplineSingleCofficient(xParamPoints, xCoefficient);

	std::vector<Ubpa::pointf2> yParamPoints(n);
	for (int i = 0; i < n; ++i)
	{
		yParamPoints[i][0] = tParams[i];
		yParamPoints[i][1] = points[i][1];
	}
	calCubicSplineSingleCofficient(yParamPoints, yCoefficient);
}

void CanvasSystem::calCubicSplineSingleCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& coefficient)
{
	coefficient.clear();
	int pointCount = points.size();
	if (pointCount < 2)
	{
		return;
	}
	Eigen::VectorXd M(pointCount);
	M(0) = 0;
	M(pointCount - 1) = 0;
	if (pointCount > 2)
	{
		Eigen::MatrixXd A(pointCount - 2, pointCount - 2);
		A.setZero();
		Eigen::VectorXd b(pointCount - 2);
		double lastH = points[1][0] - points[0][0];
		double lastB = 6.0 * (points[1][1] - points[0][1]) / lastH;
		double curH = points[2][0] - points[1][0];
		double curB = 6.0 * (points[2][1] - points[1][1]) / curH;
		A(0, 0) = 2 * (curH + lastH);
		if (pointCount > 3)
		{
			A(0, 1) = curH;
		}
		b(0) = curB - lastB;
		for (int i = 1; i < pointCount - 2; i++)
		{
			lastH = curH;
			lastB = curB;
			A(i, i - 1) = lastH;
			curH = points[i + 2][0] - points[i + 1][0];
			A(i, i) = 2 * (curH + lastH);
			curB = 6.0 * (points[i + 2][1] - points[i + 1][1]) / curH;
			if (i + 1 < pointCount - 2)
			{
				A(i, i + 1) = curH;
			}
			b(i) = curB - lastB;
		}
		M.segment(1, pointCount - 2) = A.colPivHouseholderQr().solve(b);
	}


	for (int i = 0; i < pointCount - 1; i++)
	{
		double h = points[i + 1][0] - points[i][0];
		CubicPolynomialCofficient coeff;
		coeff.a = (M(i + 1) - M(i)) / (6.0 * h);
		coeff.b = (M(i) * points[i + 1][0] - M(i + 1) * points[i][0]) / (2.0 * h);
		coeff.c = -M(i) * points[i + 1][0] * points[i + 1][0] / (2.0 * h) +
			M(i + 1) * points[i][0] * points[i][0] / (2.0 * h) +
			(points[i + 1][1] - points[i][1]) / h +
			(M(i) - M(i + 1)) * h / 6.0;
		coeff.d = M(i) * points[i + 1][0] * points[i + 1][0] * points[i + 1][0] / (6.0 * h) -
			M(i + 1) * points[i][0] * points[i][0] * points[i][0] / (6.0 * h) -
			(points[i + 1][1] / h - M(i + 1) * h / 6.0) * points[i][0] +
			(points[i][1] / h - M(i) * h / 6.0) * points[i + 1][0];
		coefficient.push_back(coeff);
	}
}

//y = a * x^3 + b * x^2 + c * x + d
void CanvasSystem::getPointTangentInfo(const Ubpa::pointf2& points, const CubicPolynomialCofficient& coefficient, Ubpa::vecf2& tangentVec)
{
	tangentVec[0] = 1;
	tangentVec[1] = 3 * coefficient.a * points[0] * points[0] + 2 * coefficient.b * points[0] + coefficient.c;
}

//计算每段曲线的端点切线信息，存储在tangentVec中
void CanvasSystem::calAllPointTangentInfo(const std::vector<Ubpa::pointf2>& points, std::vector<Ubpa::vecf2>& tangentVec, const std::vector<CubicPolynomialCofficient>& coefficient)
{
  int pointCount = points.size();
	if (pointCount == 0)
	{
		return;
	}
	tangentVec.clear();
  tangentVec.resize(2 * pointCount - 2);
	for(int i = 0; i < 2 * pointCount - 2; i++)
	{
    int pointIdx = (i + 1) / 2;
		int cofficientIdx = i / 2;
		getPointTangentInfo(points[pointIdx], coefficient[cofficientIdx], tangentVec[i]);
  }
}

void CanvasSystem::drawCubicSpline(const std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints, const ImVec2& origin, ImDrawList* draw_list)
{
	int segmentCount = interpolationPoints.size();
	if (segmentCount == 0)
	{
		return;
	}

	int lastSegIdx = 0;
	int lastPointIdx = 0;
	int currentSegIdx = 0;
	int currentPointIdx = 1;
	while (currentSegIdx < segmentCount)
	{
		if (currentPointIdx >= interpolationPoints[currentSegIdx].size())
		{
			currentSegIdx++;
			currentPointIdx = 1;
			continue;
		}
		draw_list->AddLine(ImVec2(origin.x + interpolationPoints[lastSegIdx][lastPointIdx][0], 
															origin.y + interpolationPoints[lastSegIdx][lastPointIdx][1]), 
											 ImVec2(origin.x + interpolationPoints[currentSegIdx][currentPointIdx][0], 
														 origin.y + interpolationPoints[currentSegIdx][currentPointIdx][1]), IM_COL32(0, 255, 0, 255));
		lastSegIdx = currentSegIdx;
		lastPointIdx = currentPointIdx;
		currentPointIdx++;

	}
}

void CanvasSystem::drawSingleTangentLine(const Ubpa::pointf2& point, const Ubpa::pointf2& endPoint, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight)
{
  draw_list->AddLine(ImVec2(origin.x + point[0], origin.y + point[1]), ImVec2(origin.x + endPoint[0], origin.y + endPoint[1]), IM_COL32(255, 255, 0, 255));
  ImVec2 p_min = endPoint + ImVec2(-rectWidth / 2.0, -rectHeight / 2.0);
  ImVec2 p_max = endPoint + ImVec2(rectWidth / 2.0, rectHeight / 2.0);
  draw_list->AddRectFilled(ImVec2(origin.x + p_min.x, origin.y + p_min.y), ImVec2(origin.x + p_max.x, origin.y + p_max.y), IM_COL32(255, 255, 0, 255));
}

void CanvasSystem::calCubicSplineInterpolationPoints(const std::vector<double>& tParams, const std::vector<CubicPolynomialCofficient>& xCoefficient, const std::vector<CubicPolynomialCofficient>& yCoefficient, std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints)
{
  interpolationPoints.clear();
  int pointCount = tParams.size();
	if (pointCount < 2)
	{
		return;
	}
	for (int i = 0; i < pointCount - 1; i++)
	{
		std::vector<Ubpa::pointf2> segmentInterpolationPoints;
		calCubicSplineInterpolationPointsForSegment(tParams[i], tParams[i + 1], xCoefficient[i], yCoefficient[i], segmentInterpolationPoints);
		interpolationPoints.push_back(segmentInterpolationPoints);
  }
}

void CanvasSystem::calCubicSplineInterpolationPointsForSegment(double param1, double param2, const CubicPolynomialCofficient& xCoefficient, const CubicPolynomialCofficient& yCoefficient, std::vector<Ubpa::pointf2>& interpolationPoints)
{
	interpolationPoints.clear();
	double step = 0.001;

	for (double t = param1; t <= param2; t += step)
	{
		double x = xCoefficient.a * t * t * t + xCoefficient.b * t * t + xCoefficient.c * t + xCoefficient.d;
		double y = yCoefficient.a * t * t * t + yCoefficient.b * t * t + yCoefficient.c * t + yCoefficient.d;
		interpolationPoints.push_back(Ubpa::pointf2(x, y));
	}
}

void CanvasSystem::calCofficientForSegment(const Ubpa::pointf2& p0, const Ubpa::pointf2& p1, double	t0, double t1, CubicPolynomialCofficient& coefficient)
{
	Eigen::Matrix4d A;
  	A << p0[0] * p0[0] * p0[0], p0[0] * p0[0], p0[0], 1,
			   p1[0] * p1[0] * p1[0], p1[0] * p1[0], p1[0], 1,
				 3 * p0[0] * p0[0], 2 * p0[0], 1, 0,
				 3 * p1[0] * p1[0], 2 * p1[0], 1, 0;
		Eigen::Vector4d b;
		b << p0[1], p1[1], t0, t1;
		Eigen::Vector4d x = A.colPivHouseholderQr().solve(b);
		coefficient.a = x[0];
		coefficient.b = x[1];
		coefficient.c = x[2];
		coefficient.d = x[3];
}

void CanvasSystem::calDerivativeByHandle(const Ubpa::pointf2& p0, const Ubpa::pointf2& handlePoint, double& t)
{
	Ubpa::vecf2 vec = handlePoint - p0;
	if (vec[0] < 0)
	{
		vec[0] = -vec[0];
		vec[1] = -vec[1];
	}
	t = vec[1] / vec[0];
}

void CanvasSystem::calTangentEndPoint(const std::vector<Ubpa::vecf2>& tangentVec, const Ubpa::pointf2& points, int pointIdx, const std::vector<double>& vecLength, Ubpa::pointf2& tangentEndPoint, bool isLeftTangent)
{

	int tangentIdx = isLeftTangent ? 2 * pointIdx - 1 : 2 * pointIdx;
	if (tangentIdx < 0 || tangentIdx >= tangentVec.size())
	{
		return;
	}
	Ubpa::vecf2 vec1 = tangentVec[tangentIdx];
	Ubpa::vecf2 vec = tangentVec[tangentIdx] / vec1.norm() * vecLength[tangentIdx] * sqrt(2);
	if (isLeftTangent)
	{
		vec[0] = -vec[0];
		vec[1] = -vec[1];
	}
	tangentEndPoint = points + vec;
}

double CanvasSystem::calDerivativeOnParam(double tParam, const CubicPolynomialCofficient& coefficient)
{
	return 3 * coefficient.a * tParam * tParam + 2 * coefficient.b * tParam + coefficient.c;
}

void CanvasSystem::calTangentHandle(double tParam, const CubicPolynomialCofficient& xCoefficient, const CubicPolynomialCofficient& yCoefficient, Ubpa::vecf2& handleVec, bool isLeft)
{
		handleVec[0] = calDerivativeOnParam(tParam, xCoefficient);
		handleVec[1] = calDerivativeOnParam(tParam, yCoefficient);
		if (isLeft)
		{
			handleVec[0] = -handleVec[0];
			handleVec[1] = -handleVec[1];
		}
		//等比缩小
		handleVec = handleVec / TANGENT_VEC_SCALE;
}
void CanvasSystem::calLeftCoefficientByHandle(const std::vector<Ubpa::pointf2>& points, int idx, const std::vector<double>& params, const Ubpa::vecf2& handle, CubicPolynomialCofficient& xCoefficient, CubicPolynomialCofficient& yCoefficient)
{
	Ubpa::vecf2 realHandle = handle * TANGENT_VEC_SCALE;
	Ubpa::pointf2 currentPoint = points[idx];
	Ubpa::pointf2 prePoint = points[idx - 1];
	Ubpa::pointf2 preParamX(params[idx - 1], prePoint[0]);
	Ubpa::pointf2 curParamX(params[idx], currentPoint[0]);
	double preXDerivate = calDerivativeOnParam(preParamX[0], xCoefficient);
	calCofficientForSegment(preParamX, curParamX, preXDerivate, realHandle[0], xCoefficient);

	Ubpa::pointf2 preParamY(params[idx - 1], prePoint[1]);
	Ubpa::pointf2 curParamY(params[idx], currentPoint[1]);
	double preYDerivate = calDerivativeOnParam(preParamY[0], yCoefficient);
	calCofficientForSegment(preParamY, curParamY, preYDerivate, realHandle[1], yCoefficient);
}
void CanvasSystem::calRightCoefficientByHandle(const std::vector<Ubpa::pointf2>& points, int idx, const std::vector<double>& params, const Ubpa::vecf2& handle, CubicPolynomialCofficient& xCoefficient, CubicPolynomialCofficient& yCoefficient)
{
	Ubpa::vecf2 realHandle = handle * TANGENT_VEC_SCALE;
	Ubpa::pointf2 currentPoint = points[idx];
	Ubpa::pointf2 nextPoint = points[idx + 1];
	Ubpa::pointf2 nextParamX(params[idx + 1], nextPoint[0]);
	Ubpa::pointf2 curParamX(params[idx], currentPoint[0]);
	double nextXDerivate = calDerivativeOnParam(nextParamX[0], xCoefficient);
	calCofficientForSegment(curParamX, nextParamX, realHandle[0], nextXDerivate, xCoefficient);

	Ubpa::pointf2 nextParamY(params[idx + 1], nextPoint[1]);
	Ubpa::pointf2 curParamY(params[idx], currentPoint[1]);
	double nextYDerivate = calDerivativeOnParam(nextParamY[0], yCoefficient);
	calCofficientForSegment(curParamY, nextParamY, realHandle[1], nextYDerivate, yCoefficient);
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
