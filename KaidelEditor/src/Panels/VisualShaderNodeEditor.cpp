#include "VisualShaderNodeEditor.h"
#include "Kaidel/Utils/PlatformUtils.h"
#include "Kaidel/Core/Input.h"

#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

#include "Kaidel/Asset/AssetManager.h"

#include "UI/UIHelper.h"
#include "EditorContext.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <imgui/imnodes.h>
#include <imgui/imnodes_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <queue>


namespace Kaidel {

	static uint64_t FindN(const std::string& text, const std::string& query)
	{
		int queryLen = (int)query.length();

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
			std::function<void(Ref<VisualShader> mat)> OnClicked;
			std::string UsageHint;
			std::string Name;
			VisualShaderNodePortType OutputType = VisualShaderNodePortType::Bool;
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

		void _DrawRec(Ref<VisualShader> mat, const Node& node)
		{
			if (node.Children.empty() && !node.Filtered)
			{
				const char* outputName = nullptr;

				switch (node.OutputType)
				{
				case Kaidel::VisualShaderNodePortType::Bool: outputName = "bool"; break;
				case Kaidel::VisualShaderNodePortType::Int: outputName = "int"; break;
				case Kaidel::VisualShaderNodePortType::UInt: outputName = "uint"; break;
				case Kaidel::VisualShaderNodePortType::Float: outputName = "float"; break;
				case Kaidel::VisualShaderNodePortType::Vec2: outputName = "vec2"; break;
				case Kaidel::VisualShaderNodePortType::Vec3: outputName = "vec3"; break;
				case Kaidel::VisualShaderNodePortType::Vec4: outputName = "vec4"; break;
				case Kaidel::VisualShaderNodePortType::Sampler: outputName = "sampler"; break;
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

		void Draw(Ref<VisualShader> mat)
		{
			if (ImGui::InputText("##NodeSearch", &Search) )
			{
				for (auto& [name, child] : Root.Children) {
					_Filter(Root, !Search.empty());
				}
			}

			if (ImGui::BeginChild("##Nodes", ImVec2(-1, 0), ImGuiChildFlags_Borders |
				ImGuiChildFlags_AlwaysAutoResize | 
				ImGuiChildFlags_AutoResizeY | 
				ImGuiChildFlags_AutoResizeX))
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
		void RegisterNode(const std::string& path, const std::string& hint, VisualShaderNodePortType outputType)
		{
			Node& node = Walk(path, Root);
			node.UsageHint = hint;
			node.OutputType = outputType;
			node.OnClicked = [](Ref<VisualShader> material) { 
				auto& node = material->CreateNode<T>();
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

				AssetManager::SetDirty(material->GetHandle());
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

				return Walk(path.substr(loc + 1), node);
			}
		}

		NodePicker() = default;

		NodePicker(bool) 
		{
			RegisterNode<VisualShaderNodeBoolConstant>("Constants/Bool constant", "Boolean constant (Can be converted to parameter)", VisualShaderNodePortType::Bool);
			RegisterNode<VisualShaderNodeIntConstant>("Constants/Int constant", "Integer constant (Can be converted to parameter)", VisualShaderNodePortType::Int);
			RegisterNode<VisualShaderNodeUIntConstant>("Constants/UInt constant", "Unsigned integer constant (Can be converted to parameter)", VisualShaderNodePortType::UInt);
			RegisterNode<VisualShaderNodeFloatConstant>("Constants/Float constant", "Float constant (Can be converted to parameter)", VisualShaderNodePortType::Float);
			RegisterNode<VisualShaderNodeVec2Constant>("Constants/Vec2 constant", "Vec2 constant (Can be converted to parameter)", VisualShaderNodePortType::Vec2);
			RegisterNode<VisualShaderNodeVec3Constant>("Constants/Vec3 constant", "Vec3 constant (Can be converted to parameter)", VisualShaderNodePortType::Vec3);
			RegisterNode<VisualShaderNodeVec4Constant>("Constants/Vec4 constant", "Vec4 constant (Can be converted to parameter)", VisualShaderNodePortType::Vec4);
			
			RegisterNode<VisualShaderNodeIntOp>("Operators/Int operator", "Operates on two integers", VisualShaderNodePortType::Int);
			RegisterNode<VisualShaderNodeUIntOp>("Operators/UInt operator", "Operates on two unsigned integers", VisualShaderNodePortType::UInt);
			RegisterNode<VisualShaderNodeFloatOp>("Operators/Float operator", "Operates on two floats", VisualShaderNodePortType::Float);
			RegisterNode<VisualShaderNodeVectorOp>("Operators/Vector operator", "Operates on two vectors", VisualShaderNodePortType::Vec2);

			RegisterNode<VisualShaderNodeIntFunc>("Functions/Int function", "Applies a function to an integer", VisualShaderNodePortType::Int);
			RegisterNode<VisualShaderNodeUIntFunc>("Functions/UInt function", "Applies a function to an unsigned integer", VisualShaderNodePortType::UInt);
			RegisterNode<VisualShaderNodeFloatFunc>("Functions/Float function", "Applies a function to a float", VisualShaderNodePortType::Float);
			RegisterNode<VisualShaderNodeVectorFunc>("Functions/Vector function", "Applies a function to a vector", VisualShaderNodePortType::Vec2);

			RegisterNode<VisualShaderNodeCompare>("Conditions/Compare", "Compares two values", VisualShaderNodePortType::Bool);
			RegisterNode<VisualShaderNodeSwitch>("Conditions/Switch", "Switches between two values based on a boolean", VisualShaderNodePortType::Float);

			RegisterNode<VisualShaderNodeSampler>("Textures/Sampler", "Defines sampling modes", VisualShaderNodePortType::Sampler);
			RegisterNode<VisualShaderNodeTexture2D>("Textures/Texture2D", "A 2D texture used for sampling", VisualShaderNodePortType::Vec4);

			RegisterNode<VisualShaderNodeInput>("Common/Input", "Outputs one of the shader inputs", VisualShaderNodePortType::Vec2);
			RegisterNode<VisualShaderNodeEasing>("Common/Easing", "Interpolates between two values", VisualShaderNodePortType::Vec4);
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

	static bool ParameterExists(Ref<VisualShader> mat, const char* name)
	{
		return mat->Parameters.find(name) != mat->Parameters.end();
	}

	void VisualShaderNodeEditor::OnImGuiRender()
	{
		Asset asset = EditorContext::SelectedAsset();
		
		Ref<VisualShader> shader;

		if (asset && asset->GetAssetType() == AssetType::VisualShader)
		{
			shader = asset.As<VisualShader>();
		}

		static bool isCodeWindowOpen = false;

		ImNodes::GetIO().AltMouseButton = ImGuiMouseButton_Right;

		if (isCodeWindowOpen)
		{
			if (!shader)
			{
				isCodeWindowOpen = false;
			}
			else
			{
				ImGui::Begin("Code", &isCodeWindowOpen);
				ImGui::TextWrapped(shader->Code.Code.c_str());
				ImGui::End();
			}
		}

		Styler styler;

		styler.PushStyle(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
		styler.PushStyle(ImGuiStyleVar_FramePadding, ImVec2(9, 3));
		styler.PushStyle(ImGuiStyleVar_ItemSpacing, ImVec2(20, 5));
		
		ImGui::Begin("Visual material");

		ImGui::BeginChild("##Nodes", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.y));
		ImNodes::BeginNodeEditor();
		
		if (shader)
		{
			DrawNodes(shader);
		}

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		ImGui::EndChild();

		ImGui::BeginChild("##Footer", ImVec2(-1.0f, ImGui::GetFrameHeight()));

		std::string nodeCount = fmt::format("{} Nodes", shader ? (uint32_t)shader->Nodes.size() : 0);

		ImGui::Text(nodeCount.c_str());

		ImGui::SameLine();
		
		ImGui::PushMultiItemsWidths(2, 50.0f);

		float width = ImGui::GetContentRegionAvail().x;

		width -= ImGui::CalcItemWidth();
		width -= ImGui::CalcItemWidth();

		ImGui::SetCursorPosX(width);

		ImGui::BeginDisabled(!shader);

		if (ImGui::Button("Code"))
		{
			isCodeWindowOpen = true;
		}

		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (shader)
		{
			const auto& mtd = AssetManager::GetAssetMetadata(shader->GetHandle());

			ImGui::BeginDisabled(!mtd.Dirty);

			if (ImGui::Button("Save"))
			{
				shader->Recompile();
				AssetManager::Serialize(mtd.Path, Asset(shader));
			}

			ImGui::EndDisabled();
		}
		else
		{
			ImGui::Button("Save");
		}
		
		ImGui::PopItemWidth();

		ImGui::EndDisabled();

		ImGui::EndChild();

		if(shader)
		{
			HandleInteractions(shader);
		}

		ImGui::End();
	}

	void VisualShaderNodeEditor::DrawNodePicker(Ref<VisualShader> material)
	{
		Picker.Draw(material);
	}

	static void DrawExpandedVectorPorts(uint32_t componentCount, uint32_t& portID, float width = 0.0f)
	{
		static const char* ChannelNames[] = { "R", "G", "B", "A" };

		static ImVec4 ChannelColors[] = 
		{ 
			ImVec4(0.9294f, 0.0824f, 0.0824f, 1.0f),
			ImVec4(0.062745f, 0.890196f, 0.172549f, 1.0f), 
			ImVec4(0.1098f, 0.2549f, 0.9882f, 1.0f),
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
	static bool DrawInputPort(void* value, T* node, uint32_t i, uint32_t portID, ImNodesPinShape shape, bool nameOnRight = true, float componentWidth = 30.0f)
	{
		bool changed = false;

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
			case VisualShaderNodePortType::Bool:
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
			case VisualShaderNodePortType::Int:
			{
				text = fmt::format("{}", *(int*)value); 
				componentCount = 1;
				dataType = ImGuiDataType_S32;
			} break;
			case VisualShaderNodePortType::UInt: 
			{
				text = fmt::format("{}", *(uint32_t*)value);
				componentCount = 1;
				dataType = ImGuiDataType_U32;
				min = 0.0f;
				max = (float)UINT32_MAX;
			} break;
			case VisualShaderNodePortType::Float: 
			{
				text = fmt::format("{}", *(float*)value);
				componentCount = 1;
				speed = 0.005f;
			} break;
			case VisualShaderNodePortType::Vec2:
			{
				glm::vec2 v = *(const glm::vec2*)value; text = fmt::format("{},{}", v.x, v.y);
				componentCount = 2;
				speed = 0.005f;
			} break;
			case VisualShaderNodePortType::Vec3:
			{
				glm::vec3 v = *(const glm::vec3*)value; text = fmt::format("{},{},{}", v.x, v.y, v.z);
				componentCount = 3;
				speed = 0.005f;
			} break;
			case VisualShaderNodePortType::Vec4:
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
				if (node->GetInputPortType(i) == VisualShaderNodePortType::Bool)
				{
					changed = ImGui::Checkbox("On", (bool*)value);
				}
				else
					changed = DragScalarHorizontal("##Drag", dataType, value, componentCount, componentWidth, speed, min, max);

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

		return changed;
	}

	template<typename T>
	static bool DrawFunctionNode(T* node, uint32_t& portID, const char** names, uint32_t nameCount)
	{
		bool changed = false;

		ImNodesPinShape shapes[2] = { ImNodesPinShape_Circle, ImNodesPinShape_CircleFilled };

		ImGui::PushItemWidth(100.0f);

		if (auto newFunc = Combo("Function", names, nameCount, names[(uint32_t)node->Func]); newFunc != -1)
		{
			node->Func = (decltype(node->Func))newFunc;
			changed = true;
		}

		ImGui::PopItemWidth();

		float width = ImGui::GetItemRectSize().x;

		auto value = &node->Value;

		if (DrawInputPort(value, node, 0, portID++, shapes[node->IsInputPortConnected(0)]))
		{
			changed = true;
		}

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

		return changed;
	}

	bool VisualShaderNodeEditor::DrawNodeInternal(Ref<VisualShader> mat, VisualShaderNode& node, uint32_t& portID)
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

					AssetManager::SetDirty(mat->GetHandle());
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

					AssetManager::SetDirty(mat->GetHandle());

					editingName.erase(node.NodeID);
				}
			}
		}

		bool changed = false;

		ImNodesPinShape shapes[2] = { ImNodesPinShape_Circle, ImNodesPinShape_CircleFilled };

		//Bool constant
		{
			VisualShaderNodeBoolConstant* constant = dynamic_cast<VisualShaderNodeBoolConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = ImGui::Checkbox("On", &constant->Value);
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Int constant
		{
			VisualShaderNodeIntConstant* constant = dynamic_cast<VisualShaderNodeIntConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("Int", ImGuiDataType_S32, &constant->Value, 1, 50.0f);
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//UInt constant
		{
			VisualShaderNodeUIntConstant* constant = dynamic_cast<VisualShaderNodeUIntConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("UInt", ImGuiDataType_U32, &constant->Value, 1, 50.0f, 1.0f, 0, (float)UINT32_MAX);
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Float constant
		{
			VisualShaderNodeFloatConstant* constant = dynamic_cast<VisualShaderNodeFloatConstant*>(&node);

			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("Float", ImGuiDataType_Float, &constant->Value, 1, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Vec2 constant
		{
			VisualShaderNodeVec2Constant* constant = dynamic_cast<VisualShaderNodeVec2Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("Vec2", ImGuiDataType_Float, &constant->Value.x, 2, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(2, portID);

				return changed;
			}
		}

		//Vec3 constant
		{
			VisualShaderNodeVec3Constant* constant = dynamic_cast<VisualShaderNodeVec3Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("Vec3", ImGuiDataType_Float, &constant->Value.x, 3, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(3, portID);

				return changed;
			}
		}

		//Vec4 constant
		{
			VisualShaderNodeVec4Constant* constant = dynamic_cast<VisualShaderNodeVec4Constant*>(&node);
			
			if (constant)
			{
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				changed = DragScalarHorizontal("Vec4", ImGuiDataType_Float, &constant->Value.x, 4, 50.0f, 0.005f);
				ImNodes::EndOutputAttribute();

				DrawExpandedVectorPorts(4, portID);

				return changed;
			}
		}

		//Int operator
		{
			VisualShaderNodeIntOp* op = dynamic_cast<VisualShaderNodeIntOp*>(&node);

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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				int* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					if (DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				
				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();
				
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//UInt operator
		{
			VisualShaderNodeUIntOp* op = dynamic_cast<VisualShaderNodeUIntOp*>(&node);

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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				uint32_t* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					if (DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImGui::Text(op->GetInputPortName(1).data());
				ImNodes::EndInputAttribute();

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Float operator
		{
			VisualShaderNodeFloatOp* op = dynamic_cast<VisualShaderNodeFloatOp*>(&node);

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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				float* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					if (DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(op->GetOutputPortName(0).data()).x);
				ImGui::Text(op->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Vector operator
		{
			VisualShaderNodeVectorOp* op = dynamic_cast<VisualShaderNodeVectorOp*>(&node);

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
					changed = true;
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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				glm::vec4* values[2] = { &op->ValueA, &op->ValueB };

				for (uint32_t i = 0; i < 2; ++i)
				{
					if (DrawInputPort(values[i], op, i, portID++, shapes[op->IsInputPortConnected(i)]))
					{
						changed = true;
					}

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
				return changed;
			}
		}

		//Int function
		{
			VisualShaderNodeIntFunc* func = dynamic_cast<VisualShaderNodeIntFunc*>(&node);

			if (func)
			{
				static const char* FuncNames[] =
				{
					"Abs",
					"Negate",
					"Sign",
					"BitwiseNot",
				};

				changed = DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return changed;
			}
		}

		//UInt function
		{
			VisualShaderNodeUIntFunc* func = dynamic_cast<VisualShaderNodeUIntFunc*>(&node);

			if (func)
			{
				static const char* FuncNames[] =
				{
					"BitwiseNot",
				};

				changed = DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return changed;
			}
		}

		//Float function
		{
			VisualShaderNodeFloatFunc* func = dynamic_cast<VisualShaderNodeFloatFunc*>(&node);

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

				changed = DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return changed;
			}
		}

		//Vector function
		{
			VisualShaderNodeVectorFunc* func = dynamic_cast<VisualShaderNodeVectorFunc*>(&node);

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
				
				changed = DrawFunctionNode(func, portID, FuncNames, ARRAYSIZE(FuncNames));
				return changed;
			}
		}

		//Vector compose
		{
			VisualShaderNodeVectorCompose* compose = dynamic_cast<VisualShaderNodeVectorCompose*>(&node);

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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				for (uint32_t i = 0; i < compose->GetInputPortCount(); ++i)
				{
					if (DrawInputPort(nullptr, compose, i, portID++, shapes[compose->IsInputPortConnected(i)]))
					{
						changed = true;
					}
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);

				ImGui::Indent(width - ImGui::CalcTextSize(compose->GetOutputPortName(0).data()).x);
				ImGui::Text(compose->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();
				return changed;
			}
		}

		//Sampler
		{
			VisualShaderNodeSampler* sampler = dynamic_cast<VisualShaderNodeSampler*>(&node);

			if (sampler)
			{
				float width = 0.0f;

				ImGui::PushItemWidth(100.0f);

				static const char* FilterNames[] = { "Nearest", "Linear" };
				
				if (uint32_t newFilter = Combo("Filter", FilterNames, ARRAYSIZE(FilterNames), FilterNames[(uint32_t)sampler->Filter]); newFilter != -1)
				{
					sampler->Filter = (SamplerFilter)newFilter;
					changed = true;
				}

				width = glm::max(width, ImGui::GetItemRectSize().x);
				
				static const char* MipModeNames[] = { "Nearest", "Linear" };

				if (uint32_t newMipMode = Combo("Mip mode", MipModeNames, ARRAYSIZE(MipModeNames), MipModeNames[(uint32_t)sampler->MipFilter]); newMipMode != -1)
				{
					sampler->MipFilter = (SamplerMipMapMode)newMipMode;
					changed = true;
				}

				width = glm::max(width, ImGui::GetItemRectSize().x);

				static const char* AddressModeNames[] = { "Repeat", "Clamp" };

				if (uint32_t newAddressMode = 
					Combo("Address mode", AddressModeNames, ARRAYSIZE(AddressModeNames), AddressModeNames[(uint32_t)sampler->AddressMode >> 1]); newAddressMode != -1)
				{
					sampler->AddressMode = (SamplerAddressMode)(newAddressMode << 1);
					changed = true;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);
				
				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(sampler->GetOutputPortName(0).data()).x);
				ImGui::Text(sampler->GetOutputPortName(0).data());
				ImGui::Unindent();

				ImNodes::EndOutputAttribute();
				return changed;
			}
		}

		//Texture2D
		{
			VisualShaderNodeTexture2D* texture = dynamic_cast<VisualShaderNodeTexture2D*>(&node);

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
					texture->SamplingType = (VisualShaderNodeTexture2D::SampleType)newType;
					changed = true;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);

				ImGui::BeginGroup();
				for (uint32_t i = 0; i < texture->GetInputPortCount(); ++i)
				{
					if (DrawInputPort(nullptr, texture, i, portID++, shapes[texture->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}
				ImGui::EndGroup();
				ImGui::SameLine();

				const ImVec2 imageSize = ImVec2(64, 64);

				static Ref<Texture2D> fallback =
					RendererGlobals::GetSingleColorTexture(glm::vec4(0.0f))->GetTexture();

				DescriptorSetLayoutSpecification setSpecs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
				Ref<DescriptorSet> set = DescriptorSet::Create(setSpecs);

				Ref<Texture2D> tex = texture->Image->GetTexture();

				if (!tex)
					tex = fallback;

				Ref<Sampler> sampler = RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Linear);
				set->Update(tex, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);

				ImTextureID id = (ImTextureID)set->GetSetID();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::ImageButton("##Image", id, imageSize))
				{
					AssetChooser::Render(AssetFilter({ AssetType::SpriteSheet }), [texture, mat](Asset asset)
						{
							auto t = asset.As<SpriteSheet>();

							if (texture->Image != t)
							{
								texture->Image = t;
								if (texture->Parameter.Active)
								{
									mat->DefaultTextureValues[texture->Parameter.Name] = texture->GetDescriptorTexture(0)->GetTexture();
								}
								else
								{
									mat->DefaultTextureValues[NODE_DESCRIPTOR_NAME(texture->NodeID, 0)] = texture->GetDescriptorTexture(0)->GetTexture();
								}
								AssetManager::SetDirty(mat->GetHandle());
							}
						});
				}

				if (tex != fallback && ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					//ImGui::Text(AssetManager::GetAssetMetadata(tex->GetHandle()).Path.string().c_str());
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

				return changed;
			}
		}

		//Inputs 
		{
			VisualShaderNodeInput* input = dynamic_cast<VisualShaderNodeInput*>(&node);

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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(input->GetOutputPortName(0).data()).x);
				ImGui::Text(input->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Easing
		{
			VisualShaderNodeEasing* easing = dynamic_cast<VisualShaderNodeEasing*>(&node);

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
					easing->Type = (VisualShaderNodeEasing::ValueType)newType;
					changed = true;
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
					changed = true;
				}

				ImGui::PopItemWidth();

				float width = ImGui::GetItemRectSize().x;

				for (uint32_t i = 0; i < easing->GetInputPortCount(); ++i)
				{
					if (DrawInputPort(nullptr, easing, i, portID++, shapes[easing->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(easing->GetOutputPortName(0).data()).x);
				ImGui::Text(easing->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();

				return changed;
			}
		}

		//Conditionals
		{
			VisualShaderNodeCompare* compare = dynamic_cast<VisualShaderNodeCompare*>(&node);

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
					compare->CType = (VisualShaderNodeCompare::CompareType)newType;
					compare->ValueA = VisualShaderNodeCompare::ValueType();
					compare->ValueB = VisualShaderNodeCompare::ValueType();
					changed = true;
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
					compare->CFunc = (VisualShaderNodeCompare::CompareFunc)newFunc;
					changed = true;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);


				if (compare->CType == VisualShaderNodeCompare::CompareType::Vec2 ||
					compare->CType == VisualShaderNodeCompare::CompareType::Vec3 ||
					compare->CType == VisualShaderNodeCompare::CompareType::Vec4)
				{
					static const char* CondNames[] =
					{
						"Any",
						"All"
					};

					ImGui::PushItemWidth(100.0f);

					if (auto newCond = Combo("Condition", CondNames, ARRAYSIZE(CondNames), CondNames[(uint32_t)compare->CCond]); newCond != -1)
					{
						compare->CCond = (VisualShaderNodeCompare::CompareCond)newCond;
						changed = true;
					}

					ImGui::PopItemWidth();

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				VisualShaderNodeCompare::ValueType* values[] = { &compare->ValueA, &compare->ValueB };
				for (uint32_t i = 0; i < compare->GetInputPortCount(); ++i)
				{
					if (DrawInputPort(values[i], compare, i, portID++, shapes[compare->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(compare->GetOutputPortName(0).data()).x);
				ImGui::Text(compare->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();
				return changed;
			}
		}

		{
			VisualShaderNodeSwitch* _switch = dynamic_cast<VisualShaderNodeSwitch*>(&node);

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
					_switch->SType = (VisualShaderNodeSwitch::SwitchType)newType;
					_switch->ValueA = VisualShaderNodeSwitch::ValueType();
					_switch->ValueB = VisualShaderNodeSwitch::ValueType();
					changed = true;
				}

				ImGui::PopItemWidth();

				width = glm::max(width, ImGui::GetItemRectSize().x);


				void* values[] = { &_switch->ValueA, &_switch->ValueB, &_switch->Condition };
				for (uint32_t i = 0; i < _switch->GetInputPortCount(); ++i)
				{
					if (DrawInputPort(values[i], _switch, i, portID++, shapes[_switch->IsInputPortConnected(i)]))
					{
						changed = true;
					}

					width = glm::max(width, ImGui::GetItemRectSize().x);
				}

				ImNodes::BeginOutputAttribute(portID++, shapes[1]);
				ImGui::Indent(width - ImGui::CalcTextSize(_switch->GetOutputPortName(0).data()).x);
				ImGui::Text(_switch->GetOutputPortName(0).data());
				ImNodes::EndOutputAttribute();
				return changed;
			}
		}

		//Fragment output
		{
			VisualShaderNodeFragmentOutput* output = dynamic_cast<VisualShaderNodeFragmentOutput*>(&node);

			if (output)
			{
				if(DrawInputPort(&output->Albedo, output, 0, portID++, shapes[output->IsInputPortConnected(0)], false))
				{
					changed = true;
				}

				if(DrawInputPort(&output->Metallic, output, 1, portID++, shapes[output->IsInputPortConnected(1)], false))
				{
					changed = true;
				}

				if(DrawInputPort(&output->Roughness, output, 2, portID++, shapes[output->IsInputPortConnected(2)], false))
				{
					changed = true;
				}

				if(DrawInputPort(&output->Emissive, output, 3, portID++, shapes[output->IsInputPortConnected(3)], false))
				{
					changed = true;
				}

				if(DrawInputPort(&output->Normal, output, 4, portID++, shapes[output->IsInputPortConnected(4)], false))
				{
					changed = true;
				}


				return changed;
			}
		}
		
		KD_CORE_ASSERT(false);

		return changed;
	}

	void VisualShaderNodeEditor::DrawVisualShaderNode(uint32_t nodeID, Ref<VisualShader> mat, uint32_t& portID)
	{
		VisualShaderNode& node = *mat->Nodes[nodeID];

		int node_idx = ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(nodeID), -1);

		if (node_idx == -1) 
		{
			ImNodes::SetNodeEditorSpacePos(nodeID, ImVec2(node.Position.x, node.Position.y));
		}

		node.PortOffset = portID;

		ImNodes::BeginNode(nodeID);

		bool changed = false;

		if (DrawNodeInternal(mat, node, portID))
		{
			changed = true;
		}

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(nodeID);

		if (node.Position != glm::vec2(pos.x, pos.y))
		{
			changed = true;
		}
		node.Position = glm::vec2(pos.x, pos.y);

		ImNodes::EndNode();

		if (changed)
		{
			AssetManager::SetDirty(mat->GetHandle());
		}
	}

	void VisualShaderNodeEditor::DrawNodes(Ref<VisualShader> shader)
	{
		uint32_t portID = 0;
		for (auto& [nodeID, node] : shader->Nodes)
		{
			DrawVisualShaderNode(nodeID, shader, portID);
		}

		std::vector<uint32_t> invalidConnections;

		for (int i = 0; i < shader->Connections.size(); ++i)
		{
			VisualShaderNodeConnection& connection = shader->Connections[i];

			//If connection was using the output of a node which is now nonexistent
			if (auto it = shader->Nodes.find(connection.FromNode); it == shader->Nodes.end())
			{
				invalidConnections.push_back(i);
				continue;
			}

			const VisualShaderNode& fromNode = *shader->Nodes[connection.FromNode];
			const VisualShaderNode& toNode = *shader->Nodes[connection.ToNode];

			int startPinIdx = ImNodes::ObjectPoolFindOrCreateIndex(ImNodes::EditorContextGet().Pins, connection.FromPort + fromNode.PortOffset + fromNode.GetInputPortCount());

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
			shader->RemoveConnection(*it);
		}
	}

	void VisualShaderNodeEditor::HandleInteractions(Ref<VisualShader> shader)
	{
		if (ImNodes::IsEditorHovered() && ImGui::GetIO().MouseWheel != 0 && !ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			float zoom = ImNodes::EditorContextGetZoom() + ImGui::GetIO().MouseWheel * 0.1f;
			ImNodes::EditorContextSetZoom(zoom, ImGui::GetMousePos());
		}

		bool editorHovered = ImNodes::IsEditorHovered();
		
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

		if (ImGui::BeginPopupEx(ImGui::GetCurrentWindow()->GetID("NodeCreatePopup"), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Dummy(ImVec2(256, 0));
			DrawNodePicker(shader);

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("LinkPopup"))
		{
			if (ImGui::MenuItem("Disconnect"))
			{
				shader->RemoveConnection(hoveredLink);
				AssetManager::SetDirty(shader->GetHandle());
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("NodePopup"))
		{
			auto& node = *shader->Nodes[hoveredNode];

			if (node.Parameter.Type != VisualShaderParameterType::None)
			{
				static char name[64] = { 0 };

				static const char* error = nullptr;

				if (!node.Parameter.Active && ImGui::Button("Promote to parameter"))
				{
					ImGui::OpenPopup("Parameter Naming");
					memset(name, 0, sizeof(name));
					error = nullptr;
				}

				if (node.Parameter.Active && ImGui::MenuItem("Demote to Constant"))
				{
					node.DemoteToParameter();
					AssetManager::SetDirty(shader->GetHandle());
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
							AssetManager::SetDirty(shader->GetHandle());

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
				shader->RemoveNode(hoveredNode);

				AssetManager::SetDirty(shader->GetHandle());
			}

			ImGui::EndPopup();
		}

		{
			int fromNodeCreated = 0;
			int toNodeCreated = 0;
			int fromPortCreated = 0;
			int toPortCreated = 0;

			if (ImNodes::IsLinkCreated(&fromNodeCreated, &fromPortCreated, &toNodeCreated, &toPortCreated) && fromNodeCreated != toNodeCreated && fromPortCreated != toPortCreated)
			{

				uint32_t fromPort = fromPortCreated - shader->Nodes[fromNodeCreated]->PortOffset - shader->Nodes[fromNodeCreated]->GetInputPortCount();
				uint32_t toPort = toPortCreated - shader->Nodes[toNodeCreated]->PortOffset;

				shader->AddConnection(fromNodeCreated, fromPort, toNodeCreated, toPort);

				AssetManager::SetDirty(shader->GetHandle());
			}
		}
	}

}
