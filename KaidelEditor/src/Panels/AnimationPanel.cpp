#include "AnimationPanel.h"
#include "UI/UIHelper.h"

#include <imgui/imgui.h>
#include <imgui/implot.h>
namespace Kaidel {

	static AnimationFrame* currSelected = nullptr;


	static const constexpr ImVec2 s_PlotSize = ImVec2(-1, 40.0f);
	static const constexpr ImPlotAxisFlags s_YAxisFlags = 
		ImPlotAxisFlags_NoDecorations | 
		ImPlotAxisFlags_NoGridLines | 
		ImPlotAxisFlags_NoHighlight | 
		ImPlotAxisFlags_Lock;

	static const constexpr ImPlotFlags s_PlotFlags = 
		ImPlotFlags_NoMouseText | 
		ImPlotFlags_NoFrame |
		ImPlotFlags_NoBoxSelect |
		ImPlotFlags_NoMenus;

	void AnimationPanel::OnImGuiRender()
	{
		m_Count = 1;

		ImGui::Begin("Animation");

		//auto treePtr = std::get_if<Ref<AnimationTree>>(&m_Context->Variant);


		if (m_Context)
		{
			Styler styler;
			styler.PushStyle(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
			
			PlotStyler plotStyler;
			plotStyler.PushStyle(ImPlotStyleVar_PlotPadding, ImVec2(5.0f, 0.0f));
			plotStyler.PushStyle(ImPlotStyleVar_PlotBorderSize, 0.0f);

			float height = ImGui::GetTextLineHeightWithSpacing() * 20;

			if (ImGui::BeginTable("##PlotTable", 3, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0.0f, height)))
			{
				ImGui::TableSetupScrollFreeze(0, 2);
				ImGui::TableSetupColumn("#");
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Plot");
				ImGui::TableHeadersRow();

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
				ImGui::TableNextColumn();

				ImVec2 headerSize = ImVec2(-1.0f, ImGui::GetTextLineHeightWithSpacing());

				if (ImPlot::BeginPlot("##HeaderPlot", headerSize, s_PlotFlags))
				{
					static const constexpr uint32_t s_XAxisFlags =
						ImPlotAxisFlags_NoHighlight |
						ImPlotAxisFlags_NoLabel |
						ImPlotAxisFlags_PanStretch | 
						ImPlotAxisFlags_Opposite | 
						ImPlotAxisFlags_NoSideSwitch;

					ImPlot::SetupAxis(ImAxis_X1, nullptr, s_XAxisFlags);

					ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, DBL_MAX);

					ImPlot::SetupAxis(ImAxis_Y1, nullptr, s_YAxisFlags);
					ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, 0.5);

					ImPlot::SetupAxisLinks(ImAxis_X1, &m_LimitMinX, &m_LimitMaxX);

					ImPlot::EndPlot();
				}

				AnimationTree::AnimationTreeNode& root = m_Context->RootNode;
				RenderNode(root, "Root");

				ImGui::EndTable();
			}
		}
		

		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::Begin("Plot styler");
		ImPlot::ShowStyleEditor();
		ImGui::End();
	}

	void AnimationPanel::RenderTrack(AnimationTrack& track)
	{
		ImGui::PushID(&track);
		if (ImPlot::BeginPlot("##Plot", s_PlotSize, s_PlotFlags))
		{
			static const constexpr uint32_t s_XAxisFlags =
				ImPlotAxisFlags_NoHighlight |
				ImPlotAxisFlags_NoTickLabels |
				ImPlotAxisFlags_NoTickMarks |
				ImPlotAxisFlags_NoLabel |
				ImPlotAxisFlags_PanStretch;

			ImPlot::SetupAxis(ImAxis_X1, nullptr, s_XAxisFlags);

			ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, DBL_MAX);

			ImPlot::SetupAxis(ImAxis_Y1, nullptr, s_YAxisFlags);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, 0.5);

			ImPlot::SetupAxisLinks(ImAxis_X1, &m_LimitMinX, &m_LimitMaxX);

			for (uint32_t i = 0; i < track.Frames.size(); ++i)
			{
				auto& frame = track.Frames[i];

				double x, y;
				x = frame.Time;
				y = 0.0f;

				bool clicked = false;
				ImPlot::DragPoint(i, &x, &y, ImVec4(1.0f,1.0f,1.0f,1.0f), 4.0f, 0, &clicked);

				if (clicked)
				{
					EditorContext::SelectedAnimationFrame(&frame, track.ValueType);

				}
			}

			ImPlot::EndPlot();
		}
		ImGui::PopID();
	}

	void AnimationPanel::RenderNode(AnimationTree::AnimationTreeNode& currNode, const std::string& name)
	{
		
		const char* animationValueTypeStrings[] = {
			" : Position",
			" : Rotation",
			" : Scale"
		};

		for (uint32_t i = 0; i < AnimationValueTypeCount; ++i)
		{
			if (currNode.Tracks[i])
			{
				
				ImGui::TableNextColumn();
				
				ImGui::Text("%d", m_Count++);

				ImGui::TableNextColumn();

				ImGui::Text("%s%s", name.c_str(), animationValueTypeStrings[i]);

				ImGui::TableNextColumn();

				RenderTrack(*currNode.Tracks[i]);
			}
		}

		for (auto& [name, node] : currNode.Children)
		{
			RenderNode(node, name);
		}

	}

	void AnimationPanel::ShowPanel(Ref<AnimationTree> tree)
	{
		if (!ImGui::BeginChild("##Plot"))
			return;

		{
			//Styler styler;
			//styler.PushStyle(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			//
			//PlotStyler plotStyler;
			//plotStyler.PushStyle(ImPlotStyleVar_PlotPadding, ImVec2(10, 0));
			//plotStyler.PushStyle(ImPlotStyleVar_PlotBorderSize, 0);


			if (ImGui::BeginTable("##Plots", 2))
			{
				ImGui::TableSetupColumn("Node name", ImGuiTableColumnFlags_IndentEnable);
				ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_IndentEnable);
				ImGui::TableHeadersRow();

				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
	}
}
