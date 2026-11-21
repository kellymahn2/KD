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

	static double PlotMin = -0.1 / 60.0;

	double LimitMinX = PlotMin, LimitMaxX = 15.0 / 60.0;
	uint32_t Count = 1;

	static Ref<AnimationTree> Context;

	static bool Recording = false;

	AnimationTree::AnimationTreeNode* SelectedNode;

	static bool IsParent = false;

	static std::vector<Entity> SelectedPath;
	static uint32_t PathIndex = 0;

	static double InteractionTime = 0.0f;
	static uint32_t CurrentFrame = 0;

	static void ShowDragLine()
	{
		bool held;
		ImPlot::DragLineX(
			ImGui::GetID("SharedTimeLine"),
			&InteractionTime,
			ImVec4(1.0f, 1.0f, 1.0f, 0.0f),
			1.0f, ImPlotDragToolFlags_Delayed | ImPlotDragToolFlags_NoEdge, nullptr, nullptr, &held
		);

		double currentFrameTime = CurrentFrame / 60.0;

		if (!held)
		{
			InteractionTime = currentFrameTime;
		}
		else
		{
			InteractionTime = glm::clamp(InteractionTime, LimitMinX, LimitMaxX);
		}

		if (InteractionTime - currentFrameTime >= 1.0 / 60.0)
		{
			CurrentFrame++;
			InteractionTime = CurrentFrame / 60.0;
			currentFrameTime = CurrentFrame / 60.0;
		}
		else if (InteractionTime - currentFrameTime <= -1.0 / 60.0)
		{
			CurrentFrame = glm::max(CurrentFrame - 1, 0U);

			InteractionTime = CurrentFrame / 60.0;
			currentFrameTime = CurrentFrame / 60.0;
		}

		ImPlot::DragLineX(
			ImGui::GetID("SharedTimeLine2"),
			&currentFrameTime,
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			1.0f, ImPlotDragToolFlags_Delayed | ImPlotDragToolFlags_NoEdge | ImPlotDragToolFlags_NoInputs
		);

	}

	static int FormatFrameNumber(double value, char* buffer, int size, void*)
	{
		return snprintf(buffer, size, "%.0f", value * 60.0);
	}
	
	std::vector<double> ticks;
	std::vector<std::string> labels;
	std::vector<const char*> cLabels(ticks.size(), nullptr);

	static double SetupDynamicFrameTicks(double minTime, double maxTime)
	{
		const double minFrame = minTime * 60.0;
		const double maxFrame = maxTime * 60.0;
		const double frameRange = maxFrame - minFrame;

		const double targetSpacing = frameRange / 10.0;

		const double magnitude = std::pow(10.0, std::floor(std::log10(
			std::max(targetSpacing, 1e-9))));
		const double normalized = targetSpacing / magnitude;

		double spacing;
		if (normalized <= 1.0)
			spacing = magnitude;
		else if (normalized <= 2.0)
			spacing = 2.0 * magnitude;
		else if (normalized <= 5.0)
			spacing = 5.0 * magnitude;
		else
			spacing = 10.0 * magnitude;

		spacing = glm::max(spacing, 1.0);

		ticks.clear();
		labels.clear();

		const double firstFrame = std::ceil(minFrame / spacing) * spacing;
		for (double frame = firstFrame;
			frame <= maxFrame;
			frame += spacing)
		{
			ticks.push_back(frame / 60.0);
			labels.push_back(fmt::format("{}", (uint32_t)frame));
		}

		cLabels.resize(ticks.size());

		for (uint32_t i = 0; i < ticks.size(); ++i)
		{
			cLabels[i] = labels[i].c_str();
		}

		ImPlot::SetupAxisTicks(ImAxis_X1, ticks.data(), ticks.size(), cLabels.data());

		return spacing;
	}

	void AnimationPanel::SetupPlot(bool header)
	{
		static const constexpr uint32_t s_XAxisFlags =
			ImPlotAxisFlags_NoHighlight |
			ImPlotAxisFlags_NoLabel |
			ImPlotAxisFlags_PanStretch |
			ImPlotAxisFlags_Opposite |
			ImPlotAxisFlags_NoSideSwitch;

		static const constexpr uint32_t s_XAxisNonHeaderFlags = 
			ImPlotAxisFlags_NoHighlight |
			ImPlotAxisFlags_NoTickLabels |
			ImPlotAxisFlags_NoTickMarks |
			ImPlotAxisFlags_NoLabel |
			ImPlotAxisFlags_PanStretch;

		ImPlot::SetupAxis(ImAxis_X1, nullptr, header ? s_XAxisFlags : s_XAxisNonHeaderFlags);

		ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, PlotMin, DBL_MAX);

		ImPlot::SetupAxis(ImAxis_Y1, nullptr, s_YAxisFlags);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, 0.5);

		ImPlot::SetupAxisLinks(ImAxis_X1, &LimitMinX, &LimitMaxX);

		if (header)
		{
			SetupDynamicFrameTicks(LimitMinX, LimitMaxX);
		}
		else
		{
			ImPlot::SetupAxisTicks(ImAxis_X1, ticks.data(), ticks.size(), cLabels.data());
		}
	}

	void AnimationPanel::OnImGuiRender()
	{
		Count = 1;

		ImGui::Begin("Animation");

		Asset asset = EditorContext::SelectedAsset();

		if (asset)
		{
			Ref<AnimationTree> context = asset->GetAssetType() == AssetType::Animation ? asset.As<AnimationTree>() : nullptr;
			
			if (context)
			{
				Context = context;
				SelectedNode = nullptr;
			}
		}

		Entity selected = EditorContext::SelectedEntity();

		ImGui::BeginDisabled(!selected);

		if (Context)
		{
			ImGui::Checkbox("Recording", &Recording);
			ImGui::SameLine();
			ImGui::Text("%f, %u", InteractionTime, CurrentFrame);

			Styler styler;
			styler.PushStyle(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
			
			PlotStyler plotStyler;
			plotStyler.PushStyle(ImPlotStyleVar_PlotPadding, ImVec2(5.0f, 0.0f));
			plotStyler.PushStyle(ImPlotStyleVar_PlotBorderSize, 0.0f);

			//float height = ImGui::GetTextLineHeightWithSpacing() * 20;

			if (ImGui::BeginTable("##PlotTable", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0.0f, -1.0f)))
			{
				ImGui::TableSetupScrollFreeze(0, 2);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Plot");
				ImGui::TableHeadersRow();

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();

				ImVec2 headerSize = ImVec2(-1.0f, ImGui::GetTextLineHeightWithSpacing());

				if (ImPlot::BeginPlot("##HeaderPlot", s_PlotSize, s_PlotFlags))
				{
					SetupPlot(true);

					ShowDragLine();

					ImPlot::EndPlot();
				}

				AnimationTree::AnimationTreeNode& root = Context->RootNode;
				RenderNode(root, "Root");

				ImGui::EndTable();
			}
		}

		ImGui::EndDisabled();

		ImGui::End();

		ImGui::Begin("Plot styler");
		ImPlot::ShowStyleEditor();
		ImGui::End();

		ImPlot::ShowDemoWindow();
	}

	static AnimationTree::AnimationTreeNode* GetCurrentNode()
	{
		if (IsParent)
		{
			while (PathIndex >= 0)
			{
				SelectedNode = &SelectedNode->Children[SelectedPath[PathIndex].GetName()];

				if (PathIndex == 0)
				{
					break;
				}

				--PathIndex;
			}
		}

		IsParent = false;
	
		return SelectedNode;
	}

	static AnimationFrame* FindFrameInTrack(Scope<AnimationTrack>& track)
	{
		if (track->Frames.empty())
		{
			return nullptr;
		}

		size_t start = 0;
		size_t end = static_cast<size_t>(track->Frames.size()) - 1;

		while (start <= end)
		{
			size_t mid = start + (end - start) / 2;
			double frameTime = track->Frames[mid].Time;

			if (frameTime == CurrentFrame)
			{
				return &track->Frames[mid];
			}

			if (frameTime > CurrentFrame)
			{
				end = mid - 1;
			}
			else
			{
				start = mid + 1;
			}
		}

		return nullptr;
	}

	void AnimationPanel::OnPositionChanged()
	{
		if (!Recording || !SelectedNode)
		{
			return;
		}

		auto& track = GetCurrentNode()->Tracks[(uint32_t)AnimationValueType::Position];

		if (!track)
		{
			track = CreateScope<AnimationTrack>();

			track->ValueType = AnimationValueType::Position;
		}

		AnimationFrame* frame = FindFrameInTrack(track);

		auto& newTarget = EditorContext::SelectedEntity().GetComponent<TransformComponent>().Translation;

		if (frame)
		{
			frame->Position.Target = newTarget;
		}
		else
		{
			AnimationFrame f(AnimationPositionData{ newTarget }, CurrentFrame);
			
			track->Frames.push_back(f);

			//Not first frame.
			if (track->Frames.size() > 1)
			{
				size_t i = track->Frames.size() - 1;

				while (track->Frames[i].Time < track->Frames[i - 1].Time)
				{
					std::swap(track->Frames[i], track->Frames[i - 1]);

					i--;
				}
			}
		}

		AssetManager::SetDirty(Context->GetHandle(), true);
	}

	void AnimationPanel::OnRotationChanged()
	{
		if (!Recording || !SelectedNode)
		{
			return;
		}
	}

	void AnimationPanel::OnScaleChanged()
	{
		if (!Recording || !SelectedNode)
		{
			return;
		}
	}

	static bool FindPathToAnimRoot(Entity current, std::vector<Entity>& path)
	{
		auto apc = current.TryGetComponent<AnimationPlayerComponent>();

		if (apc && apc->Animation == Context) 
		{
			//path.push_back(current);
			return true;
		}
		
		{
			auto pc = current.TryGetComponent<ChildComponent>();

			if (pc)
			{
				path.push_back(current);
				return FindPathToAnimRoot(Entity(pc->Parent, current.GetScene()), path);
			}

			return false;
		}
	}

	void AnimationPanel::OnSelectionChanged()
	{
		Entity selected = EditorContext::SelectedEntity();

		if (selected && Context)
		{
			SelectedPath.clear();

			if(FindPathToAnimRoot(selected, SelectedPath))
			{
				SelectedNode = &Context->RootNode;
				IsParent = false;

				if(!SelectedPath.empty())
				{
					PathIndex = SelectedPath.size() - 1;

					do 
					{
						const auto& name = SelectedPath[PathIndex].GetComponent<TagComponent>().Tag;

						auto it = SelectedNode->Children.find(name);
						if (it != SelectedNode->Children.find(name))
						{
							IsParent = false;
							SelectedNode = &it->second;
						}
						else
						{
							//Not yet added as an animation node.
							IsParent = true;
							return;
						}

						--PathIndex;
					} while (SelectedPath[PathIndex] != selected);

					KD_CORE_ASSERT(PathIndex == 0);
				}

				return;
			}
		}
	
		SelectedNode = nullptr;
	}
	

	void AnimationPanel::RenderTrack(AnimationTrack& track)
	{
		ImGui::PushID(&track);
		if (ImPlot::BeginPlot("##Plot", s_PlotSize, s_PlotFlags))
		{
			SetupPlot(false);


			for (uint32_t i = 0; i < track.Frames.size(); ++i)
			{
				auto& frame = track.Frames[i];

				double x, y;
				x = frame.Time * 1.0 / 60.0;
				y = 0.0f;

				bool clicked = false;
				bool held = false;

				static int LastHeld = -1;

				if (ImPlot::DragPoint(i + 1, &x, &y, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 4.0f, ImPlotDragToolFlags_Delayed, &clicked, nullptr, &held))
				{
					double f = x * 60.0;

					frame.Time = glm::round(f);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
				{
					EditorContext::SelectedAnimationFrame(&frame, track.ValueType);
				}

				if (held && LastHeld == -1)
				{
					LastHeld = i;
				}

				if (LastHeld == i && !held)
				{
					LastHeld = -1;
				}
			}

			ShowDragLine();

			ImPlot::EndPlot();
		}
		ImGui::PopID();
	}

	void AnimationPanel::RenderNode(AnimationTree::AnimationTreeNode& currNode, const std::string& name)
	{
		
		const char* animationValueTypeStrings[] = {
			"p",
			"r",
			"s"
		};

		for (uint32_t i = 0; i < AnimationValueTypeCount; ++i)
		{
			if (currNode.Tracks[i])
			{
				
				ImGui::TableNextColumn();

				ImGui::Text("%s %s", animationValueTypeStrings[i], name.c_str());

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
