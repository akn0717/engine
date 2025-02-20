﻿#include "asset_font.hxx"

#include <ice/uri.hxx>
#include <ice/resource.hxx>
#include <ice/resource_meta.hxx>
#include <ice/resource_tracker.hxx>
#include <ice/task_sync_wait.hxx>
#include <ice/font.hxx>

#include <msdf-atlas-gen/msdf-atlas-gen.h>


namespace ice
{

    auto create_engine_object(
        ice::Allocator& alloc,
        std::vector<msdf_atlas::GlyphGeometry> const& glyphs,
        msdfgen::BitmapConstRef<msdfgen::byte, 4> const& bitmap
    ) noexcept -> ice::Memory
    {
        using ice::Font;
        using ice::FontAtlas;
        using ice::GlyphRange;
        using ice::Glyph;

        static_assert(alignof(Font) == 8);
        static_assert(alignof(FontAtlas) == 4);
        static_assert(alignof(GlyphRange) == 4);

        ice::u32 size = sizeof(Font) + sizeof(FontAtlas) + sizeof(GlyphRange) + 8 /* for alignment */;
        size += sizeof(Glyph) * glyphs.size();
        size += bitmap.width * bitmap.height * 4;

        void* data = alloc.allocate(size, 8);

        Font* gfx_font = reinterpret_cast<Font*>(data);
        FontAtlas* gfx_atlas = reinterpret_cast<FontAtlas*>(
            ice::memory::ptr_align_forward(gfx_font + 1, alignof(FontAtlas))
        ); // TODO: Multi-atlas support
        GlyphRange* gfx_glyph_range = reinterpret_cast<GlyphRange*>(
            ice::memory::ptr_align_forward(gfx_atlas + 1, alignof(GlyphRange))
        ); // TODO: Multi-atlas support
        Glyph* font_glyphs = reinterpret_cast<Glyph*>(gfx_glyph_range + 1); // TODO: Multi range support
        void* atlas_bitmap = font_glyphs + glyphs.size();

        void const* end_ptr = ice::memory::ptr_add(atlas_bitmap, bitmap.width * bitmap.height * 4);
        ICE_ASSERT(end_ptr <= ice::memory::ptr_add(data, size), "Insufficient memory!");

        gfx_font->atlases = { gfx_atlas, 1 };
        gfx_font->ranges = { gfx_glyph_range, 1 };
        gfx_font->glyphs = { font_glyphs, glyphs.size() };

        gfx_glyph_range->type = ice::GlyphRangeType::Explicit;
        gfx_glyph_range->glyph_count = glyphs.size();
        gfx_glyph_range->glyph_index = 0;
        gfx_glyph_range->glyph_atlas = 0;

        gfx_atlas->image_size = ice::vec2u(bitmap.width, bitmap.height);
        gfx_atlas->image_data_offset = ice::memory::ptr_distance(data, atlas_bitmap);
        gfx_atlas->image_data_size = bitmap.width * bitmap.height * 4;

        ice::u32* offset = reinterpret_cast<ice::u32*>(std::addressof(gfx_font->glyphs));
        offset[0] = ice::memory::ptr_distance(data, font_glyphs);
        offset[1] = static_cast<ice::u32>(glyphs.size());

        offset = reinterpret_cast<ice::u32*>(std::addressof(gfx_font->ranges));
        offset[0] = ice::memory::ptr_distance(data, gfx_glyph_range);
        offset[1] = 1;

        offset = reinterpret_cast<ice::u32*>(std::addressof(gfx_font->atlases));
        offset[0] = ice::memory::ptr_distance(data, gfx_atlas);
        offset[1] = 1;

        ice::f32 const atlas_width = static_cast<ice::f32>(bitmap.width);
        ice::f32 const atlas_height = static_cast<ice::f32>(bitmap.height);

        ice::u32 glyph_idx = 0;
        for (msdf_atlas::GlyphGeometry const& glyph_geometry : glyphs)
        {
            ice::i32 x, y, w, h;
            glyph_geometry.getBoxRect(x, y, w, h);

            ice::vec<4,ice::f64> l;
            glyph_geometry.getQuadPlaneBounds(l.x, l.y, l.z, l.w);

            Glyph& gfx_glyph = font_glyphs[glyph_idx];
            gfx_glyph.atlas_x = static_cast<ice::f32>(x) / atlas_width;
            gfx_glyph.atlas_y = static_cast<ice::f32>(y) / atlas_height;
            gfx_glyph.atlas_w = static_cast<ice::f32>(w) / atlas_width;
            gfx_glyph.atlas_h = static_cast<ice::f32>(h) / atlas_height;
            gfx_glyph.advance = static_cast<ice::f32>(glyph_geometry.getAdvance());
            gfx_glyph.offset.x = static_cast<ice::f32>(l.x);
            gfx_glyph.offset.y = static_cast<ice::f32>(l.y);
            gfx_glyph.size.x = static_cast<ice::f32>(l.z - l.x);
            gfx_glyph.size.y = static_cast<ice::f32>(l.y - l.w);
            gfx_glyph.codepoint = static_cast<ice::u32>(glyph_geometry.getCodepoint());

            glyph_idx += 1;
        }

        ice::memcpy(atlas_bitmap, bitmap.pixels, gfx_atlas->image_data_size);
        return { .location = data, .size = size, .alignment = 8 };
    }

