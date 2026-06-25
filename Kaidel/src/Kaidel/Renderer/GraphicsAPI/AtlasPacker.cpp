#include "KDpch.h"
#include "AtlasPacker.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cassert>

namespace Kaidel {

	static const constexpr uint32_t padding = 1;

	AtlasManager::AtlasManager(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		m_FreeNodes.emplace_back(0, 0, width, height, false);
	}

	const AtlasSamplingCoords& AtlasManager::GetAtlasSamplingCoords(uint32_t i) const
	{
		KD_CORE_ASSERT(i < m_SamplingCoords.size(), "Invalid atlas index!");
		return m_SamplingCoords[i];
	}

	uint32_t AtlasManager::AddToAtlas(const AtlasRect& rect)
	{
		uint32_t x, y;
		int bestFitIndex = FindBestFit(rect, x, y);

		if (bestFitIndex == -1)
		{
			ResizeAtlas(1.5f);
			bestFitIndex = FindBestFit(rect, x, y);
			KD_CORE_ASSERT(bestFitIndex != -1, "Failed to place rect even after resize!");
		}

		SplitNode(bestFitIndex, rect, glm::ivec2(x, y));
		m_FreeNodes.erase(m_FreeNodes.begin() + bestFitIndex);
		MergeFreeNodes();

		PlacedRect placed;
		placed.Rect = rect;
		placed.Position = glm::ivec2(x, y);
		m_PlacedRects.push_back(placed);

		uint32_t newIndex = static_cast<uint32_t>(m_SamplingCoords.size());
		m_SamplingCoords.emplace_back();
		UpdateSamplingCoords(newIndex);

		return newIndex;
	}

	uint32_t AtlasManager::GetWidth() const
	{
		return m_Width;
	}

	uint32_t AtlasManager::GetHeight() const
	{
		return m_Height;
	}

	void AtlasManager::ResizeAtlas(float factor)
	{
		uint32_t oldWidth = m_Width;
		uint32_t oldHeight = m_Height;

		m_Width = static_cast<uint32_t>(static_cast<float>(m_Width) * factor);
		m_Height = static_cast<uint32_t>(static_cast<float>(m_Height) * factor);

		// Add the new expanded area as free space
		// The new area is to the right and bottom of the original atlas

		// Add the right strip
		if (m_Width > oldWidth)
		{
			m_FreeNodes.emplace_back(oldWidth, 0, m_Width - oldWidth, oldHeight, false);
		}

		// Add the bottom strip (including the corner that might overlap with right strip)
		if (m_Height > oldHeight)
		{
			m_FreeNodes.emplace_back(0, oldHeight, m_Width, m_Height - oldHeight, false);
		}

		// Merge any adjacent free nodes (including the newly added ones)
		MergeFreeNodes();

		// Update sampling coordinates for all existing rectangles
		// since the atlas dimensions have changed
		for (uint32_t i = 0; i < m_SamplingCoords.size(); i++)
		{
			UpdateSamplingCoords(i);
		}
	}

	int AtlasManager::FindBestFit(const AtlasRect& rect, uint32_t& outX, uint32_t& outY) const
	{

		int bestIndex = -1;
		uint32_t bestArea = std::numeric_limits<uint32_t>::max();
		uint32_t bestWidthFit = std::numeric_limits<uint32_t>::max();

		uint32_t w = rect.Width + padding, h = rect.Height + padding;

		for (size_t i = 0; i < m_FreeNodes.size(); i++)
		{
			const Node& node = m_FreeNodes[i];

			// Check if rectangle fits in this node
			if (rect.Width <= node.Width && rect.Height <= node.Height)
			{
				uint32_t leftoverArea = (node.Width - rect.Width) * (node.Height - rect.Height);
				uint32_t widthFit = node.Width - rect.Width;

				if (leftoverArea < bestArea || (leftoverArea == bestArea && widthFit < bestWidthFit))
				{
					bestArea = leftoverArea;
					bestWidthFit = widthFit;
					bestIndex = static_cast<int>(i);
					outX = node.Position.x;
					outY = node.Position.y;
				}
			}

			//// Try rotated placement if rectangle is not square
			//if (rect.Width != rect.Height && rect.Height <= node.Width && rect.Width <= node.Height)
			//{
			//	uint32_t leftoverArea = (node.Width - rect.Height) * (node.Height - rect.Width);
			//	uint32_t widthFit = node.Width - rect.Height;
			//
			//	if (leftoverArea < bestArea || (leftoverArea == bestArea && widthFit < bestWidthFit))
			//	{
			//		bestArea = leftoverArea;
			//		bestWidthFit = widthFit;
			//		bestIndex = static_cast<int>(i);
			//		outX = node.Position.x;
			//		outY = node.Position.y;
			//	}
			//}
		}

		return bestIndex;
	}

