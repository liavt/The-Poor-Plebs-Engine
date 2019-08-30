/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#pragma once
#ifndef MACE__GRAPHICS_OGL_FREETYPEFONT_H
#define MACE__GRAPHICS_OGL_FREETYPEFONT_H

//The api docs shouldn't include a bunch of internal classes, since any end user wouldn't care about them
#ifndef MACE__DOXYGEN_PASS

#include <MACE/Graphics/Font.h>

//sadly Freetype "types" are actually typedefs for pointers, meaning you can't forward declare them
#undef FT_CONFIG_OPTION_USE_HARFBUZZ
#include <ft2build.h>
#include FT_FREETYPE_H

namespace mc {
	namespace internal {
		/**
		Namespace for Freetype-related classes

		@internal
		*/
		namespace fty {
			//forward declare for FreetypeFont
			class FreetypeLibrary;

			class FreetypeFont: public gfx::FontImpl {
			public:
				FreetypeFont(const gfx::FontDesc& desc, FreetypeLibrary& library);
				~FreetypeFont() override;

				void fillGlyph(gfx::Glyph& glyph, const wchar_t character) const override;

				void calculateMetricsForSize(const gfx::FontSize height) override;

				gfx::FontMetrics getFontMetrics() override;

				Vector<RelativeTranslation, 2> getKerning(const wchar_t prev, const wchar_t current) const override;
			private:
				FT_Face face;
				FT_Library& library;
			};

			/*
			in order to decouple the implementation of Freetype from any GraphicsContextComponent,
			the FreetypeLibrary is the public Freetype interface which FreetypeFont uses.

			this also doubles as allowing the GraphicsContextComponent.h to forward declare this class,
			instead of having to include the entirety of Freetype for FT_Library
			*/
			class FreetypeLibrary: public Initializable {
				friend class FreetypeFont;
			public:
				~FreetypeLibrary() noexcept;

				void init() override;
			private:
				FT_Library freetype{};

				bool isInit() const noexcept override;
			};
		}//fty
	}//internal
}//mc

#endif//MACE__DOXYGEN_PASS

#endif//MACE__GRAPHICS_OGL_FREETYPEFONT_H