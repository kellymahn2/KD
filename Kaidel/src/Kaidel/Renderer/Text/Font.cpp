#include "KDpch.h"
#include "Font.h"
#include "Kaidel/Renderer/RendererGlobals.h"


#undef INFINITE
#include "msdf-atlas-gen.h"
#include "MSDFData.h"

namespace Kaidel {

	
	template<typename T,typename S,int N,msdf_atlas::GeneratorFunction<S,N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize,
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

		const bool flip = false;

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
		}
		Texture2DSpecification specs;
		specs.Width = bitmap.width;
		specs.Height = bitmap.height;
		specs.Depth = 1;
		specs.Layers = 1;
		specs.Mips = 1;
		specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
		specs.Samples = TextureSamples::x1;
		specs.Format = textureFormat;
		TextureData data;
		data.Data = pixels;
		specs.InitialDatas.push_back(data);

		Ref<Texture2D> texture = Texture2D::Create(specs);
		
		if (flip) {
			delete[] pixels;
		}
		
		return texture;
	}

	Font::Font(const Path& path)
		:m_Data(new MSDFData())
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		
		KD_CORE_ASSERT(ft);

		std::string fileString = path.string();

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());

		if (!font) {
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
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		uint64_t loaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		KD_CORE_INFO("Loaded {} glyphs from font {} (out of {})", loaded,fileString,charset.size());


		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;

		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);

		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());

		KD_CORE_ASSERT(remaining == 0);

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

		if (expensiveColoring) {
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs) {
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 4,msdf_atlas::mtsdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height,Format::RGBA8UN);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		CreateDescriptor();
	}
	Font::~Font() {
		delete m_Data;
	}
	void Font::CreateDescriptor()
	{
		DescriptorSetLayoutSpecification specs(
			{
				{DescriptorType::Texture,ShaderStage_FragmentShader},
				{DescriptorType::Sampler,ShaderStage_FragmentShader}
			}
		);

		m_Set = DescriptorSet::Create(specs);

		m_Set->Update(m_AtlasTexture, {}, ImageLayout::ShaderReadOnlyOptimal, 0, 0);
		m_Set->Update({}, RendererGlobals::GetSamler(SamplerFilter::Linear,SamplerMipMapMode::Nearest), {}, 1, 0);
	}
}

