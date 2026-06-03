#include "CanvasSystem.h"


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
			if (is_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
        if (data->m_bIsDragging)
				{
					data->m_bIsDragging = false;
					data->m_dragPointIdx = -1;
				}
				else
				{
					data->points.push_back(mouse_pos_in_canvas);
				}
			}
      if (is_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
				for (int i = 0; i < data->points.size(); i++)
				{
          if ((mouse_pos_in_canvas - data->points[i]).norm() < data->m_pointRadius)
					{	
						data->m_dragPointIdx = i;
						data->m_bIsDragging = true;
						break;
					}
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
      if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, mouse_threshold_for_pan) && data->m_bIsDragging)
      {
        data->points[data->m_dragPointIdx] = mouse_pos_in_canvas;
      }
      

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (ImGui::MenuItem("Remove one", NULL, false, data->points.size() > 0)) { data->points.resize(data->points.size() - 1); }
				if (ImGui::MenuItem("Remove all", NULL, false, data->points.size() > 0)) { data->points.clear(); }
				ImGui::EndPopup();
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
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}

void CanvasSystem::calCubicSplineCofficient(const std::vector<Ubpa::pointf2>& points, std::vector<CubicPolynomialCofficient>& cofficient)
{

}

//y = a * x^3 + b * x^2 + c * x + d
void CanvasSystem::getPointTangentInfo(const Ubpa::pointf2& points, Ubpa::vecf2& tangentVec, const CubicPolynomialCofficient& cofficient)
{
	tangentVec[0] = 1;
	tangentVec[1] = 3 * cofficient.a * points[0] * points[0] + 2 * cofficient.b * points[0] + cofficient.c;
}

//计算每段曲线的端点切线信息，存储在tangentVec中
void CanvasSystem::calAllPointTangentInfo(const std::vector<Ubpa::pointf2>& points, std::vector<Ubpa::vecf2>& tangentVec, const std::vector<CubicPolynomialCofficient>& cofficient)
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
		getPointTangentInfo(points[pointIdx], tangentVec[i], cofficient[cofficientIdx]);
  }
}

//根据切线信息绘制切线，isLeftPoint表示当前点是曲线段的左端点还是右端点
void CanvasSystem::drawTangentLine(const Ubpa::vecf2 tangentVec, const Ubpa::pointf2& point, double length, const ImVec2& origin, bool isLeftPoint, ImDrawList* draw_list, double rectWidth, double rectHeight)
{
	Ubpa::vecf2 vec = tangentVec / tangentVec.norm() * length * sqrt(2);
	if (isLeftPoint)
	{
    vec[0] = -vec[0];
    vec[1] = -vec[1];
	}
  Ubpa::pointf2 endPoint = point + vec;
  draw_list->AddLine(ImVec2(origin.x + point[0], origin.y + point[1]), ImVec2(origin.x + endPoint[0], origin.y + endPoint[1]), IM_COL32(255, 255, 0, 255));
  ImVec2 p_min = point + ImVec2(-rectWidth / 2.0, -rectHeight / 2.0);
  ImVec2 p_max = point + ImVec2(rectWidth / 2.0, rectHeight / 2.0);
  draw_list->AddRectFilled(ImVec2(origin.x + p_min.x, origin.y + p_min.y), ImVec2(origin.x + p_max.x, origin.y + p_max.y), IM_COL32(255, 255, 0, 255));
}
