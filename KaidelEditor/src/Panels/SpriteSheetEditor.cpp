#include "SpriteSheetEditor.h"

#include <Kaidel\Renderer\GraphicsAPI\DescriptorSet.h>

#include <Kaidel\Core\Application.h>

#include "imgui/misc/cpp/imgui_stdlib.h"

template<glm::length_t L, typename T, glm::qualifier Q>
glm::vec<L, T, Q> MoveTowards(const glm::vec<L, T, Q>& current,
	const glm::vec<L, T, Q>& target,
	T maxDistanceDelta)
{
	const glm::vec<L, T, Q> toTarget = target - current;
	const T sqrDist = glm::dot(toTarget, toTarget);

	// Already there, or close enough to arrive this step.
	if (sqrDist == T(0) ||
		(maxDistanceDelta >= T(0) && sqrDist <= maxDistanceDelta * maxDistanceDelta))
		return target;

	const T dist = glm::sqrt(sqrDist);
	return current + toTarget / dist * maxDistanceDelta;
}

namespace Kaidel
{
	enum Handle { Handle_Left = 1, Handle_Right = 2, Handle_Top = 4, Handle_Bottom = 8 };
	static struct {
		bool IsOpen = false;

		glm::vec2 TopLeft = glm::vec2(0.0f), BottomRight = glm::vec2(1.0f);
		glm::vec2 Center = glm::vec2(0.5f);

		float Zoom = 0.5f;

		bool ImageActive = false;
		bool ImageHovered = false;
		bool ImageLeftClicked = false;
		bool ImageLeftHeld = false;
		bool ImagePanningRightClick = false;
		bool ImagePanningLeftClick = false;


		glm::vec2 MouseUVPos;

		uint32_t SliceType = 0;

		uint32_t SliceParams[2] = { 0, 0 };

		uint32_t Selected = -1;

		ImVec2 HandleLeft, HandleRight, HandleTop, HandleBottom;

		ImVec2 ImageTopLeft;

		int ResizeHandle;

		bool Resizing = false;
		bool Moving = false;
		bool Creating = false;

		ImVec2 StartCreatePos;
		ImVec2 CreateSize;

		Ref<SpriteSheet> Sheet;
		Ref<SpriteSheet> OrgSheet;
	} s_Data;


	static const constexpr ImVec2 HandleHalfSize = ImVec2(4.0f, 4.0f);

	static bool ShouldClampPan(float a, float b, float delta)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		//panning left
		if (a == 0.0f && delta < 0.0f)
		{
			return true;
		}
		//panning right
		if (b == 1.0f && delta > 0.0f)
		{
			return true;
		}