	void AtlasManager::SplitNode(uint32_t nodeID, const AtlasRect& rect, const glm::ivec2& position)
	{
		auto* node = m_FreeNodes.data() + nodeID;

		uint32_t w = rect.Width + padding, h = rect.Height + padding;

		// Create the new free nodes from the remaining space
		// Right node
		if (position.x + w < node->Position.x + node->Width)
		{
			uint32_t rightWidth = (node->Position.x + node->Width) - (position.x + w);
			uint32_t rightHeight = h;
			m_FreeNodes.emplace_back(position.x + w, position.y, rightWidth, rightHeight, false);
			node = m_FreeNodes.data() + nodeID;
		}

		// Top node
		if (position.y + h < node->Position.y + node->Height)
		{
			uint32_t topWidth = w;
			uint32_t topHeight = (node->Position.y + node->Height) - (position.y + h);
			m_FreeNodes.emplace_back(position.x, position.y + h, topWidth, topHeight, false);
			node = m_FreeNodes.data() + nodeID;
		}

		// If there's space both right and top, also add the corner
		if (position.x + w < node->Position.x + node->Width &&
			position.y + h < node->Position.y + node->Height)
		{
			uint32_t cornerWidth = (node->Position.x + node->Width) - (position.x + w);
			uint32_t cornerHeight = (node->Position.y + node->Height) - (position.y + h);
			m_FreeNodes.emplace_back(position.x + w, position.y + h, cornerWidth, cornerHeight, false);
			node = m_FreeNodes.data() + nodeID;
		}
	}

	void AtlasManager::MergeFreeNodes()
	{
		bool merged = true;
		while (merged)
		{
			merged = false;
			for (size_t i = 0; i < m_FreeNodes.size(); i++)
			{
				for (size_t j = i + 1; j < m_FreeNodes.size(); j++)
				{
					Node& a = m_FreeNodes[i];
					Node& b = m_FreeNodes[j];

					// Check if nodes can be merged horizontally
					if (a.Position.y == b.Position.y && a.Height == b.Height &&
						a.Position.x + a.Width == b.Position.x)
					{
						a.Width += b.Width;
						m_FreeNodes.erase(m_FreeNodes.begin() + j);
						merged = true;
						break;
					}
					// Check if nodes can be merged vertically
					else if (a.Position.x == b.Position.x && a.Width == b.Width &&
						a.Position.y + a.Height == b.Position.y)
					{
						a.Height += b.Height;
						m_FreeNodes.erase(m_FreeNodes.begin() + j);
						merged = true;
						break;
					}
				}
				if (merged) break;
			}
		}

		// Sort free nodes by area (largest first) for better packing efficiency
		std::sort(m_FreeNodes.begin(), m_FreeNodes.end(),
			[](const Node& a, const Node& b) {
				return (a.Width * a.Height) > (b.Width * b.Height);
			});
	}

	void AtlasManager::UpdateSamplingCoords(uint32_t index)
	{
		const PlacedRect& placed = m_PlacedRects[index];

		AtlasSamplingCoords& coords = m_SamplingCoords[index];

		// Pixel coordinates
		coords.BottomLeftPixel = placed.Position;
		coords.TopRightPixel = placed.Position + glm::ivec2(placed.Rect.Width, placed.Rect.Height);

		// Normalized UV coordinates (0-1 range)
		coords.BottomLeft = glm::vec2(
			static_cast<float>(placed.Position.x) / static_cast<float>(m_Width),
			static_cast<float>(placed.Position.y) / static_cast<float>(m_Height)
		);

		coords.TopRight = glm::vec2(
			static_cast<float>(placed.Position.x + placed.Rect.Width) / static_cast<float>(m_Width),
			static_cast<float>(placed.Position.y + placed.Rect.Height) / static_cast<float>(m_Height)
		);
	}

} // namespace Kaidel
