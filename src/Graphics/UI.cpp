/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#include <MACE/Graphics/UI.h>
#include <MACE/Graphics/RenderTarget.h>
#include <MACE/Core/System.h>

namespace mc {
	namespace gfx {
		namespace {
			MACE_CONSTEXPR const unsigned char DEFAULT_UI_CORNER[] = {
				0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
				0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0a,
				0x08, 0x06, 0x00, 0x00, 0x00, 0x8d, 0x32, 0xcf, 0xbd, 0x00, 0x00, 0x00,
				0x04, 0x67, 0x41, 0x4d, 0x41, 0x00, 0x00, 0xb1, 0x8f, 0x0b, 0xfc, 0x61,
				0x05, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0e,
				0xc2, 0x00, 0x00, 0x0e, 0xc2, 0x01, 0x15, 0x28, 0x4a, 0x80, 0x00, 0x00,
				0x00, 0x18, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
				0x72, 0x65, 0x00, 0x70, 0x61, 0x69, 0x6e, 0x74, 0x2e, 0x6e, 0x65, 0x74,
				0x20, 0x34, 0x2e, 0x31, 0x2e, 0x36, 0xfd, 0x4e, 0x09, 0xe8, 0x00, 0x00,
				0x00, 0x55, 0x49, 0x44, 0x41, 0x54, 0x28, 0x53, 0x8d, 0xd0, 0xb1, 0x09,
				0x00, 0x31, 0x0c, 0x03, 0x40, 0x17, 0x5f, 0x64, 0xc0, 0x2f, 0xb3, 0x55,
				0xc6, 0xc8, 0x50, 0x29, 0x7e, 0x10, 0x45, 0x7a, 0x08, 0x04, 0xe1, 0xc2,
				0x86, 0x73, 0x23, 0x15, 0xc6, 0xc1, 0x01, 0x2d, 0x9a, 0xd4, 0xa9, 0x01,
				0x08, 0xa7, 0xd1, 0xbe, 0x7d, 0xf4, 0x56, 0x8a, 0xc7, 0xa0, 0xa7, 0x52,
				0x94, 0x51, 0x2d, 0xca, 0x7f, 0x86, 0xc6, 0x03, 0xa7, 0x9b, 0x1b, 0xa5,
				0xa1, 0xd3, 0x37, 0xd2, 0xc0, 0xe9, 0x75, 0x69, 0xe0, 0xf4, 0xe7, 0x34,
				0x30, 0x81, 0x0d, 0xbc, 0x5a, 0x78, 0x04, 0x5c, 0xfe, 0xcd, 0xe3, 0x00,
				0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
			};
		}//anonymous namespace

		void UIButton::onInit() {
			auto context = getRoot()->getComponent<GraphicsContextComponent>();

			NineSliceDesc desc{};
			desc.center = context->createTextureFromColor(Colors::BLACK);
			desc.top = desc.center;
			desc.left = desc.center;
			desc.right = desc.center;
			desc.bottom = desc.center;

			desc.topLeft = context->createTextureFromMemory(DEFAULT_UI_CORNER, os::getArraySize(DEFAULT_UI_CORNER));
			desc.topRight = desc.topLeft;
			desc.bottomLeft = desc.topLeft;
			desc.bottomRight = desc.topLeft;

			desc.flipSides = true;
			nineSlice.setDesc(desc);
			addComponent(nineSlice);
		}

		void UIButton::onHover() {
			auto context = getRoot()->getComponent<GraphicsContextComponent>();

			NineSliceDesc& desc = nineSlice.getDesc();
			desc.center = context->createTextureFromColor(Colors::BLUE);
			desc.top = desc.center;
			desc.left = desc.center;
			desc.right = desc.center;
			desc.bottom = desc.center;

			desc.topLeft = Texture(desc.topLeft, Colors::BLUE);
			desc.topRight = desc.topLeft;
			desc.bottomLeft = desc.topLeft;
			desc.bottomRight = desc.topLeft;
		}

		void UIButton::onRender(Painter&) {
			//do nothing, but has to override this function for inheritence reasons

			//the nine slice component does all the rendering necessary
		}
	}//gfx
}//mc