		return false;
	}

	static bool CheckHit(const ImVec2& rel)
	{
		return rel.x >= -HandleHalfSize.x && rel.x <= HandleHalfSize.x
			&& rel.y >= -HandleHalfSize.y && rel.y <= HandleHalfSize.y;
	}

	static bool HitHandle()
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		s_Data.ResizeHandle = 0;
		
		if (s_Data.Selected == -1)
		{
			return false;
		}
		
		//check left handle.
		ImVec2 rel = mousePos - s_Data.HandleLeft;
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Left;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			return true;
		}

		//check right handle.
		rel = mousePos - s_Data.HandleRight;
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Right;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			return true;
		}

		//check top handle.
		rel = mousePos - s_Data.HandleTop;
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Top;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			return true;
		}

		//check bottom handle.
		rel = mousePos - s_Data.HandleBottom;
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Bottom;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			return true;
		}

		//check top left.
		rel = mousePos - ImVec2(s_Data.HandleLeft.x, s_Data.HandleTop.y);
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Top | Handle_Left;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
			return true;
		}

		//check top right.
		rel = mousePos - ImVec2(s_Data.HandleRight.x, s_Data.HandleTop.y);
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Top | Handle_Right;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
			return true;
		}

		//check bottom left.
		rel = mousePos - ImVec2(s_Data.HandleLeft.x, s_Data.HandleBottom.y);
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Bottom | Handle_Left;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
			return true;
		}

		//check top left.
		rel = mousePos - ImVec2(s_Data.HandleRight.x, s_Data.HandleBottom.y);
		if (CheckHit(rel))
		{
			s_Data.ResizeHandle = Handle_Bottom | Handle_Right;
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
			return true;
		}

		return false;
	}
	
	void SpriteSheetEditor::Open(Ref<SpriteSheet> sheet)
	{
		s_Data.IsOpen = true;
		//make a copy
		s_Data.Sheet = CreateRef<SpriteSheet>(*sheet);
		s_Data.OrgSheet = std::move(sheet);
	}

	bool SpriteSheetEditor::OnImGuiRender()
	{
		bool changed = false;

		auto& io = ImGui::GetIO();

		if (!s_Data.IsOpen)
		{
			s_Data.OrgSheet = {};
			s_Data.Sheet = {};
			s_Data.Selected = -1;
			s_Data.Moving = s_Data.Resizing = false;
			return changed;
		}

		ImGui::Begin("Sprite editor", &s_Data.IsOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

		DrawImage();

		DrawSpriteProperties();

		if (s_Data.ResizeHandle == 0 && !s_Data.Moving)
		{
			HitHandle();
		}

		HandlePan();

		if ((s_Data.ResizeHandle != 0) && ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			HandleSpriteResize();
		}
		else if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && s_Data.Selected != -1)
		{
			HandleMove();
		}

		if (s_Data.ImageHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			if (!s_Data.Creating)
			{
				s_Data.Creating = true;
				s_Data.StartCreatePos = ImGui::GetMousePos() - s_Data.ImageTopLeft;

				s_Data.CreateSize = ImVec2(0, 0);
			}
			else
			{
				s_Data.CreateSize += ImGui::GetIO().MouseDelta;
			}
		}
		else if(s_Data.Creating)
		{
			s_Data.Creating = false;

			if (s_Data.CreateSize.x != 0 && s_Data.CreateSize.y != 0)
			{
				s_Data.Sheet->GetSprites().push_back({});

				auto& sprite = s_Data.Sheet->GetSprites().back();
				sprite.Name = "Untitled";
				sprite.Position = 
					glm::ivec2(s_Data.CreateSize.x > 0 ? s_Data.StartCreatePos.x : s_Data.StartCreatePos.x + s_Data.CreateSize.x, 
						s_Data.CreateSize.y > 0 ? s_Data.StartCreatePos.y : s_Data.StartCreatePos.y + s_Data.CreateSize.y);
				sprite.Width = glm::abs(s_Data.CreateSize.x);
				sprite.Height = glm::abs(s_Data.CreateSize.y);
			
				auto& textureSpecs = s_Data.Sheet->GetTexture()->GetTextureSpecification();

				sprite.RecalculateUVs(textureSpecs.Width, textureSpecs.Height);
			}
		}

		if (!(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)))
		{
			s_Data.ResizeHandle = 0;
			s_Data.Moving = false;
		}

		if (s_Data.ResizeHandle == 0 && !s_Data.Moving && !s_Data.Creating)
		{
			HandleZoom();
			HandleSelection();
		}

		bool openPopup = false;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Slice"))
			{
				openPopup = true;
				ImGui::OpenPopup("##SliceMenu");
				s_Data.SliceType = 0;
				s_Data.SliceParams[0] = s_Data.SliceParams[1] = 2;
			}
			if (ImGui::MenuItem("Clear all"))
			{
				s_Data.Sheet->GetSprites().clear();
				s_Data.Selected = -1;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			float x = ImGui::GetContentRegionAvail().x;
			x -= (ImGui::CalcTextSize("Save").x + ImGui::GetStyle().ItemSpacing.x * 2.0f);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);

			if (ImGui::MenuItem("Save"))
			{
				s_Data.OrgSheet->GetSprites() = s_Data.Sheet->GetSprites();
				changed = true;
			}

			ImGui::PopStyleVar();

			ImGui::EndMenuBar();
		}

		if (openPopup)
		{
			ImGui::OpenPopup("##SliceMenu");
		}

		if (ImGui::BeginPopup("##SliceMenu"))
		{
			const char* types[] = { "Grid by cell size", "Grid by cell count" };

			ImGui::SetNextItemWidth(200.0f);
			uint32_t newType = Combo("##SliceTypes", types, ARRAYSIZE(types), types[s_Data.SliceType]);
			
			if (newType != -1)
			{
				s_Data.SliceType = newType;
			}
			const char* titles[] = { "Cell size", "Cell count" };
			
			ImGui::Text(titles[s_Data.SliceType]);

			ImGui::SameLine();

			ImGui::PushItemWidth(50);

			uint32_t min = 1.0, max = UINT32_MAX;
			DragScalar("##Width", ImGuiDataType_U32, &s_Data.SliceParams[0], 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_RoundCornersAll);

			ImGui::SameLine();

			DragScalar("##Height", ImGuiDataType_U32, &s_Data.SliceParams[1], 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_RoundCornersAll);

			ImGui::PopItemWidth();

			if (ImGui::Button("Slice"))
			{
				switch (s_Data.SliceType)
				{
				case 0:
				{
					s_Data.Sheet->SplitByCellSize(s_Data.SliceParams[0], s_Data.SliceParams[1]);
				} break;
				case 1:
				{
					s_Data.Sheet->SplitByCellCount(s_Data.SliceParams[0], s_Data.SliceParams[1]);
				} break;
				default:
					KD_CORE_ASSERT(false);
					break;
				}
			}

			ImGui::EndPopup();
		}
	
		ImGui::End();
	
		return changed;
	}

	void SpriteSheetEditor::DrawSpriteProperties()
	{
		const auto& textureSpecs = s_Data.Sheet->GetTexture()->GetTextureSpecification();

		if (s_Data.Selected != -1)
		{
			auto& sprite = s_Data.Sheet->GetSprites()[s_Data.Selected];

			ImGui::Text("Name");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120.0f);

			ImGui::InputText("##Name", &sprite.Name);

			ImGui::Text("Size");
			uint32_t min = 1, max = UINT32_MAX;

			uint32_t width = sprite.Width;

			ImGui::SameLine();
			ImGui::PushItemWidth(50);

			bool changed = DragScalar("##Width", ImGuiDataType_U32, &width, 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_None);

			ImGui::SameLine();

			uint32_t height = sprite.Height;

			changed |= DragScalar("##Height", ImGuiDataType_U32, &height, 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_None);

			ImGui::PopItemWidth();

			if (width < textureSpecs.Width - sprite.Position.x)
			{
				sprite.Width = width;
			}

			if (height < textureSpecs.Height - sprite.Position.y)
			{
				sprite.Height = height;
			}

			ImGui::Text("Position");

			ImGui::SameLine();

			min = 0;
			max = textureSpecs.Width - sprite.Width - 1;

			ImGui::PushItemWidth(50);

			changed |= DragScalar("##PosX", ImGuiDataType_U32, &sprite.Position.x, 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_None);

			ImGui::SameLine();

			min = 0;
			max = textureSpecs.Height - sprite.Height - 1;

			changed |= DragScalar("##PosY", ImGuiDataType_U32, &sprite.Position.y, 1, &min, &max, "%d", ImGuiSliderFlags_None, ImDrawFlags_None);

			ImGui::PopItemWidth();
			if (changed)
			{
				sprite.RecalculateUVs(textureSpecs.Width, textureSpecs.Height);
			}
		}
	}

	void SpriteSheetEditor::HandlePan()
	{
		auto& io = ImGui::GetIO();
		if (s_Data.ImagePanningRightClick)
		{
			glm::vec2 delta = (glm::vec2&)io.MouseDelta;
			if (ShouldClampPan(s_Data.TopLeft.y, s_Data.BottomRight.y, -delta.y))
			{
				delta.y = 0.0f;
			}

			if (ShouldClampPan(s_Data.TopLeft.x, s_Data.BottomRight.x, -delta.x))
			{
				delta.x = 0.0f;
			}

			s_Data.Center -= delta * io.DeltaTime * s_Data.Zoom * 0.5f;
			s_Data.Center = glm::clamp(s_Data.Center, glm::vec2(0.0f), glm::vec2(1.0f));
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}
	}

	void SpriteSheetEditor::HandleZoom()
	{
		auto& io = ImGui::GetIO();
		if (s_Data.ImageHovered && io.MouseWheel != 0.0f)
		{
			s_Data.Zoom = glm::clamp(s_Data.Zoom - io.MouseWheel * io.DeltaTime, 0.2f, 0.5f);

			//Zoom in
			if (io.MouseWheel > 0 && s_Data.Zoom != 0.2f)
			{
				s_Data.Center = MoveTowards(s_Data.Center, s_Data.MouseUVPos, io.DeltaTime * glm::abs(io.MouseWheel));
			}
			//Zoom out to center
			else if (io.MouseWheel < 0 && s_Data.Zoom != 0.5f)
			{
				s_Data.Center = MoveTowards(s_Data.Center, glm::vec2(0.5f), io.DeltaTime * glm::abs(io.MouseWheel));
			}
			else if(io.MouseWheel < 0)
			{
				s_Data.Center = glm::vec2(0.5f);
			}
		}
	}

	void SpriteSheetEditor::HandleSpriteResize()
	{
		auto& io = ImGui::GetIO();
		const auto& textureSpecs = s_Data.Sheet->GetTexture()->GetTextureSpecification();

		glm::ivec2 delta = (glm::vec2&)io.MouseDelta;

		auto& sprite = s_Data.Sheet->GetSprites()[s_Data.Selected];

		glm::ivec2 deltaSize = glm::ivec2(0), deltaPos = glm::ivec2(0);

		glm::uvec2 topLeft = sprite.Position;
		glm::uvec2 bottomRight = topLeft + glm::uvec2(sprite.Width, sprite.Height);

		glm::uvec2 bottomRightDistToEdge = glm::uvec2(textureSpecs.Width, textureSpecs.Height) - bottomRight;

		glm::uvec2 allowedTopLeft = glm::uvec2(delta.x > 0 ? sprite.Width - 1 : topLeft.x,
			delta.y > 0 ? sprite.Height - 1 : topLeft.y);

		glm::uvec2 allowedBottomRight = glm::uvec2(delta.x > 0 ? bottomRightDistToEdge.x : sprite.Width - 1,
			delta.y > 0 ? bottomRightDistToEdge.y : sprite.Height - 1);

		const uint32_t Speed = 1;

		if (s_Data.ResizeHandle & Handle_Left)
		{
			uint32_t speed = glm::abs((int)Speed * delta.x);

			deltaSize.x = -glm::sign(delta.x) * (int)glm::min(speed, allowedTopLeft.x);
			deltaPos.x = -deltaSize.x;
		}
		if (s_Data.ResizeHandle & Handle_Right)
		{
			uint32_t speed = glm::abs((int)Speed * delta.x);

			deltaSize.x = glm::sign(delta.x) * (int)glm::min(speed, allowedBottomRight.x);
		}
		if (s_Data.ResizeHandle & Handle_Top)
		{
			uint32_t speed = glm::abs((int)Speed * delta.y);

			deltaSize.y = -glm::sign(delta.y) * (int)glm::min(speed, allowedTopLeft.y);
			deltaPos.y = -deltaSize.y;
		}
		if (s_Data.ResizeHandle & Handle_Bottom)
		{
			uint32_t speed = glm::abs((int)Speed * delta.y);

			deltaSize.y = glm::sign(delta.y) * (int)glm::min(speed, allowedBottomRight.y);
		}

		sprite.Width += deltaSize.x;
		sprite.Height += deltaSize.y;

		sprite.Position += deltaPos;

		sprite.RecalculateUVs(textureSpecs.Width, textureSpecs.Height);
	}

	void SpriteSheetEditor::HandleMove()
	{
		auto& io = ImGui::GetIO();
		const auto& textureSpecs = s_Data.Sheet->GetTexture()->GetTextureSpecification();

		s_Data.Moving = true;
		auto& sprite = s_Data.Sheet->GetSprites()[s_Data.Selected];

		glm::ivec2 delta = (glm::vec2&)io.MouseDelta;

		const uint32_t Speed = 1;

		glm::uvec2 topLeft = sprite.Position;
		glm::uvec2 bottomRight = topLeft + glm::uvec2(sprite.Width, sprite.Height);

		glm::uvec2 topLeftDist = glm::uvec2(topLeft);
		glm::uvec2 bottomRightDist = glm::uvec2(textureSpecs.Width, textureSpecs.Height) - bottomRight;

		delta *= Speed;

		//dist to right.
		if (delta.x > 0)
		{
			delta.x = glm::min(delta.x, (int)bottomRightDist.x);
		}
		//dist to left
		else
		{
			delta.x = -glm::min(-delta.x, (int)topLeftDist.x);
		}

		//dist to bottom
		if (delta.y > 0)
		{
			delta.y = glm::min(delta.y, (int)bottomRightDist.y);
		}
		//dist to top
		else
		{
			delta.y = -glm::min(-delta.y, (int)topLeftDist.y);
		}

		sprite.Position += delta;

		sprite.RecalculateUVs(textureSpecs.Width, textureSpecs.Height);
	}

	void SpriteSheetEditor::HandleSelection()
	{
		if (s_Data.ImageLeftClicked && s_Data.ResizeHandle == 0)
		{
			s_Data.Selected = -1;

			for (int64_t i = s_Data.Sheet->GetSprites().size() - 1; i >= 0; --i)
			{
				auto& sprite = s_Data.Sheet->GetSprites()[i];

				if (s_Data.MouseUVPos.x >= sprite.UV0.x && s_Data.MouseUVPos.x <= sprite.UV1.x &&
					s_Data.MouseUVPos.y >= sprite.UV1.y && s_Data.MouseUVPos.y <= sprite.UV0.y)
				{
					s_Data.Selected = i;
					break;
				}
			}
		}
	}

	void SpriteSheetEditor::DrawImage()
	{
		auto dl = ImGui::GetWindowDrawList();

		Styler styler;
		styler.PushStyle(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImVec2 avail = ImGui::GetContentRegionAvail();

		auto texture = s_Data.Sheet->GetTexture();
		auto sampler = s_Data.Sheet->GetSampler();

		DescriptorSetLayoutSpecification specs({ { DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader } });

		Ref<DescriptorSet> set = DescriptorSet::Create(specs);
		set->Update(texture, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);

		float width = texture->GetTextureSpecification().Width;

		float height = texture->GetTextureSpecification().Height;

		ImVec2 drawAt;

		float zoom = s_Data.Zoom;

		ImVec2 start = ImGui::GetCursorPos();

		ImVec2 displaySize;

		//if (width < avail.x && height < avail.y)
		{
			drawAt = avail - ImVec2(width, height);
			drawAt *= 0.5f;
			ImGui::SetCursorPos(start + drawAt);

			displaySize = ImVec2(width, height);
		}

		//Calculate bounds for panning and zoom.
		s_Data.TopLeft = s_Data.Center + glm::vec2(-zoom, -zoom);

		s_Data.BottomRight = s_Data.Center + glm::vec2(zoom);

		auto& data = s_Data;

		s_Data.TopLeft = glm::clamp(s_Data.TopLeft, glm::vec2(0.0f), glm::vec2(1.0f));
		s_Data.BottomRight = glm::clamp(s_Data.BottomRight, glm::vec2(0.0f), glm::vec2(1.0f));

		s_Data.ImageTopLeft = ImGui::GetCursorScreenPos();

		ImGui::Image(set->GetSetID(), displaySize, (ImVec2&)s_Data.TopLeft, (ImVec2&)s_Data.BottomRight);
		dl->AddRect(s_Data.ImageTopLeft - ImVec2(1 , 1), s_Data.ImageTopLeft + displaySize, IM_COL32(120, 120, 120, 255));

		s_Data.ImageHovered = ImGui::IsItemHovered();
		s_Data.ImageActive = ImGui::IsItemActive();

		s_Data.ImageLeftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
		s_Data.ImageLeftHeld = (s_Data.ImageHovered || s_Data.ImageActive) && ImGui::IsMouseDown(ImGuiMouseButton_Left);

		s_Data.ImagePanningRightClick = (s_Data.ImageHovered || s_Data.ImageActive) && ImGui::IsMouseDragging(ImGuiMouseButton_Right);
		s_Data.ImagePanningLeftClick = (s_Data.ImageHovered || s_Data.ImageActive) && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

		ImVec2 mousePos = ImGui::GetMousePos() - s_Data.ImageTopLeft;

		int px = (int)floorf(mousePos.x);
		int py = (int)floorf(mousePos.y);

		float tx = (float)px / displaySize.x;
		float ty = (float)py / displaySize.y;

		float x = (1.0f - tx) * s_Data.TopLeft.x + tx * s_Data.BottomRight.x;
		float y = (1.0f - ty) * s_Data.TopLeft.y + ty * s_Data.BottomRight.y;

		s_Data.MouseUVPos = glm::vec2(x, y);

		
		dl->PushClipRect(s_Data.ImageTopLeft, s_Data.ImageTopLeft + displaySize, true);

		glm::vec2 dist = s_Data.BottomRight - s_Data.TopLeft;

		auto& sprites = s_Data.Sheet->GetSprites();
		//Draw Sprites
		for (uint32_t i = 0; i < sprites.size(); ++i)
		{
			Sprite& sprite = sprites[i];

			//To the left of bounds
			if (sprite.UV1.x < s_Data.TopLeft.x)
			{
				continue;
			}

			//To the right of bounds
			if (sprite.UV0.x > s_Data.BottomRight.x)
			{
				continue;
			}

			//To the top of bounds
			if (sprite.UV0.y < s_Data.TopLeft.y)
			{
				continue;
			}

			if (sprite.UV1.y > s_Data.BottomRight.y)
			{
				continue;
			}

			bool selected = s_Data.Selected == i;

			float tX = (sprite.UV0.x - s_Data.TopLeft.x) / dist.x;
			float tY = (sprite.UV1.y - s_Data.TopLeft.y) / dist.y;

			ImVec2 tl = ImVec2(s_Data.ImageTopLeft.x + tX * displaySize.x, s_Data.ImageTopLeft.y + tY * displaySize.y);

			float dx = sprite.UV1.x - sprite.UV0.x;

			float dy = -sprite.UV1.y + sprite.UV0.y;

			float w = dx * displaySize.x / dist.x;
			float h = dy * displaySize.y / dist.y;

			float leftEdge = tl.x;
			float rightEdge = leftEdge + w;
			float topEdge = tl.y;
			float bottomEdge = topEdge + h;

			ImU32 col;

			if (selected)
			{
				col = IM_COL32(80, 200, 255, 255);
			}
			else
			{
				col = IM_COL32(60, 230, 90, 220);
			}

			dl->AddRect(tl, tl + ImVec2(w, h), col, 0.0f, 0, 1.0f);


			if (s_Data.Selected == i)
			{
				s_Data.HandleLeft = ImVec2(leftEdge, (topEdge + bottomEdge) * 0.5f);
				s_Data.HandleRight = ImVec2(rightEdge, (topEdge + bottomEdge) * 0.5f);
				s_Data.HandleTop = ImVec2((leftEdge + rightEdge) * 0.5f, topEdge);
				s_Data.HandleBottom = ImVec2((leftEdge + rightEdge) * 0.5f, bottomEdge);

				dl->AddRectFilled(s_Data.HandleLeft - HandleHalfSize, s_Data.HandleLeft + HandleHalfSize, col);
				dl->AddRectFilled(s_Data.HandleRight - HandleHalfSize, s_Data.HandleRight + HandleHalfSize, col);
				dl->AddRectFilled(s_Data.HandleTop - HandleHalfSize, s_Data.HandleTop + HandleHalfSize, col);
				dl->AddRectFilled(s_Data.HandleBottom - HandleHalfSize, s_Data.HandleBottom + HandleHalfSize, col);
			}
		}

		if (s_Data.Creating && s_Data.CreateSize.x != 0 && s_Data.CreateSize.y != 0)
		{
			dl->AddRect(s_Data.ImageTopLeft + s_Data.StartCreatePos, s_Data.ImageTopLeft + s_Data.StartCreatePos + s_Data.CreateSize, IM_COL32(255, 255, 255, 255));
		}

		dl->PopClipRect();
	}

}
