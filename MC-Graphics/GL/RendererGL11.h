#pragma once

#include <MC-Graphics/GL/RendererGL.h>

namespace mc {
	class RendererGL11 : public RendererGL{
	public:
		RendererGL11();
		void render(GraphicsObject* object);
	};
}