    auto asset_font_oven(
        void*,
        ice::Allocator& alloc,
        ice::ResourceTracker const& tracker,
        ice::Resource_v2 const& resource,
        ice::Data data,
        ice::Memory& memory
    ) noexcept -> ice::Task<bool>
    {
        bool result = true;

        msdfgen::FreetypeHandle* const freetype = msdfgen::initializeFreetype();
        if (freetype != nullptr)
        {
            msdfgen::FontHandle* const font = msdfgen::loadFontData(freetype, static_cast<msdfgen::byte const*>(data.location), data.size);
            if (font != nullptr)
            {
                std::vector<msdf_atlas::GlyphGeometry> glyphs;
                msdf_atlas::FontGeometry geometry{ &glyphs };

                msdf_atlas::Charset charset = msdf_atlas::Charset::ASCII;
                // TODO: Properly support bigger charsets.
                charset.add((uint32_t)U"日"[0]);
                charset.add((uint32_t)U"本"[0]);
                charset.add((uint32_t)U"わ"[0]);
                charset.add((uint32_t)U"た"[0]);
                charset.add((uint32_t)U"し"[0]);
                geometry.loadCharset(font, 2.0f, charset);

                [[maybe_unused]]
                msdfgen::FontMetrics const& metrics = geometry.getMetrics();

                // Apply MSDF edge coloring
                ice::f64 constexpr maxCornerAngle = 3.0;
                for (msdf_atlas::GlyphGeometry& glyph_geometry : glyphs)
                {
                    glyph_geometry.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
                }

                msdf_atlas::TightAtlasPacker atlas_packer;
                atlas_packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);
                atlas_packer.setMinimumScale(32.0);
                atlas_packer.setPixelRange(2.0);
                atlas_packer.setMiterLimit(1.0);
                atlas_packer.pack(glyphs.data(), glyphs.size());

                ice::i32 width, height;
                atlas_packer.getDimensions(width, height);

                ice::i32 constexpr atlas_channels = 4;

                using BitmapConstRef = msdfgen::BitmapConstRef<msdfgen::byte, atlas_channels>;
                using BitmapStorage = msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, atlas_channels>;
                using BitmapAtlasGenerator = msdf_atlas::ImmediateAtlasGenerator<
                    float,
                    atlas_channels,
                    msdf_atlas::mtsdfGenerator,
                    BitmapStorage
                >;

                BitmapAtlasGenerator bitmapGenerator{ width, height };

                msdf_atlas::GeneratorAttributes attribs;
                bitmapGenerator.setAttributes(attribs);
                bitmapGenerator.setThreadCount(2);
                bitmapGenerator.generate(glyphs.data(), glyphs.size());

                memory = ice::create_engine_object(
                    alloc,
                    glyphs,
                    bitmapGenerator.atlasStorage()
                );

                BitmapConstRef bitmap_ref = bitmapGenerator.atlasStorage();
                msdfgen::savePng(bitmap_ref, "test.png");
                msdfgen::destroyFont(font);
            }
            msdfgen::deinitializeFreetype(freetype);
        }
        co_return result;
    }

    auto asset_font_loader(
        void*,
        ice::Allocator& alloc,
        ice::AssetStorage& storage,
        ice::Metadata const& meta,
        ice::Data data,
        ice::Memory& out_data
    ) noexcept -> ice::Task<bool>
    {
        out_data.size = sizeof(ice::Font);
        out_data.alignment = alignof(ice::Font);
        out_data.location = alloc.allocate(out_data.size, out_data.alignment);

        ice::Font* font = reinterpret_cast<ice::Font*>(out_data.location);
        ice::Font const* raw_font = reinterpret_cast<ice::Font const*>(data.location);
        font->data_ptr = raw_font;

        ice::u32 const* offsets = reinterpret_cast<ice::u32 const*>(ice::addressof(raw_font->atlases));
        font->atlases = { reinterpret_cast<ice::FontAtlas const*>(ice::memory::ptr_add(raw_font, offsets[0])), offsets[1] };
        offsets = reinterpret_cast<ice::u32 const*>(ice::addressof(raw_font->ranges));
        font->ranges = { reinterpret_cast<ice::GlyphRange const*>(ice::memory::ptr_add(raw_font, offsets[0])), offsets[1] };
        offsets = reinterpret_cast<ice::u32 const*>(ice::addressof(raw_font->glyphs));
        font->glyphs = { reinterpret_cast<ice::Glyph const*>(ice::memory::ptr_add(raw_font, offsets[0])), offsets[1] };
        co_return true;
    }

    void asset_type_font_definition(ice::AssetTypeArchive& asset_type_archive) noexcept
    {
        static ice::Utf8String extensions[]{ u8".ttf" };

        static ice::AssetTypeDefinition type_definition{
            .resource_extensions = extensions,
            .fn_asset_oven = asset_font_oven,
            .fn_asset_loader = asset_font_loader
        };

        asset_type_archive.register_type(ice::AssetType_Font, type_definition);
    }

} // namespace ice
