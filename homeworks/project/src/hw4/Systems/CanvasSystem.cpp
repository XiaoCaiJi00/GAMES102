#include "CanvasSystem.h"
#include <Eigen/Dense>


using namespace Ubpa;

static const double INIT_TANGENT_VEC_LENGTH = 30.0;

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
				for (int i = 0; i < data->points.size(); i++)
				{
					if ((mouse_pos_in_canvas - data->points[i]).norm() < data->m_pointRadius)
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
						if ((mouse_pos_in_canvas - data->m_leftTangentEndPoint).norm() < data->m_pointRadius)
						{
							data->m_bIsDragLeftTangentPoint = true;
						}
						else if (data->m_bIsRightTangentDisplayed && (mouse_pos_in_canvas - data->m_rightTangentEndPoint).norm() < data->m_pointRadius)
						{
							data->m_bIsDragRightTangentPoint = true;
						}
						else
						{
							data->m_bIsDragLeftTangentPoint = false;
							data->m_bIsDragRightTangentPoint = false;
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
				else
				{
					data->points.push_back(mouse_pos_in_canvas);
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
					data->points[data->m_dragPointIdx] = mouse_pos_in_canvas;
					data->m_bReCalculate = true;
				}
				else if (data->m_bIsDragLeftTangentPoint)
				{
					double t0, t1;
					int idx = 2 * data->m_tangentLineSelectedPointIdx - 1;
					calDerivativeByHandle(data->points[data->m_tangentLineSelectedPointIdx], data->m_leftTangentEndPoint, t1, data->m_cubicSplineTangentVecLength[idx]);
				}
				else if (data->m_bIsDragRightTangentPoint)
				{

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
				if (ImGui::MenuItem("hide tangentLine", NULL, false, data->points.size() > 0)) { data->m_bTangentDisplayed = false; }
				ImGui::EndPopup();
			}

			if (data->m_bReCalculate)
			{
				data->m_bTangentDisplayed = false;
				calCubicSplineCofficient(data->points, data->m_cubicSplineCofficients);
				calCubicSplineInterpolationPoints(data->points, data->m_cubicSplineCofficients, data->m_cubicSplineInterpolationPoints);
				if (data->points.size() > 1)
				{
					data->m_cubicSplineTangentVecLength.assign(2 * data->points.size() - 2, INIT_TANGENT_VEC_LENGTH);
				}
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
      for (int n = 0; n < data->points.size(); ++n)
      {
        draw_list->AddCircleFilled(ImVec2(origin.x + data->points[n][0], origin.y + data->points[n][1]), data->m_pointRadius, IM_COL32(255, 0, 0, 255));
      }

			drawCubicSpline(data->m_cubicSplineInterpolationPoints, origin, draw_list);
			
			// Draw tangent line if needed
			if (data->m_bTangentDisplayed &&  data->m_tangentLineSelectedPointIdx < data->points.size())
			{
				calAllPointTangentInfo(data->points, data->m_cubicSplineTangentVec, data->m_cubicSplineCofficients);
				if (data->m_tangentLineSelectedPointIdx > 0)
				{
					data->m_bIsLeftTangentDisplayed = true;
					calTangentEndPoint(data->m_cubicSplineTangentVec, data->points[data->m_tangentLineSelectedPointIdx], data->m_tangentLineSelectedPointIdx, data->m_cubicSplineTangentVecLength, data->m_leftTangentEndPoint, true);
					drawSingleTangentLine(data->points[data->m_tangentLineSelectedPointIdx], data->m_leftTangentEndPoint, origin, draw_list, data->m_tangentLinePointWidth, data->m_tangentLinePointHeight);
				}
				if (data->m_tangentLineSelectedPointIdx < data->points.size() - 1)
				{
					data->m_bIsRightTangentDisplayed = true;
					calTangentEndPoint(data->m_cubicSplineTangentVec, data->points[data->m_tangentLineSelectedPointIdx], data->m_tangentLineSelectedPointIdx, data->m_cubicSplineTangentVecLength, data->m_rightTangentEndPoint, false);
					drawSingleTangentLine(data->points[data->m_tangentLineSelectedPointIdx], data->m_rightTangentEndPoint, origin, draw_list, data->m_tangentLinePointWidth, data->m_tangentLinePointHeight);
				}
			}
			data->m_bReCalculate = false;
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}

void CanvasSystem::calCubicSplineCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& coefficient)
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

////获取切线信息，pointIdx表示当前点在points中的索引，vecLength表示切线的长度，origin表示画布原点坐标，draw_list表示ImDrawList对象，rectWidth和rectHeight表示切线端点矩形的宽和高
//void CanvasSystem::drawBothTangentLine(const std::vector<Ubpa::vecf2>& tangentVec, const std::vector<Ubpa::pointf2>& points, int pointIdx, const std::vector<double>& vecLength, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight)
//{
//	if (pointIdx < 0 || pointIdx >= points.size())
//	{
//		return;
//	}
//	if (pointIdx > 0 && 2 * pointIdx - 1 < tangentVec.size())
//	{
//		drawSingleTangentLine(tangentVec[2 * pointIdx - 1], points[pointIdx], vecLength[2 * pointIdx - 1], origin, true, draw_list, rectWidth, rectHeight);
//	}
//	if (pointIdx < points.size() - 1 && 2 * pointIdx < tangentVec.size())
//	{
//		drawSingleTangentLine(tangentVec[2 * pointIdx], points[pointIdx], vecLength[2 * pointIdx], origin, false, draw_list, rectWidth, rectHeight);
//	}
//}

void CanvasSystem::drawSingleTangentLine(const Ubpa::pointf2& point, const Ubpa::pointf2& endPoint, const ImVec2& origin, ImDrawList* draw_list, double rectWidth, double rectHeight)
{
  draw_list->AddLine(ImVec2(origin.x + point[0], origin.y + point[1]), ImVec2(origin.x + endPoint[0], origin.y + endPoint[1]), IM_COL32(255, 255, 0, 255));
  ImVec2 p_min = endPoint + ImVec2(-rectWidth / 2.0, -rectHeight / 2.0);
  ImVec2 p_max = endPoint + ImVec2(rectWidth / 2.0, rectHeight / 2.0);
  draw_list->AddRectFilled(ImVec2(origin.x + p_min.x, origin.y + p_min.y), ImVec2(origin.x + p_max.x, origin.y + p_max.y), IM_COL32(255, 255, 0, 255));
}

void CanvasSystem::calCubicSplineInterpolationPoints(const std::vector<Ubpa::pointf2>& points, const std::vector<CubicPolynomialCofficient>& coefficient, std::vector<std::vector<Ubpa::pointf2>>& interpolationPoints)
{
  interpolationPoints.clear();
  int pointCount = points.size();
	if (pointCount < 2)
	{
		return;
	}
	for (int i = 0; i < pointCount - 1; i++)
	{
		std::vector<Ubpa::pointf2> segmentInterpolationPoints;
		calCubicSplineInterpolationPointsForSegment(points[i], points[i + 1], coefficient[i], segmentInterpolationPoints);
		interpolationPoints.push_back(segmentInterpolationPoints);
  }
}

void CanvasSystem::calCubicSplineInterpolationPointsForSegment(const Ubpa::pointf2& p0, const Ubpa::pointf2& p1, const CubicPolynomialCofficient& coefficient, std::vector<Ubpa::pointf2>& interpolationPoints)
{
	interpolationPoints.clear();
	double step = 1.0;

	for (double x = p0[0]; x <= p1[0]; x += step)
	{
		double y = coefficient.a * x * x * x + coefficient.b * x * x + coefficient.c * x + coefficient.d;
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

void CanvasSystem::calDerivativeByHandle(const Ubpa::pointf2& p0, const Ubpa::pointf2& handlePoint, double& t, double length)
{
	Ubpa::vecf2 vec = handlePoint - p0;
	if (vec[0] < 0)
	{
		vec[0] = -vec[0];
		vec[1] = -vec[1];
	}
	length = vec.norm();
	t = vec[1] / vec[0];
}

void CanvasSystem::calTangentEndPoint(const std::vector<Ubpa::vecf2>& tangentVec, const Ubpa::pointf2& points, int pointIdx, const std::vector<double>& vecLength, Ubpa::pointf2& tangentEndPoint, bool isLeftTangent)
{

	int tangentIdx = isLeftTangent ? 2 * pointIdx - 1 : 2 * pointIdx;
	if (tangentIdx < 0 || tangentIdx >= tangentVec.size())
	{
		return;
	}
	Ubpa::vecf2 vec = tangentVec[tangentIdx] / tangentVec[tangentIdx].norm() * vecLength[tangentIdx] * sqrt(2);
	if (isLeftTangent)
	{
		vec[0] = -vec[0];
		vec[1] = -vec[1];
	}
	tangentEndPoint = points + vec;
}

void CanvasSystem::calDerivativeByCoefficient(const CubicPolynomialCofficient& coefficient, double& t, bool isLeft)
{
	t = 3 * coefficient.a * t * t + 2 * coefficient.b * t + coefficient.c;
}
