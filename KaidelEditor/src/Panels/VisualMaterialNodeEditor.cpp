#include "VisualMaterialNodeEditor.h"
#include "Kaidel/Utils/PlatformUtils.h"
#include "Kaidel/Core/Input.h"

#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

#include "UI/UIHelper.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <imgui/imnodes.h>
#include <imgui/imnodes_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <queue>


namespace Kaidel {

	static uint64_t FindN(const std::string& text, const std::string& query)
	{
		int queryLen = query.length();

		if (queryLen == 0 || text.empty())
		{
			return -1;
		}

		if (queryLen > text.length())
			return -1;

		const char* srcData = text.data();

		for (int i = 0; i <= (text.length() - queryLen); i++)
		{
			bool found = true;
			for (int j = 0; j < queryLen; j++)
			{
				int read_pos = i + j;

				char src = std::tolower(srcData[read_pos]);
				char dst = std::tolower(query[j]);

				if (src != dst)
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return i;
			}
		}

		return -1;
	}

	static glm::vec4 s_PortColors[] =
	{
		glm::vec4(0.0f, 0.6f, 0.0f, 1.0f),
		glm::vec4(0.2f, 0.4f, 0.9f, 1.0f),
		glm::vec4(0.0f, 0.6f, 1.0f, 1.0f),
		glm::vec4(1.0f, 0.6f, 0.0f, 1.0f),
		glm::vec4(0.6f, 0.0f, 0.9f, 1.0f),
		glm::vec4(0.0f, 0.75f, 0.75f, 1.0f),
		glm::vec4(0.9f, 0.0f, 0.9f, 1.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
	};

	static glm::vec2 LocalPos;

	static int StartNode = -1, StartLink = -1;

	static int RenamingNode = -1;

	struct NodePicker {

		struct Node 
		{
			std::map<std::string, Node> Children;
			std::function<void(Ref<VisualMaterial> mat)> OnClicked;
			std::string UsageHint;
			std::string Name;
			VisualMaterialNodePortType OutputType = VisualMaterialNodePortType::Bool;
			bool Filtered = false;
		};

		Node Root;
		std::string Search;

		void _Filter(Node& node, bool defaultVal)
		{
			node.Filtered = defaultVal;

			//Leaf
			if (defaultVal && node.Children.empty())
			{
				node.Filtered = FindN(node.Name, Search) == -1;
			}
			else
			{
				for (auto& [name, child] : node.Children)
				{
					_Filter(child, defaultVal);

					if (defaultVal && !child.Filtered)
					{
						node.Filtered = false;
					}
				}
			}
		}

		void _DrawRec(Ref<VisualMaterial> mat, const Node& node)
		{
			if (node.Children.empty() && !node.Filtered)
			{
				const char* outputName = nullptr;

				switch (node.OutputType)
				{
				case Kaidel::VisualMaterialNodePortType::Bool: outputName = "bool"; break;
				case Kaidel::VisualMaterialNodePortType::Int: outputName = "int"; break;
				case Kaidel::VisualMaterialNodePortType::UInt: outputName = "uint"; break;
				case Kaidel::VisualMaterialNodePortType::Float: outputName = "float"; break;
				case Kaidel::VisualMaterialNodePortType::Vec2: outputName = "vec2"; break;
				case Kaidel::VisualMaterialNodePortType::Vec3: outputName = "vec3"; break;
				case Kaidel::VisualMaterialNodePortType::Vec4: outputName = "vec4"; break;
				case Kaidel::VisualMaterialNodePortType::Sampler: outputName = "sampler"; break;
				}

				if (ImGui::TreeNodeEx(node.Name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf, "%s : %s", node.Name.c_str(), outputName))
				{
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						node.OnClicked(mat);
					}

					ImGui::TreePop();
				}

				if (ImGui::BeginItemTooltip())
				{
					ImGui::Text(node.UsageHint.c_str());
					ImGui::EndTooltip();
				}
			}
			else if (ImGui::TreeNodeEx(node.Name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				for (auto& [name, child] : node.Children)
				{
					if(!child.Filtered)
						_DrawRec(mat, child);
				}
				ImGui::TreePop();
			}
		}

		void Draw(Ref<VisualMaterial> mat)
		{
			if (ImGui::InputText("##NodeSearch", &Search) )
			{
				for (auto& [name, child] : Root.Children) {
					_Filter(Root, !Search.empty());
				}
			}

			if (ImGui::BeginChild("##Nodes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders))
			{
				for (auto& [name, node] : Root.Children)
				{
					if (!node.Filtered)
						_DrawRec(mat, node);
				}
				ImGui::EndChild();
			}
		}

		template<typename T>
		void RegisterNode(const std::string& path, const std::string& hint, VisualMaterialNodePortType outputType)
		{
			Node& node = Walk(path, Root);
			node.UsageHint = hint;
			node.OutputType = outputType;
			node.OnClicked = [](Ref<VisualMaterial> material) { 
				auto& node = material->CreateNode<T>();
				node->Position = LocalPos;
				ImGui::CloseCurrentPopup();

				if (StartNode != -1 && StartLink != -1)
				{
					uint32_t startLink = StartLink - material->Nodes[StartNode]->PortOffset;

					bool isInput = material->Nodes[StartNode]->GetInputPortCount() > startLink;

					//if start is input then new node is output(from node)
					if (isInput)
						material->AddConnection(node->NodeID, 0, StartNode, startLink);
					else
						material->AddConnection(StartNode, startLink - material->Nodes[StartNode]->GetInputPortCount(), node->NodeID, 0);
				}
			};
		}

		Node& Walk(std::string_view path, Node& current)
		{
			uint64_t loc = path.find_first_of('/');
			if (loc == std::string::npos)
			{
				std::string_view curr = path;
				
				auto& node = current.Children[(std::string)curr];
				node.Name = curr;
				return node;
			}
			else {
				std::string_view curr = path.substr(0, loc);
				for (auto& [name, node] : current.Children)
				{
					if (name == curr)
					{
						return Walk(path.substr(loc + 1), node);
					}
				}
				auto& node = current.Children[(std::string)curr];
				node.Name = (std::string)curr;

				Walk(path.substr(loc + 1), node);
			}
		}

		NodePicker() = default;

		NodePicker(bool) 
		{
			RegisterNode<VisualMaterialNodeBoolConstant>("Constants/Bool constant", "Boolean constant (Can be converted to parameter)", VisualMaterialNodePortType::Bool);
			RegisterNode<VisualMaterialNodeIntConstant>("Constants/Int constant", "Integer constant (Can be converted to parameter)", VisualMaterialNodePortType::Int);
			RegisterNode<VisualMaterialNodeUIntConstant>("Constants/UInt constant", "Unsigned integer constant (Can be converted to parameter)", VisualMaterialNodePortType::UInt);
			RegisterNode<VisualMaterialNodeFloatConstant>("Constants/Float constant", "Float constant (Can be converted to parameter)", VisualMaterialNodePortType::Float);
			RegisterNode<VisualMaterialNodeVec2Constant>("Constants/Vec2 constant", "Vec2 constant (Can be converted to parameter)", VisualMaterialNodePortType::Vec2);
			RegisterNode<VisualMaterialNodeVec3Constant>("Constants/Vec3 constant", "Vec3 constant (Can be converted to parameter)", VisualMaterialNodePortType::Vec3);
			RegisterNode<VisualMaterialNodeVec4Constant>("Constants/Vec4 constant", "Vec4 constant (Can be converted to parameter)", VisualMaterialNodePortType::Vec4);
			
			RegisterNode<VisualMaterialNodeIntOp>("Operators/Int operator", "Operates on two integers", VisualMaterialNodePortType::Int);
			RegisterNode<VisualMaterialNodeUIntOp>("Operators/UInt operator", "Operates on two unsigned integers", VisualMaterialNodePortType::UInt);
			RegisterNode<VisualMaterialNodeFloatOp>("Operators/Float operator", "Operates on two floats", VisualMaterialNodePortType::Float);
			RegisterNode<VisualMaterialNodeVectorOp>("Operators/Vector operator", "Operates on two vectors", VisualMaterialNodePortType::Vec2);

			RegisterNode<VisualMaterialNodeIntFunc>("Functions/Int function", "Applies a function to an integer", VisualMaterialNodePortType::Int);
			RegisterNode<VisualMaterialNodeUIntFunc>("Functions/UInt function", "Applies a function to an unsigned integer", VisualMaterialNodePortType::UInt);
			RegisterNode<VisualMaterialNodeFloatFunc>("Functions/Float function", "Applies a function to a float", VisualMaterialNodePortType::Float);
			RegisterNode<VisualMaterialNodeVectorFunc>("Functions/Vector function", "Applies a function to a vector", VisualMaterialNodePortType::Vec2);

			RegisterNode<VisualMaterialNodeCompare>("Conditions/Compare", "Compares two values", VisualMaterialNodePortType::Bool);
			RegisterNode<VisualMaterialNodeSwitch>("Conditions/Switch", "Switches between two values based on a boolean", VisualMaterialNodePortType::Float);

			RegisterNode<VisualMaterialNodeSampler>("Textures/Sampler", "Defines sampling modes", VisualMaterialNodePortType::Sampler);
			RegisterNode<VisualMaterialNodeTexture2D>("Textures/Texture2D", "A 2D texture used for sampling", VisualMaterialNodePortType::Vec4);

			RegisterNode<VisualMaterialNodeInput>("Common/Input", "Outputs one of the shader inputs", VisualMaterialNodePortType::Vec2);
			RegisterNode<VisualMaterialNodeEasing>("Common/Easing", "Interpolates between two values", VisualMaterialNodePortType::Vec4);
		}
	};

	static NodePicker Picker = NodePicker(false);

	static bool DragScalarHorizontal(
		const char* label, ImGuiDataType dragType, void* data,
		int count, float width,
		float speed = 1.0f, float min = 0.0f, float max = 0.0f, float padding = 50.0f)
	{
		ImGui::PushItemWidth(width * count + padding);
		bool b = ImGui::DragScalarN(label, dragType, data, count, speed, &min, &max);
		ImGui::PopItemWidth();
		return b;
	}

	static bool ParameterExists(Ref<VisualMaterial> mat, const char* name)
	{
		return mat->Parameters.find(name) != mat->Parameters.end();
	}

	void VisualMaterialNodeEditor::OnImGuiRender(Ref<VisualMaterial> material)
	{
		static bool isCodeWindowOpen = false;
		static std::string code;

		ImNodes::GetIO().AltMouseButton = ImGuiMouseButton_Right;

		if (isCodeWindowOpen)
		{
			ImGui::Begin("Code", &isCodeWindowOpen);
			ImGui::TextWrapped(code.c_str());
			ImGui::End();
		}

		ImGui::Begin("Visual material");
	
		ImGui::Text("Node count %d", (uint32_t)material->Nodes.size());
		ImGui::Text("Connection count %d", (uint32_t)material->Connections.size());

		if (ImGui::Button("Code"))
		{
			isCodeWindowOpen = true;
			code = material->GenerateCode().Code;
		}

		if (ImGui::Button("Compile"))
		{
			material->Recompile();
		}

		ImNodes::BeginNodeEditor();

		uint32_t portID = 0;
		for (auto& [nodeID, node] : material->Nodes)
		{
			DrawVisualMaterialNode(nodeID, material, portID);
		}

		std::vector<uint32_t> invalidConnections;

		for (int i = 0; i < material->Connections.size(); ++i)
		{
			VisualMaterialNodeConnection& connection = material->Connections[i];

			//If connection was using the output of a node which is now nonexistent
			if (auto it = material->Nodes.find(connection.FromNode); it == material->Nodes.end())
			{
				invalidConnections.push_back(i);
				continue;
			}

			const VisualMaterialNode& fromNode = *material->Nodes[connection.FromNode];
			const VisualMaterialNode& toNode = *material->Nodes[connection.ToNode];

			int startPinIdx= ImNodes::ObjectPoolFindOrCreateIndex(ImNodes::EditorContextGet().Pins, connection.FromPort + fromNode.PortOffset + fromNode.GetInputPortCount());
			
			auto& startPin = ImNodes::EditorContextGet().Pins.Pool[startPinIdx];

			auto colStart = startPin.ColorStyle.Background;
			auto colStartHovered = startPin.ColorStyle.Hovered;

			int endPinIdx = ImNodes::ObjectPoolFindOrCreateIndex(ImNodes::EditorContextGet().Pins, connection.ToPort + toNode.PortOffset);

			auto& endPin = ImNodes::EditorContextGet().Pins.Pool[endPinIdx];

			auto colEnd = endPin.ColorStyle.Background;
			auto colEndHovered = endPin.ColorStyle.Hovered;

			ImNodes::PushColorStyle(ImNodesCol_LinkStart, colStart);
			ImNodes::PushColorStyle(ImNodesCol_LinkSelectedStart, colStart);
			ImNodes::PushColorStyle(ImNodesCol_LinkHoveredStart, colStartHovered);

			ImNodes::PushColorStyle(ImNodesCol_LinkEnd, colEnd);
			ImNodes::PushColorStyle(ImNodesCol_LinkSelectedEnd, colEnd);
			ImNodes::PushColorStyle(ImNodesCol_LinkHoveredEnd, colEndHovered);

			ImNodes::Link(i, connection.FromPort + fromNode.PortOffset + fromNode.GetInputPortCount(), connection.ToPort + toNode.PortOffset);

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}

		//Reverse iterating to always have a valid index
		for (auto it = invalidConnections.rbegin(); it != invalidConnections.rend(); ++it)
		{
			material->RemoveConnection(*it);
		}
		
		bool editorHovered = ImNodes::IsEditorHovered();

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		if (ImNodes::IsEditorHovered() && ImGui::GetIO().MouseWheel != 0 && !ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			float zoom = ImNodes::EditorContextGetZoom() + ImGui::GetIO().MouseWheel * 0.1f;
			ImNodes::EditorContextSetZoom(zoom, ImGui::GetMousePos());
		}

		ImVec2 itemMin = ImGui::GetItemRectMin();
		ImVec2 itemMax = ImGui::GetItemRectMax();

		ImVec2 mouse = ImGui::GetMousePos();
		bool isInside = (mouse.x >= itemMin.x && mouse.y >= itemMin.y &&
			mouse.x < itemMax.x && mouse.y < itemMax.y);

		glm::vec2 mouseInEditor = glm::vec2(mouse.x - itemMin.x, mouse.y - itemMin.y);

		static int hoveredLink = -1;

		static int hoveredNode = -1;

		static int droppedLink = -1;

		static int droppedLinkNode = -1;

		{
			static int a;
			if (ImNodes::IsLinkStarted(&a))
			{
				int pinIdx = ImNodes::ObjectPoolFindOrCreateIndex(ImNodes::EditorContextGet().Pins, a);

				auto& pin = ImNodes::EditorContextGet().Pins.Pool[pinIdx];

				//ImNodes::PushColorStyle(ImNodesCol_LinkStart, pin.ColorStyle.Background);
				//ImNodes::PushColorStyle(ImNodesCol_LinkSelectedStart, pin.ColorStyle.Background);
				//ImNodes::PushColorStyle(ImNodesCol_LinkHoveredStart, pin.ColorStyle.Hovered);
			}
			
			if (!ImNodes::IsLinkDropped(&droppedLink, &droppedLinkNode))
			{
				droppedLink = -1;
				droppedLinkNode = -1;
			}
			else
			{
				//ImNodes::PopColorStyle();
				//ImNodes::PopColorStyle();
				//ImNodes::PopColorStyle();

				ImGui::OpenPopup("NodeCreatePopup");

				LocalPos = mouseInEditor;
				StartLink = droppedLink;
				StartNode = droppedLinkNode;
			}
		}

		if (editorHovered)
		{
			if (!ImNodes::IsLinkHovered(&hoveredLink))
				hoveredLink = -1;

			if (!ImNodes::IsNodeHovered(&hoveredNode))
				hoveredNode = -1;

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right) && !ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.001f))
			{
				if (hoveredLink != -1)
					ImGui::OpenPopup("LinkPopup");
				else if (hoveredNode != -1)
					ImGui::OpenPopup("NodePopup");
				else
				{
					ImGui::OpenPopup("NodeCreatePopup");
					LocalPos = mouseInEditor;
					StartLink = -1;
					StartNode = -1;
				}
			}
		}

		if (ImGui::BeginPopupEx(ImGui::GetCurrentWindow()->GetID("NodeCreatePopup"), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Dummy(ImVec2(256, 0));
			DrawNodePicker(material);

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("LinkPopup"))
		{
			if (ImGui::MenuItem("Disconnect"))
			{
				material->RemoveConnection(hoveredLink);
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("NodePopup"))
		{
			auto& node = *material->Nodes[hoveredNode];
			
			if (node.Parameter.Type != VisualMaterialParameterType::None)
			{
				static char name[64] = { 0 };

				static const char* error = nullptr;

				if (!node.Parameter.Active && ImGui::Button("Promote to parameter"))
				{
					ImGui::OpenPopup("Parameter Naming");
					memset(name, 0, sizeof(name));
					error = nullptr;
				}

				if (node.Parameter.Active && ImGui::MenuItem("Demote to parameter"))
				{
					node.DemoteToParameter();
				}

				if (ImGui::BeginPopup("Parameter Naming"))
				{
					if (error)
					{
						ImGui::TextColored(ImVec4(0.75f, 0.075f, 0.075f, 1.0f), error);
					}
					ImGui::InputText("Parameter name*", name, 63);
					if (Input::IsKeyDown(Key::Enter) || Input::IsKeyDown(Key::KPEnter))
					{
						if (name[0] != '\0')
						{
							node.PromoteToParameter(name);
							ImGui::ClosePopupToLevel(0, true);
						}
						else
						{
							error = "Parameter name cannot be empty";
						}
					}
					ImGui::EndPopup();
				}

				ImGui::Separator();
			}

			if (ImGui::MenuItem("Delete"))
			{
				ImNodes::EditorContextGet().Nodes.IdMap.Clear();

				ImNodes::EditorContextGet().NodeDepthOrder.clear();
				ImNodes::GetCurrentContext()->NodeIdxSubmissionOrder.clear();
				material->RemoveNode(hoveredNode);
			}

			ImGui::EndPopup();
		}

		{
			int fromNodeCreated = 0;
			int toNodeCreated = 0;
			int fromPortCreated = 0;
			int toPortCreated = 0;

			if (ImNodes::IsLinkCreated(&fromNodeCreated, &fromPortCreated, &toNodeCreated, &toPortCreated))
			{

				uint32_t fromPort = fromPortCreated - material->Nodes[fromNodeCreated]->PortOffset - material->Nodes[fromNodeCreated]->GetInputPortCount();
				uint32_t toPort = toPortCreated - material->Nodes[toNodeCreated]->PortOffset;

				material->AddConnection(fromNodeCreated, fromPort, toNodeCreated, toPort);
			}
		}



		ImGui::End();
	}

	void VisualMaterialNodeEditor::DrawNodePicker(Ref<VisualMaterial> material)
	{
		Picker.Draw(material);
	}

	static void DrawExpandedVectorPorts(uint32_t componentCount, uint32_t& portID, float width = 0.0f)
	{
		static const char* ChannelNames[] = { "R", "G", "B", "A" };

		static ImVec4 ChannelColors[] = 
		{ 
			ImVec4(0.9294, 0.0824, 0.0824, 1.0f),
			ImVec4(0.062745f, 0.890196f, 0.172549f, 1.0f), 
			ImVec4(0.1098, 0.2549, 0.9882, 1.0f),
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f) 
		};

		if(width == 0.0f)
			width = ImGui::GetItemRectSize().x;

		auto& b = ImNodes::GetCurrentContext()->Style.Colors[ImNodesCol_Pin];
		auto& a = ImNodes::GetCurrentContext()->Style.Colors[ImNodesCol_PinHovered];

		for (uint32_t i = 0; i < componentCount; ++i)
		{
			ImNodes::PushColorStyle(ImNodesCol_Pin, ImGui::GetColorU32(ImGui::GetColorU32(ChannelColors[i]), 0.9f));

			ImNodes::PushColorStyle(ImNodesCol_PinHovered, ImGui::GetColorU32(ChannelColors[i]));

			ImNodes::BeginOutputAttribute(portID++);
			
			ImGui::Indent(width - ImGui::CalcTextSize(ChannelNames[i]).x);
			ImGui::Text(ChannelNames[i]);
			ImGui::Unindent();

			ImNodes::EndOutputAttribute();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}
	}

	template<typename T>
	static void DrawInputPort(void* value, T* node, uint32_t i, uint32_t portID, ImNodesPinShape shape, bool nameOnRight = true, float componentWidth = 30.0f)
	{
		ImNodes::BeginInputAttribute(portID, shape);

		if (!nameOnRight)
		{
			ImGui::Text(node->GetInputPortName(i).data());
		}

		if (value != nullptr && !node->IsInputPortConnected(i))
		{
			if (!nameOnRight)
			{
				ImGui::SameLine();
			}

			std::string text;
			uint32_t componentCount = 0;
			float speed = 1.0f;
			float min = 0.0f;
			float max = 0.0f;
			ImGuiDataType dataType = ImGuiDataType_Float;
			
			switch (node->GetInputPortType(i))
			{
			case VisualMaterialNodePortType::Bool:
			{
				if (*(bool*)value)
				{
					text = "true";
				}
				else
				{
					text = "false";
				}
				componentCount = 1;
				componentWidth = 40.0f;
			} break;
			case VisualMaterialNodePortType::Int:
			{
				text = fmt::format("{}", *(int*)value); 
				componentCount = 1;
				dataType = ImGuiDataType_S32;
			} break;
			case VisualMaterialNodePortType::UInt: 
			{
				text = fmt::format("{}", *(uint32_t*)value);
				componentCount = 1;
				dataType = ImGuiDataType_U32;
				min = 0.0f;
				max = UINT32_MAX;
			} break;
			case VisualMaterialNodePortType::Float: 
			{
				text = fmt::format("{}", *(float*)value);
				componentCount = 1;
				speed = 0.005f;
			} break;
			case VisualMaterialNodePortType::Vec2:
			{
				glm::vec2 v = *(const glm::vec2*)value; text = fmt::format("{},{}", v.x, v.y);
				componentCount = 2;
				speed = 0.005f;
			} break;
			case VisualMaterialNodePortType::Vec3:
			{
				glm::vec3 v = *(const glm::vec3*)value; text = fmt::format("{},{},{}", v.x, v.y, v.z);
				componentCount = 3;
				speed = 0.005f;
			} break;
			case VisualMaterialNodePortType::Vec4:
			{
				glm::vec4 v = *(const glm::vec4*)value; text = fmt::format("{},{},{},{}", v.x, v.y, v.z, v.w);
				componentCount = 4;
				speed = 0.005f;
			} break;
			}

			float textSize = ImGui::CalcTextSize(text.c_str()).x;

			const char* displayedText = text.c_str();

			const float maxWidth = componentWidth * componentCount;

			if (textSize > maxWidth)
			{
				displayedText = "##Button";
				
			}

			if (ImGui::Button(displayedText, ImVec2(maxWidth, 0.0f)))
			{
				ImGui::OpenPopup("##InputChangePopup", ImGuiPopupFlags_NoOpenOverExistingPopup);
			}


			if (ImGui::BeginPopup("##InputChangePopup"))
			{
				if (node->GetInputPortType(i) == VisualMaterialNodePortType::Bool)
				{
					ImGui::Checkbox("On", (bool*)value);
				}
				else
					DragScalarHorizontal("##Drag", dataType, value, componentCount, componentWidth, speed, min, max);

				ImGui::EndPopup();
			}
			
			if (nameOnRight)
			{
				ImGui::SameLine();
			}
		}

		if (nameOnRight)
		{
			ImGui::Text(node->GetInputPortName(i).data());
		}

		ImNodes::EndInputAttribute();
	}

	template<typename T>
	static void DrawFunctionNode(T* node, uint32_t& portID, const char** names, uint32_t nameCount)
	{
		ImNodesPinShape shapes[2] = { ImNodesPinShape_Circle, ImNodesPinShape_CircleFilled };

		ImGui::PushItemWidth(100.0f);

		if (auto newFunc = Combo("Function", names, nameCount, names[(uint32_t)node->Func]); newFunc != -1)
		{
			node->Func = (decltype(node->Func))newFunc;
		}

		ImGui::PopItemWidth();

		float width = ImGui::GetItemRectSize().x;

		auto value = &node->Value;

		DrawInputPort(value, node, 0, portID++, shapes[node->IsInputPortConnected(0)]);
		width = glm::max(width, ImGui::GetItemRectSize().x);

		ImNodes::BeginOutputAttribute(portID++, shapes[1]);

		ImGui::Indent(width - ImGui::CalcTextSize(node->GetOutputPortName(0).data()).x);
		ImGui::Text(node->GetOutputPortName(0).data());
		ImGui::Unindent();

		ImNodes::EndOutputAttribute();

		uint32_t channelCount = node->GetOutputPortCount() - 1;
		
		if (channelCount != 0)
		{
			DrawExpandedVectorPorts(channelCount, portID, width);
		}
	}

	void VisualMaterialNodeEditor::DrawNodeInternal(Ref<VisualMaterial> mat, VisualMaterialNode& node, uint32_t& portID)
	{
		ImNodes::BeginNodeTitleBar();
		
		ImGui::Text(node.GetCaption().data());
		
		ImNodes::EndNodeTitleBar();

		static std::unordered_map<uint32_t, std::string> editingName;

		if(node.Parameter.Active)
		{
			ImGui::SetNextItemWidth(100.0f);
			if(editingName.find(node.NodeID) != editingName.end())
				ImGui::InputText("##ParameterName", &editingName.at(node.NodeID));
			else
				ImGui::InputText("##ParameterName", &node.Parameter.Name);

			if (ImGui::IsItemActivated())
			{
				RenamingNode = node.NodeID;
				editingName[node.NodeID] = node.Parameter.Name;
			}

			if (ImGui::IsItemEdited())
			{
				if (!editingName.at(node.NodeID).empty())
				{
					node.Parameter.Name = editingName.at(node.NodeID);
				}
			}

			if (ImGui::IsItemDeactivated())
			{
				if (editingName.at(node.NodeID).empty())
				{
					editingName.erase(node.NodeID);
					RenamingNode = -1;
				}
				else
				{
					node.Parameter.Name = editingName.at(node.NodeID);
					editingName.erase(node.NodeID);
				}
			}
		}

		ImNodesPinShape shapes[2] = { ImNodesPinShape_Circle, ImNodesPinShape_CircleFilled };

		//Bool constant
		{
			VisualMaterialNodeBoolConstant* constant = dynamic_cast<VisualMaterialNodeBoolConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Checkbox("On", &constant->Value);
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//Int constant
		{
			VisualMaterialNodeIntConstant* constant = dynamic_cast<VisualMaterialNodeIntConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("Int", ImGuiDataType_S32, &constant->Value, 1, 50.0f);
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//UInt constant
		{
			VisualMaterialNodeUIntConstant* constant = dynamic_cast<VisualMaterialNodeUIntConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("UInt", ImGuiDataType_U32, &constant->Value, 1, 50.0f, 1.0f, 0, UINT32_MAX);
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//Float constant
		{
			VisualMaterialNodeFloatConstant* constant = dynamic_cast<VisualMaterialNodeFloatConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("Float", ImGuiDataType_Float, &constant->Value, 1, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//Vec2 constant
		{
			VisualMaterialNodeVec2Constant* constant = dynamic_cast<VisualMaterialNodeVec2Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("Vec2", ImGuiDataType_Float, &constant->Value.x, 2, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(2, portID);

				return;
			}
		}

		//Vec3 constant
		{
			VisualMaterialNodeVec3Constant* constant = dynamic_cast<VisualMaterialNodeVec3Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("Vec3", ImGuiDataType_Float, &constant->Value.x, 3, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(3, portID);

				return;
			}
		}

		//Vec4 constant
		{
			VisualMaterialNodeVec4Constant* constant = dynamic_cast<VisualMaterialNodeVec4Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				DragScalarHorizontal("Vec4", ImGuiDataType_Float, &constant->Value.x, 4, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(4, portID);

				return;
			}
		}

		//Int operator
		{
			VisualMaterialNodeIntOp* op = dynamic_cast<VisualMaterialNodeIntOp*>(&node);

			if (op)
			{
				static const char* OpNames[] =
				{
					"Add",
					"Subtract",
					"Multiply",
					"Divide",
					"Remainder",
					"Max",
					"Min",
					"BitwiseAnd",
					"BitwiseOr",
					"BitsiseXor",
					"LeftShift",
					"RightShift",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newOp = Combo("Operator", OpNames, ARRAYSIZE(OpNames), OpNames[(uint32_t)op->OpType]); newOp != -1)
				{
					op->OpType = (IntOp)newOp;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				int* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				
				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();
				
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//UInt operator
		{
			VisualMaterialNodeUIntOp* op = dynamic_cast<VisualMaterialNodeUIntOp*>(&node);

			if (op)
			{
				static const char* OpNames[] =
				{
					"Add",
					"Subtract",
					"Multiply",
					"Divide",
					"Remainder",
					"Max",
					"Min",
					"BitwiseAnd",
					"BitwiseOr",
					"BitsiseXor",
					"LeftShift",
					"RightShift",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newOp = Combo("Operator", OpNames, ARRAYSIZE(OpNames), OpNames[(uint32_t)op->OpType]); newOp != -1)
				{
					op->OpType = (UIntOp)newOp;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				uint32_t* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImGui::Text(op->GetInputPortName(1).data());
				ImNodes::EndInputAttribute();

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//Float operator
		{
			VisualMaterialNodeFloatOp* op = dynamic_cast<VisualMaterialNodeFloatOp*>(&node);

			if (op)
			{
				static const char* OpNames[] =
				{
					"Add",
					"Subtract",
					"Multiply",
					"Divide",
					"Remainder",
					"Power",
					"Max",
					"Min",
					"Atan2",
					"Step",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newOp = Combo("Operator", OpNames, ARRAYSIZE(OpNames), OpNames[(uint32_t)op->OpType]); newOp != -1)
				{
					op->OpType = (FloatOp)newOp;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				float* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();

				return;
			}
		}

		//Vector operator
		{
			VisualMaterialNodeVectorOp* op = dynamic_cast<VisualMaterialNodeVectorOp*>(&node);

			if (op)
			{
				static const char* TypeNames[] =
				{
					"Vec2",
					"Vec3",
					"Vec4"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)op->Type]); newType != -1)
				{
					op->Type = (VectorType)newType;
				}

				ImGui::PopItemWidth();

				static const char* OpNames[] =
				{
					"Add",
					"Subtract",
					"Multiply",
					"Divide",
					"Remainder",
					"Power",
					"Max",
					"Min",
					"Cross",
					"Dot",
					"Atan2",
					"Reflect",
					"Step",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newOp = Combo("Operator", OpNames, ARRAYSIZE(OpNames), OpNames[(uint32_t)op->OpType]); newOp != -1)
				{
					op->OpType = (VectorOp)newOp;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				glm::vec4* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();
				
				uint32_t channelCount = op->GetOutputPortCount() - 1;

				if (channelCount != 0)
				{
					DrawExpandedVectorPorts(channelCount, portID, width);
				}
				return;
			}
		}

		//Int function
		{
			VisualMaterialNodeIntFunc* func = dynamic_cast<VisualMaterialNodeIntFunc*>(&node);

			if (func)
			{
				static const char* FuncNames[] =
				{
					"Abs",
					"Negate",
					"Sign",
					"BitwiseNot",
				};

				DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return;
			}
		}

		//UInt function
		{
			VisualMaterialNodeUIntFunc* func = dynamic_cast<VisualMaterialNodeUIntFunc*>(&node);

			if (func)
			{
				static const char* FuncNames[] =
				{
					"BitwiseNot",
				};

				DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return;
			}
		}

		//Float function
		{
			VisualMaterialNodeFloatFunc* func = dynamic_cast<VisualMaterialNodeFloatFunc*>(&node);

			if (func)
			{
				static const char* FuncNames[] =
				{
					"Sin",
					"Cos",
					"Tan",
					"ASin",
					"ACos",
					"ATan",
					"Sinh",
					"Cosh",
					"Tanh",
					"Log",
					"Exp",
					"Sqrt",
					"Abs",
					"Sign",
					"Floor",
					"Round",
					"Ceil",
					"Fract",
					"Saturate",
					"Negate",
					"ASinh",
					"ACosh",
					"ATanh",
					"Degrees",
					"Exp2",
					"InvSqrt",
					"Log2",
					"Radians",
					"Reciprocal",
					"RoundEven",
					"Trunc",
					"OneMinus"
				};

				DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return;
			}
		}

		//Vector function
		{
			VisualMaterialNodeVectorFunc* func = dynamic_cast<VisualMaterialNodeVectorFunc*>(&node);

			if (func)
			{
				static const char* TypeNames[] =
				{
					"Vec2",
					"Vec3",
					"Vec4"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)func->Type]); newType != -1)
				{
					func->Type = (VectorType)newType;
				}

				ImGui::PopItemWidth();

				static const char* FuncNames[] =
				{
					"Normalize",
					"Sin",
					"Cos",
					"Tan",
					"ASin",
					"ACos",
					"ATan",
					"Sinh",
					"Cosh",
					"Tanh",
					"Log",
					"Exp",
					"Sqrt",
					"Abs",
					"Sign",
					"Floor",
					"Round",
					"Ceil",
					"Fract",
					"Saturate",
					"Negate",
					"ASinh",
					"ACosh",
					"ATanh",
					"Degrees",
					"Exp2",
					"InvSqrt",
					"Log2",
					"Radians",
					"Reciprocal",
					"RoundEven",
					"Trunc",
					"OneMinus"
				};
				
				DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return;
			}
		}

		//Vector compose
		{
			VisualMaterialNodeVectorCompose* compose = dynamic_cast<VisualMaterialNodeVectorCompose*>(&node);

			if (compose)
			{
				static const char* TypeNames[] =
				{
					"Vec2",
					"Vec3",
					"Vec4"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)compose->Type]); newType != -1)
				{
					compose->Type = (VectorType)newType;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				for (uint32_t i = 0; i < compose->GetInputPortCount(); ++i)
				{
					DrawInputPort(nullptr, compose, i, portID++, shapes[compose->IsInputPortConnected(i)]);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(compose->GetOutputPortName(0).data()).x);
				ImGui::Text(compose->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();
				return;
			}
		}

		//Sampler
		{
			VisualMaterialNodeSampler* sampler = dynamic_cast<VisualMaterialNodeSampler*>(&node);

			if (sampler)
			{
				float width = 0.0f;

				ImGui::PushItemWidth(100.0f);

				static const char* FilterNames[] = { "Nearest", "Linear" };
				
				if (uint32_t newFilter = Combo("Filter", FilterNames, ARRAYSIZE(FilterNames), FilterNames[(uint32_t)sampler->Filter]); newFilter != -1)
				{
					sampler->Filter = (SamplerFilter)newFilter;
				}

				width = glm::max(width, ImGui::GetItemRectSize().x);
				
				static const char* MipModeNames[] = { "Nearest", "Linear" };

				if (uint32_t newMipMode = Combo("Mip mode", MipModeNames, ARRAYSIZE(MipModeNames), MipModeNames[(uint32_t)sampler->MipFilter]); newMipMode != -1)
				{
					sampler->MipFilter = (SamplerMipMapMode)newMipMode;
				}

				width = glm::max(width, ImGui::GetItemRectSize().x);

				static const char* AddressModeNames[] = { "Repeat", "Clamp" };

				if (uint32_t newAddressMode = 
					Combo("Address mode", AddressModeNames, ARRAYSIZE(AddressModeNames), AddressModeNames[(uint32_t)sampler->AddressMode >> 1]); newAddressMode != -1)
				{
					sampler->AddressMode = (SamplerAddressMode)(newAddressMode << 1);
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);
				
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(sampler->GetOutputPortName(0).data()).x);
				ImGui::Text(sampler->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();
				return;
			}
		}

		//Texture2D
		{
			VisualMaterialNodeTexture2D* texture = dynamic_cast<VisualMaterialNodeTexture2D*>(&node);

			if (texture)
			{
				float width = 0.0f;

				ImGui::BeginGroup();
				
				static const char* SamplingTypeNames[] =
				{
					"Data",
					"Color",
					"Normal",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Sampling type", SamplingTypeNames, ARRAYSIZE(SamplingTypeNames), SamplingTypeNames[(uint32_t)texture->SamplingType]); newType != -1)
				{
					texture->SamplingType = (VisualMaterialNodeTexture2D::SampleType)newType;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);

				ImGui::BeginGroup();
				for (uint32_t i = 0; i < texture->GetInputPortCount(); ++i)
				{
					DrawInputPort(nullptr, texture, i, portID++, shapes[texture->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}
				ImGui::EndGroup();
				ImGui::SameLine();

				const ImVec2 imageSize = ImVec2(64, 64);

				static Ref<Texture2D> fallback =
					TextureLibrary::Load("Resources/Icons/TextureNullCross.png", ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

				DescriptorSetLayoutSpecification setSpecs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
				Ref<DescriptorSet> set = DescriptorSet::Create(setSpecs);

				Ref<Texture2D> tex = texture->Image;

				if (!tex)
					tex = fallback;

				Ref<Sampler> sampler = RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear);
				set->Update(tex, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);

				ImTextureID id = (ImTextureID)set->GetSetID();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::ImageButton("##Image", id, imageSize))
				{
					auto path = FileDialogs::OpenFile("jpg (*.jpg)\0*.jpg\0png (*.png)\0*.png\0");
					if (path)
					{
						Ref<Texture2D> t = TextureLibrary::Load(*path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
						if (t)
						{
							texture->Image = t;
						}
					}
				}

				if (tex != fallback && ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(TextureLibrary::GetPath(tex).string().c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				}

				ImGui::PopStyleVar(1);

				ImGui::EndGroup();

				width = glm::max(width, ImGui::GetItemRectSize().x);

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(texture->GetOutputPortName(0).data()).x);
				ImGui::Text(texture->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(4, portID);

				return;
			}
		}

		//Inputs
		{
			VisualMaterialNodeInput* input = dynamic_cast<VisualMaterialNodeInput*>(&node);

			if (input)
			{
				static const char* TypeNames[] =
				{
					"Position",
					"Tangent",
					"Normal",
					"Bitangent",
					"UV",
					"ViewPos",
					"Resolution",
					"ScreenUV",
					"Time",
					"DeltaTime"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)input->Type]); newType != -1)
				{
					input->Type = (InputType)newType;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(input->GetOutputPortName(0).data()).x);
				ImGui::Text(input->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		{
			VisualMaterialNodeEasing* easing = dynamic_cast<VisualMaterialNodeEasing*>(&node);

			if (easing)
			{
				static const char* TypeNames[] =
				{
					"Float",
					"Vec2",
					"Vec3",
					"Vec4"
				};


				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)easing->Type]); newType != -1)
				{
					easing->Type = (VisualMaterialNodeEasing::ValueType)newType;
				}

				ImGui::PopItemWidth();


				static const char* EasingNames[] =
				{

				"Linear",

				"SineIn",
				"SineOut",
				"Sine",

				"QuadIn",
				"QuadOut",
				"Quad",

				"CubicIn",
				"CubicOut",
				"Cubic",

				"ExpoIn",
				"ExpoOut",
				"Expo",

				"CircIn",
				"CircOut",
				"Circ",

				"BounceIn",
				"BounceOut",
				"Bounce"

				};


				ImGui::PushItemWidth(100.0f);

				if (auto newFunc = Combo("Easing function", EasingNames, ARRAYSIZE(EasingNames), EasingNames[(uint32_t)easing->Ease]); newFunc != -1)
				{
					easing->Ease = (EaseType)newFunc;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				for (uint32_t i = 0; i < easing->GetInputPortCount(); ++i)
				{
					DrawInputPort(nullptr, easing, i, portID++, shapes[easing->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(easing->GetOutputPortName(0).data()).x);
				ImGui::Text(easing->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();

				return;
			}
		}

		{
			VisualMaterialNodeCompare* compare = dynamic_cast<VisualMaterialNodeCompare*>(&node);

			if (compare)
			{
				float width = 0.0f;

				static const char* TypeNames[] =
				{
					"Int", 
					"UInt",
					"Float", 
					"Vec2", 
					"Vec3", 
					"Vec4"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)compare->CType]); newType != -1)
				{
					compare->CType = (VisualMaterialNodeCompare::CompareType)newType;
					compare->ValueA = VisualMaterialNodeCompare::ValueType();
					compare->ValueB = VisualMaterialNodeCompare::ValueType();
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);


				static const char* FuncNames[] =
				{
					"Equal",
					"Not equal",
					"Greater than",
					"Greater than or equal",
					"Less than",
					"Less than or equal",
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newFunc = Combo("Function", FuncNames, ARRAYSIZE(FuncNames), FuncNames[(uint32_t)compare->CFunc]); newFunc != -1)
				{
					compare->CFunc = (VisualMaterialNodeCompare::CompareFunc)newFunc;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);


				if (compare->CType == VisualMaterialNodeCompare::CompareType::Vec2 ||
					compare->CType == VisualMaterialNodeCompare::CompareType::Vec3 ||
					compare->CType == VisualMaterialNodeCompare::CompareType::Vec4)
				{
					static const char* CondNames[] =
					{
						"Any",
						"All"
					};

					ImGui::PushItemWidth(100.0f);

					if (auto newCond = Combo("Condition", CondNames, ARRAYSIZE(CondNames), CondNames[(uint32_t)compare->CCond]); newCond != -1)
					{
						compare->CCond = (VisualMaterialNodeCompare::CompareCond)newCond;
					}

					ImGui::PopItemWidth();

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				VisualMaterialNodeCompare::ValueType* values[] = { &compare->ValueA, &compare->ValueB };
				for (uint32_t i = 0; i < compare->GetInputPortCount(); ++i)
				{
					DrawInputPort(values[i], compare, i, portID++, shapes[compare->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(compare->GetOutputPortName(0).data()).x);
				ImGui::Text(compare->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();
				return;
			}
		}

		{
			VisualMaterialNodeSwitch* _switch = dynamic_cast<VisualMaterialNodeSwitch*>(&node);

			if (_switch)
			{
				float width = 0.0f;

				static const char* TypeNames[] =
				{
					"Int",
					"UInt",
					"Float",
					"Vec2",
					"Vec3",
					"Vec4"
				};

				ImGui::PushItemWidth(100.0f);

				if (auto newType = Combo("Type", TypeNames, ARRAYSIZE(TypeNames), TypeNames[(uint32_t)_switch->SType]); newType != -1)
				{
					_switch->SType = (VisualMaterialNodeSwitch::SwitchType)newType;
					_switch->ValueA = VisualMaterialNodeSwitch::ValueType();
					_switch->ValueB = VisualMaterialNodeSwitch::ValueType();
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);


				VisualMaterialNodeSwitch::ValueType* values[] = { &_switch->ValueA, &_switch->ValueB };
				for (uint32_t i = 0; i < _switch->GetInputPortCount(); ++i)
				{
					DrawInputPort(values[i], _switch, i, portID++, shapes[_switch->IsInputPortConnected(i)]);
					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(_switch->GetOutputPortName(0).data()).x);
				ImGui::Text(_switch->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();
				return;
			}
		}

		//Fragment output
		{
			VisualMaterialNodeFragmentOutput* output = dynamic_cast<VisualMaterialNodeFragmentOutput*>(&node);

			if (output)
			{
				DrawInputPort(&output->Albedo, output, 0, portID++, shapes[output->IsInputPortConnected(0)], false);
				DrawInputPort(&output->Metallic, output, 1, portID++, shapes[output->IsInputPortConnected(1)], false);
				DrawInputPort(&output->Roughness, output, 2, portID++, shapes[output->IsInputPortConnected(2)], false);
				DrawInputPort(&output->Emissive, output, 3, portID++, shapes[output->IsInputPortConnected(3)], false);
				DrawInputPort(&output->Normal, output, 4, portID++, shapes[output->IsInputPortConnected(4)], false);

				return;
			}
		}
		
	}

	void VisualMaterialNodeEditor::DrawVisualMaterialNode(uint32_t nodeID, Ref<VisualMaterial> mat, uint32_t& portID)
	{
		VisualMaterialNode& node = *mat->Nodes[nodeID];

		int node_idx = ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(nodeID), -1);

		if (node_idx == -1) 
		{
			ImNodes::SetNodeEditorSpacePos(nodeID, ImVec2(node.Position.x, node.Position.y));
		}

		node.PortOffset = portID;

		ImNodes::BeginNode(nodeID);

		DrawNodeInternal(mat, node, portID);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(nodeID);

		node.Position = glm::vec2(pos.x, pos.y);

		ImNodes::EndNode();
	}

}
