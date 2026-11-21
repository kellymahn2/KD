#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

#include <glm/glm.hpp>

#include <vector>
#include <optional>

namespace Kaidel {

	struct AtlasRect {
		uint32_t Width, Height;
	};

	struct AtlasSamplingCoords {
		glm::vec2 BottomLeft;
		glm::vec2 TopRight;

		glm::ivec2 BottomLeftPixel;
		glm::ivec2 TopRightPixel;
	};

	// Packing using MaxRects algorithm
	class AtlasManager {
	public:
		AtlasManager(uint32_t width, uint32_t height);

		// i is Index returned by AddToAtlas
		const AtlasSamplingCoords& GetAtlasSamplingCoords(uint32_t i) const;

		uint32_t AddToAtlas(const AtlasRect& rect);

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

	private:
		struct Node {
			glm::ivec2 Position;
			uint32_t Width;
			uint32_t Height;
			bool Used;

			Node(uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool used = false)
				: Position(x, y), Width(w), Height(h), Used(used) {
			}
		};

		struct PlacedRect {
			AtlasRect Rect;
			glm::ivec2 Position;
		};

		void ResizeAtlas(float factor = 1.5f);
		int FindBestFit(const AtlasRect& rect, uint32_t& outX, uint32_t& outY) const;
		void SplitNode(uint32_t nodeID, const AtlasRect& rect, const glm::ivec2& position);
		void MergeFreeNodes();
		void UpdateSamplingCoords(uint32_t index);

		uint32_t m_Width;
		uint32_t m_Height;
		std::vector<Node> m_FreeNodes;
		std::vector<PlacedRect> m_PlacedRects;
		std::vector<AtlasSamplingCoords> m_SamplingCoords;
	};
}
