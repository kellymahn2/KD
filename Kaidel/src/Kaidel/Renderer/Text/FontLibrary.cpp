#include "KDpch.h"
#include "FontLibrary.h"

#include "Kaidel/Renderer/RendererGlobals.h"

#undef INFINITE
#include "msdf-atlas-gen.h"
#include "MSDFData.h"

namespace Kaidel {


	namespace Utils {
		template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
		static Ref<Texture2D> CreateAndCacheAtlas(const FileSystem::path& cachePath, float fontSize,
			const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& geo, uint32_t width, uint32_t height, Format textureFormat)
		{
			msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);

			msdf_atlas::GeneratorAttributes attributes;
			attributes.config.overlapSupport = true;
			attributes.scanlinePass = true;

			generator.setAttributes(attributes);
			generator.setThreadCount(8);

			generator.generate(glyphs.data(), (int)glyphs.size());

			msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();
			const T* pixels = bitmap.pixels;

			/*const bool flip = false;

			if (flip) {
				uint64_t length = bitmap.width * bitmap.height * sizeof(T) * N;
				uint64_t offset = bitmap.width * sizeof(T) * N;
				T* data = new T[length];

				const T* currSrcPixel = bitmap.pixels + (bitmap.height - 1) * offset;
				T* currDstPixel = data;

				while (currSrcPixel >= bitmap.pixels) {
					std::memcpy(currDstPixel, currSrcPixel, bitmap.width * sizeof(T) * N);
					currSrcPixel -= offset;
					currDstPixel += offset;
				}
				pixels = data;
			}*/

			Texture2DSpecification specs;
			specs.Width = bitmap.width;
			specs.Height = bitmap.height;
			specs.Depth = 1;
			specs.Layers = 1;
			specs.Mipped = false;
			specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
			specs.Samples = TextureSamples::x1;
			specs.Format = textureFormat;
			TextureData data;
			data.Data = pixels;
			specs.InitialDatas.push_back(data);

			Ref<Texture2D> texture = Texture2D::Create(specs);

			if (!cachePath.empty())
			{
				std::ofstream file(cachePath, std::ios::binary | std::ios::out);

				KD_ASSERT(file.is_open());

				file.write((const char*)&bitmap.width, sizeof(uint32_t));
				file.write((const char*)&bitmap.height, sizeof(uint32_t));

				uint64_t size = (uint64_t)bitmap.width * bitmap.height * N * sizeof(T);
				file.write((const char*)pixels, size);
			}

			/*if (flip) {
				delete[] pixels;
			}*/

			return texture;
		}
	}

	void FontLibrary::Init()
	{

	}


	void FontLibrary::Shutdown()
	{

	}

	void FontLibrary::LoadInto(Ref<Font> font, const Path& path)
	{
		font->m_Data = new MSDFData();

		

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();

		KD_CORE_ASSERT(ft);

		std::string fileString = path.string();

		msdfgen::FontHandle* f = msdfgen::loadFont(ft, fileString.c_str());

		if (!f) {
			KD_ERROR("Failed to load font: {}", fileString);
			return;
		}

		struct CharsetRange {

			uint32_t Begin, End;

		};

		//From imgui_draw.cpp
		static const CharsetRange charsetRanges[] = {
			{0x0020, 0x00FF}
		};

		msdf_atlas::Charset charset;

		for (const CharsetRange& range : charsetRanges) {
			for (uint32_t c = range.Begin; c <= range.End; ++c) {
				charset.add(c);
			}
		}

		double fontScale = 1.0;
		font->m_Data->FontGeometry = msdf_atlas::FontGeometry(&font->m_Data->Glyphs);
		uint64_t loaded = font->m_Data->FontGeometry.loadCharset(f, fontScale, charset);
		KD_CORE_INFO("Loaded {} glyphs from font {} (out of {})", loaded, fileString, charset.size());


		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;

		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);

		int remaining = atlasPacker.pack(font->m_Data->Glyphs.data(), (int)font->m_Data->Glyphs.size());

		KD_CORE_ASSERT(remaining == 0);

		if (FileSystem::exists(path.string() + ".cache"))
		{
			std::ifstream file(path.string() + ".cache", std::ios::binary | std::ios::out);

			uint32_t width, height;

			file.read((char*)&width, sizeof(width));
			file.read((char*)&height, sizeof(height));

			std::vector<uint8_t> buffer(width * height * 4, 0);

			file.read((char*)buffer.data(), width * height * 4);

			Texture2DSpecification specs;
			specs.Width = width;
			specs.Height = height;
			specs.Depth = 1;
			specs.Layers = 1;
			specs.Mipped = false;
			specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
			specs.Samples = TextureSamples::x1;
			specs.Format = Format::RGBA8UN;
			TextureData data;
			data.Data = buffer.data();
			specs.InitialDatas.push_back(data);

			Ref<Texture2D> texture = Texture2D::Create(specs);

			font->m_AtlasTexture = texture;

			return;
		}

		int width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

		//if MSDF || MTSDF

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

		bool expensiveColoring = false;



		uint64_t coloringSeed = 0;

		{
			SCOPED_TIMER("Font loading");
			if (expensiveColoring) {
				msdf_atlas::Workload([&glyphs = font->m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
					unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
					glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
					return true;
					}, (int)font->m_Data->Glyphs.size()).finish(THREAD_COUNT);
			}
			else {
				unsigned long long glyphSeed = coloringSeed;
				for (msdf_atlas::GlyphGeometry& glyph : font->m_Data->Glyphs) {
					glyphSeed *= LCG_MULTIPLIER;
					glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				}
			}
		}

		font->m_AtlasTexture = 
			Utils::CreateAndCacheAtlas<uint8_t, float, 4, msdf_atlas::mtsdfGenerator>(
				path.string() + ".cache",
				(float)emSize, font->m_Data->Glyphs, 
				font->m_Data->FontGeometry, width, height, 
				Format::RGBA8UN);

		msdfgen::destroyFont(f);
		msdfgen::deinitializeFreetype(ft);
	}


	Kaidel::Ref<Kaidel::Font> FontLibrary::Load(const Path& path)
	{
		Ref<Font> font = CreateRef<Font>();

		LoadInto(font, path);

		return font;
	}

	Kaidel::Ref<Kaidel::Font> FontLibrary::Get(const Path& path)
	{
		return {};
	}


	const Kaidel::Path& FontLibrary::GetPath(Ref<Font> texture)
	{
		return {};
	}


	bool FontLibrary::Exists(const Path& path)
	{
		return false;
	}


}
