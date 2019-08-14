/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/

//std::copy raises a dumb warning on this version of msvc which does not contribute anything at all
//the reason why this check does not use MACE_MSVC is because that would require including Constants.h which is what raises the warning
#if defined(_MSC_VER) && _MSC_VER >= 1400 
#	pragma warning( push ) 
#	pragma warning( disable: 4996 ) 
#endif 

#include <MACE/Graphics/OGL/OGLRenderer.h>
#include <MACE/Graphics/Context.h>
#include <MACE/Core/System.h>

#include <GLFW/glfw3.h>

//we need to include algorithim for std::copy
#include <algorithm>
//cstring is for strcmp
#include <cstring>
//std::begin and std::end
#include <iterator>

//output error messages to console
#include <sstream>

#ifdef MACE_DEBUG
#	include <iostream>
#endif

namespace mc {
	namespace internal {
		namespace ogl {
			//how many floats in the uniform buffer
#define MACE__ENTITY_DATA_BUFFER_SIZE sizeof(float) * 28
			//which binding location the uniform buffer goes to
#define MACE__ENTITY_DATA_LOCATION 0
#define MACE__ENTITY_DATA_STORAGE_FLAGS GL_DYNAMIC_STORAGE_BIT
			//the definition is later stringified. cant be a string because this gets added to the shader via a macro (see createShader)
#define MACE__ENTITY_DATA_NAME _mc_EntityData

#define MACE__PAINTER_DATA_BUFFER_SIZE sizeof(float) * 44
#define MACE__PAINTER_DATA_LOCATION 1
#define MACE__PAINTER_DATA_STORAGE_FLAGS GL_DYNAMIC_STORAGE_BIT
#define MACE__PAINTER_DATA_NAME _mc_PainterData

#define MACE__SCENE_ATTACHMENT_INDEX 0
#define MACE__ID_ATTACHMENT_INDEX 1
#define MACE__DATA_ATTACHMENT_INDEX 2

#define MACE__HAS_RENDER_FEATURE(features, feature) (features & gfx::Painter::RenderFeatures::feature) != gfx::Painter::RenderFeatures::NONE

			namespace {
				Shader createShader(const Enum type, const gfx::Painter::RenderFeatures features, const char* source) {
					Shader s = Shader(type);
					s.init();
#define MACE__SHADER_MACRO(name, def) "#define " #name " " MACE_STRINGIFY_DEFINITION(def) "\n"
					std::vector<const char*> sources = std::vector<const char*>({
						MACE__SHADER_MACRO(MACE_ENTITY_DATA_LOCATION, MACE__ENTITY_DATA_LOCATION),
						MACE__SHADER_MACRO(MACE_ENTITY_DATA_NAME, MACE__ENTITY_DATA_NAME),
						MACE__SHADER_MACRO(MACE_PAINTER_DATA_LOCATION, MACE__PAINTER_DATA_LOCATION),
						MACE__SHADER_MACRO(MACE_PAINTER_DATA_NAME, MACE__PAINTER_DATA_NAME),
						MACE__SHADER_MACRO(MACE_SCENE_ATTACHMENT_INDEX, MACE__SCENE_ATTACHMENT_INDEX),
						MACE__SHADER_MACRO(MACE_ID_ATTACHMENT_INDEX, MACE__ID_ATTACHMENT_INDEX),
						MACE__SHADER_MACRO(MACE_DATA_ATTACHMENT_INDEX, MACE__DATA_ATTACHMENT_INDEX),
						MACE__SHADER_MACRO(MACE_VAO_DEFAULT_VERTICES_LOCATION, MACE__VAO_DEFAULT_VERTICES_LOCATION),
						MACE__SHADER_MACRO(MACE_VAO_DEFAULT_TEXTURE_COORD_LOCATION, MACE__VAO_DEFAULT_TEXTURE_COORD_LOCATION),
#include <MACE/Graphics/OGL/Shaders/Shared.glsl>
																				});
#undef MACE__SHADER_MACRO

#define MACE__SHADER_RENDER_FEATURE(name) if(MACE__HAS_RENDER_FEATURE(features, name)){ sources.insert(sources.begin(), "#define MACE_" MACE_STRINGIFY(name) " 1\n"); }
					MACE__SHADER_RENDER_FEATURE(DISCARD_INVISIBLE);
					MACE__SHADER_RENDER_FEATURE(FILTER);
					MACE__SHADER_RENDER_FEATURE(TEXTURE);
					MACE__SHADER_RENDER_FEATURE(TEXTURE_TRANSFORM);
					MACE__SHADER_RENDER_FEATURE(STORE_ID);
#undef MACE__SHADER_RENDER_FEATURE

					sources.insert(sources.begin(), "#version 330 core\n");

					if (type == GL_VERTEX_SHADER) {
						sources.push_back(
#include <MACE/Graphics/OGL/Shaders/Vert.glsl>
						);
					} else if (type == GL_FRAGMENT_SHADER) {
						sources.push_back(
#include <MACE/Graphics/OGL/Shaders/Frag.glsl>
						);
					}
#ifdef MACE_DEBUG
					else MACE_UNLIKELY{
						MACE__THROW(gfx::BadFormat, "Internal Error: Unknown GLSL shader type: " + std::to_string(type));
					}
#endif
					sources.push_back(source);
					s.setSource(static_cast<const GLsizei>(sources.size()), sources.data(), nullptr);
					s.compile();
					return s;
				}

				void initializeRenderProtocolForSettings(Renderer::RenderProtocol& prot, const std::pair<gfx::Painter::Brush, gfx::Painter::RenderFeatures>& settings) {
					ogl::ShaderProgram program;

					program.init();

					ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error initializing ShaderProgram");

					program.attachShader(createShader(GL_VERTEX_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/RenderTypes/standard.v.glsl>
					));

					if (settings.first == gfx::Painter::Brush::COLOR) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/color.f.glsl>
						));

						program.link();
					} else if (settings.first == gfx::Painter::Brush::TEXTURE) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/texture.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex");

						program.setUniform("tex", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
					} else if (settings.first == gfx::Painter::Brush::MASK) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/mask.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex");
						program.createUniform("mask");

						//binding the samplers
						program.setUniform("tex", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
						program.setUniform("mask", static_cast<int>(gfx::PainterTextureSlots::MASK));
					} else if (settings.first == gfx::Painter::Brush::MASK) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/mask.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex");
						program.createUniform("mask");

						//binding the samplers
						program.setUniform("tex", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
						program.setUniform("mask", static_cast<int>(gfx::PainterTextureSlots::MASK));
					} else if (settings.first == gfx::Painter::Brush::BLEND) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/blend.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex1");
						program.createUniform("tex2");

						program.setUniform("tex1", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
						program.setUniform("tex2", static_cast<int>(gfx::PainterTextureSlots::BACKGROUND));
					} else if (settings.first == gfx::Painter::Brush::CONDITIONAL_MASK) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/conditional_mask.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex1");
						program.createUniform("tex2");
						program.createUniform("mask");

						//binding the samplers
						program.setUniform("tex1", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
						program.setUniform("tex2", static_cast<int>(gfx::PainterTextureSlots::BACKGROUND));
						program.setUniform("mask", static_cast<int>(gfx::PainterTextureSlots::MASK));
					} else if (settings.first == gfx::Painter::Brush::MULTICOMPONENT_BLEND) {
						program.attachShader(createShader(GL_FRAGMENT_SHADER, settings.second,
#							include <MACE/Graphics/OGL/Shaders/Brushes/multicomponent_blend.f.glsl>
						));

						program.link();

						program.bind();

						program.createUniform("tex1");
						program.createUniform("tex2");

						program.setUniform("tex1", static_cast<int>(gfx::PainterTextureSlots::FOREGROUND));
						program.setUniform("tex2", static_cast<int>(gfx::PainterTextureSlots::BACKGROUND));

						prot.sourceBlend = GL_SRC1_COLOR;
						prot.destBlend = GL_ONE_MINUS_SRC1_COLOR;

						prot.multitarget = false;
					} else MACE_UNLIKELY{
						MACE__THROW(gfx::BadFormat, "OpenGL 3.3 Renderer: Unsupported brush type: " + std::to_string(static_cast<unsigned int>(settings.first)));
					}

					ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating shader program for RenderProtocol");

					program.bind();

					program.createUniformBuffer(MACE_STRINGIFY_DEFINITION(MACE__ENTITY_DATA_NAME), MACE__ENTITY_DATA_LOCATION);
					program.createUniformBuffer(MACE_STRINGIFY_DEFINITION(MACE__PAINTER_DATA_NAME), MACE__PAINTER_DATA_LOCATION);

					prot.program = program;

					prot.created = true;

					ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating uniform buffer for RenderProtocol ShaderProgram");
				}

				Renderer::ProtocolHash hashSettings(const std::pair<gfx::Painter::Brush, gfx::Painter::RenderFeatures>& settings) {
					Renderer::ProtocolHash j;
					/*
					endianness doesn't really matter, since the only purpose for this hash is
					for unordered_map. so as long as it's consisent at runtime, its good.
					*/
					j = static_cast<Renderer::ProtocolHash>(settings.first);
					j <<= 8;
					j |= static_cast<Renderer::ProtocolHash>(settings.second);
					return j;
				}

				std::unordered_map<gfx::FrameBufferTarget, const Enum*> generateFramebufferTargetLookup() {
					static MACE_CONSTEXPR const Enum colorBuffers[] = {
								GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX,
								GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX,
					};

					static MACE_CONSTEXPR const Enum dataBuffers[] = {
							GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX,
							GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX,
					};

					std::unordered_map<gfx::FrameBufferTarget, const Enum*> out{};
					out[gfx::FrameBufferTarget::COLOR] = colorBuffers;
					out[gfx::FrameBufferTarget::DATA] = dataBuffers;
					return out;
				}

				const Enum* lookupFramebufferTarget(const gfx::FrameBufferTarget target) {
					static auto lookupTable = generateFramebufferTargetLookup();

					return lookupTable[target];
				}
			}//anon namespace

			Renderer::Renderer(std::shared_ptr<Context> context) : Dispatchable(context) {}

			void Renderer::onInit() {
				const int version = gladLoadGL(glfwGetProcAddress);
				if (version == 0) {
					std::ostringstream errorMessage;
					errorMessage << "This system (OpenGL " << glGetString(GL_VERSION) << ")";
					errorMessage << " does not support the required OpenGL version required by this renderer, ";
					errorMessage << "OpenGL 3.3";
					MACE__THROW(gfx::UnsupportedRenderer, errorMessage.str());
				}

#ifdef MACE_DEBUG
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "OpenGL Info:\n";
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "Version:\n\t";
				std::cout << os::consoleColor(os::ConsoleColor::GREEN) << glGetString(GL_VERSION) << '\n';
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "Vendor:\n\t";
				std::cout << os::consoleColor(os::ConsoleColor::GREEN) << glGetString(GL_VENDOR) << '\n';
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "Renderer:\n\t";
				std::cout << os::consoleColor(os::ConsoleColor::GREEN) << glGetString(GL_RENDERER) << '\n';
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "Shader version:\n\t";
				std::cout << os::consoleColor(os::ConsoleColor::GREEN) << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
				std::cout << os::consoleColor(os::ConsoleColor::LIGHT_GREEN) << "Supported extensions:\n";
				std::cout << os::consoleColor(os::ConsoleColor::GREEN);
#define MACE__DEBUG_OUTPUT_EXTENSION(ext) if(GLAD_##ext){std::cout << "\t* " << #ext << '\n';}
				MACE__DEBUG_OUTPUT_EXTENSION(GL_ARB_buffer_storage);
				MACE__DEBUG_OUTPUT_EXTENSION(GL_ARB_texture_storage);
				MACE__DEBUG_OUTPUT_EXTENSION(GL_ARB_invalidate_subdata);
				MACE__DEBUG_OUTPUT_EXTENSION(GL_ARB_multi_bind);
				MACE__DEBUG_OUTPUT_EXTENSION(GL_ARB_direct_state_access);
				MACE__DEBUG_OUTPUT_EXTENSION(GL_EXT_direct_state_access);
#undef MACE__DEBUG_OUTPUT_EXTENSION
				std::cout << os::consoleColor() << std::flush;
#endif

				ogl::enable(GL_BLEND);
				ogl::enable(GL_MULTISAMPLE);
			}

			void Renderer::onSetUp(gfx::WindowModule*) {
				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: An error occured before onSetUp");

				ogl::resetBlending();

				frameBuffer.bind();

				MACE_CONSTEXPR const Enum buffers[] = {
					GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX,
					GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX,
					GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX
				};

				frameBuffer.setDrawBuffers(3, buffers);

				glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);

				glClearBufferfv(GL_COLOR, MACE__SCENE_ATTACHMENT_INDEX, clearColor.begin());

				MACE_CONSTEXPR const GLuint idClearValue = 0;

				glClearBufferuiv(GL_COLOR, MACE__ID_ATTACHMENT_INDEX, &idClearValue);

				MACE_CONSTEXPR const float dataClearValues[] = {
					0.0f,
					0.0f,
					0.0f,
					0.0f
				};

				glClearBufferfv(GL_COLOR, MACE__DATA_ATTACHMENT_INDEX, dataClearValues);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Failed to clear framebuffer");

				RenderProtocol& proto = protocols[currentProtocol];
				if (proto.created) {
					ogl::setBlending(proto.sourceBlend, proto.destBlend);

					bindCurrentTarget();
				}
			}

			void Renderer::onTearDown(gfx::WindowModule* win) {
				ogl::checkGLError(__LINE__, __FILE__, "Error occured during rendering");

				//frameBuffer.unbind();

				const Vector<Pixels, 2> size = win->getWindowSize();

				if (GLAD_GL_ARB_direct_state_access) {
					glNamedFramebufferReadBuffer(frameBuffer.getID(), GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX);
					//glNamedFramebufferDrawBuffer(frameBuffer.getID(), GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX);
					//glNamedFramebufferDrawBuffer(0, GL_FRONT);
				} else {
					glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer.getID());
					ogl::FrameBuffer::setReadBuffer(GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX);
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				ogl::FrameBuffer::setDrawBuffer(GL_FRONT);

				if (GLAD_GL_ARB_invalidate_subdata) {
					MACE_CONSTEXPR const Enum buffers[] = {
						GL_COLOR
					};

					glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, os::getArraySize<GLsizei>(buffers), buffers);
				}

				if (GLAD_GL_ARB_direct_state_access) {
					glBlitNamedFramebuffer(frameBuffer.getID(), 0, 0, 0, size[0], size[1], 0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
				} else {
					glBlitFramebuffer(0, 0, size[0], size[1], 0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
				}

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Failed to tear down renderer");

				//glfwSwapBuffers(win->getGLFWWindow());

				if (GLAD_GL_ARB_invalidate_subdata) {
					MACE_CONSTEXPR const Enum buffers[] = {
						GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX,
						GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX,
						GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX
					};

					if (GLAD_GL_ARB_direct_state_access) {
						glInvalidateNamedFramebufferData(frameBuffer.getID(), os::getArraySize<GLsizei>(buffers), buffers);
					} else {
						//our framebuffer was bound to GL_READ_FRAMEBUFFER in the previous if statement for blitting
						glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, os::getArraySize<GLsizei>(buffers), buffers);
					}
				}

				glFlush();

				ogl::forceCheckGLError(__LINE__, __FILE__, "An OpenGL error occurred during a rendering frame");
			}

			void Renderer::onResize(gfx::WindowModule*, const Pixels width, const Pixels height) {
				frameBuffer.destroy();
				{
					Object* renderBuffers[] = {
						&sceneBuffer,
						&idBuffer,
						&dataBuffer,
						&depthStencilBuffer
					};

					RenderBuffer::destroy(renderBuffers, 4);
				}

				//if the window is iconified, width and height will be 0. we cant create a framebuffer of size 0, so we make it 1 instead

				ogl::setViewport(0, 0, math::max<Pixels>(1, width), math::max<Pixels>(1, height));

				generateFramebuffer(math::max<Pixels>(1, width), math::max<Pixels>(1, height));

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error resizing framebuffer for renderer");
			}

			void Renderer::onDestroy() {
				frameBuffer.destroy();

				{
					Object* renderBuffers[] = {
						&sceneBuffer,
						&idBuffer,
						&dataBuffer,
						&depthStencilBuffer
					};

					RenderBuffer::destroy(renderBuffers, 4);
				}

				for (auto iter = protocols.begin(); iter != protocols.end(); ++iter) {
					iter->second.program.destroy();
				}

				protocols.clear();

				ogl::forceCheckGLError(__LINE__, __FILE__, "Internal Error: Error destroying OpenGL 3.3 renderer");

			}

			void Renderer::onQueue(gfx::Entity*) {}

			void Renderer::setRefreshColor(const float r, const float g, const float b, const float a) {
				clearColor = Color(r, g, b, a);
			}

			void Renderer::generateFramebuffer(const Pixels width, const Pixels height) {
				{
					Object* renderBuffers[] = {
						&sceneBuffer,
						&idBuffer,
						&dataBuffer,
						&depthStencilBuffer
					};

					RenderBuffer::init(renderBuffers, 4);
				}

				depthStencilBuffer.setStorage(GL_DEPTH_STENCIL, width, height);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating depth buffers for renderer");

				sceneBuffer.setStorage(GL_RGBA, width, height);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating scene texture for renderer");

				idBuffer.setStorage(GL_R32UI, width, height);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating id texture for renderer");

				dataBuffer.setStorage(GL_RGBA, width, height);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating data texture for renderer");

				frameBuffer.init();
				frameBuffer.bind();

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error creating FrameBuffer for the renderer");

				frameBuffer.attachRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX, sceneBuffer);
				frameBuffer.attachRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX, idBuffer);
				frameBuffer.attachRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX, dataBuffer);
				frameBuffer.attachRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilBuffer);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error attaching texture to FrameBuffer for the renderer");

				switch (frameBuffer.checkStatus(GL_FRAMEBUFFER)) {
				case GL_FRAMEBUFFER_UNDEFINED:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_UNDEFINED: The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist. ");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: One of the framebuffer attachments are incomplete!");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: The framebuffer is missing at least one image");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER. ");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi. ");
					break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached images violates an implementation-dependent set of restrictions. ");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES. It can also be that the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures. ");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					MACE__THROW(internal::ogl::Framebuffer, "GL_FRAMEBUFFER_LAYER_TARGETS: Any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target. ");
					break;
				MACE_LIKELY case GL_FRAMEBUFFER_COMPLETE:
					MACE_FALLTHROUGH;
				default:
					//success
					break;
				}

				setTarget(gfx::FrameBufferTarget::COLOR);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error setting draw buffers in FrameBuffer for the renderer");

				ogl::setViewport(0, 0, width, height);

				ogl::resetBlending();

				ogl::forceCheckGLError(__LINE__, __FILE__,
					("Internal Error: Renderer: Error resizing framebuffer for width " + std::to_string(width) + " and height " + std::to_string(height)).c_str());
			}

			void Renderer::getEntitiesAt(const Pixels x, const Pixels y, const Pixels w, const Pixels h, gfx::EntityID* arr) const {
				if (GLAD_GL_ARB_direct_state_access) {
					glNamedFramebufferReadBuffer(frameBuffer.getID(), GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX);
				} else {
					frameBuffer.bind();

					ogl::FrameBuffer::setReadBuffer(GL_COLOR_ATTACHMENT0 + MACE__ID_ATTACHMENT_INDEX);
				}
				//opengl y-axis is inverted from window coordinates
				frameBuffer.readPixels(x, getHeight() - y, w, h, GL_RED_INTEGER, GL_UNSIGNED_INT, arr);

				ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error getting Entity from index buffer");
			}

			void Renderer::getPixelsAt(const Pixels x, const Pixels y, const Pixels w, const Pixels h, Color* arr, const gfx::FrameBufferTarget target) const {
				Enum colorAttachment;
				switch (target) {
				case gfx::FrameBufferTarget::COLOR:
					MACE_FALLTHROUGH;
				default:
					colorAttachment = GL_COLOR_ATTACHMENT0 + MACE__SCENE_ATTACHMENT_INDEX;
					break;
				case gfx::FrameBufferTarget::DATA:
					colorAttachment = GL_COLOR_ATTACHMENT0 + MACE__DATA_ATTACHMENT_INDEX;
					break;
				}

				if (GLAD_GL_ARB_direct_state_access) {
					glNamedFramebufferReadBuffer(frameBuffer.getID(), colorAttachment);
				} else {
					frameBuffer.bind();

					ogl::FrameBuffer::setReadBuffer(colorAttachment);
				}

				//opengl y-axis is inverted from window coordinates
				frameBuffer.readPixels(x, getHeight() - y, w, h, GL_RGBA, GL_FLOAT, arr);
			}

			std::shared_ptr<gfx::PainterImpl> Renderer::createPainterImpl() {
				return std::shared_ptr<gfx::PainterImpl>(new Painter(context, this));
			}

			void Renderer::bindProtocol(Painter* painter, const std::pair<gfx::Painter::Brush, gfx::Painter::RenderFeatures> settings) {
				const Renderer::ProtocolHash hash = hashSettings(settings);
				//its a pointer so that we dont do a copy operation on assignment here
				RenderProtocol& protocol = protocols[hash];

				if (!protocol.created) MACE_UNLIKELY{
					initializeRenderProtocolForSettings(protocol, settings);
				}

				MACE_ASSERT(protocol.created, "Internal Error: initalizeRenderProtocolForSettings() did not properly create the requested RenderProtocol");

				const RenderProtocol& oldProtocol = protocols[currentProtocol];

				//if there hasn't been a protocol bound yet, bind everything to default values
				if (!oldProtocol.created || hash != currentProtocol) {
					currentProtocol = hash;

					if (oldProtocol.sourceBlend != protocol.sourceBlend || oldProtocol.destBlend != protocol.destBlend) {
						ogl::setBlending(protocol.sourceBlend, protocol.destBlend);
					}

					if (oldProtocol.multitarget != protocol.multitarget) {
						bindCurrentTarget();
					}

					ogl::checkGLError(__LINE__, __FILE__, "Internal Error: Error swapping RenderProtocol");
				}

				if (oldProtocol.program.getID() != protocol.program.getID()) {
					protocol.program.bind();
				}

				const UniformBuffer* buffers[] = {
					&painter->uniformBuffers.entityData,
					&painter->uniformBuffers.painterData
				};

				protocol.program.bindUniformBuffers(buffers, os::getArraySize(buffers));

				checkGLError(__LINE__, __FILE__, "Internal Error: An error occured binding the RenderProtocol");
			}

			void Renderer::setTarget(const gfx::FrameBufferTarget& target) {
				if (target != currentTarget) {
					currentTarget = target;
					bindCurrentTarget();
				}
			}

			void Renderer::bindCurrentTarget() {
				frameBuffer.setDrawBuffers(protocols[currentProtocol].multitarget ? 2 : 1, lookupFramebufferTarget(currentTarget));
			}

			Painter::Painter(std::shared_ptr<Context> con, Renderer* const r) : renderer(r), Dispatchable(con) {}

			void Painter::init() {
				dispatch([this]() {
					Object* buffers[] = {
						&uniformBuffers.entityData,
						&uniformBuffers.painterData
					};

					UniformBuffer::init(buffers, 2);

					createPainterData();
					createEntityData();
				});
			}

			void Painter::createEntityData() {
				MACE_STATIC_ASSERT(sizeof(float) >= sizeof(gfx::EntityID), "This system doesn't not support the required size for EntityID");

				savedMetrics = painter->getEntity()->getMetrics();

				const Transformation& transform = savedMetrics.transform;
				const Transformation& inherited = savedMetrics.inherited;

				float entityDataBuffer[MACE__ENTITY_DATA_BUFFER_SIZE / sizeof(float)] = {};

				transform.translation.flatten(entityDataBuffer);
				//offset by 4
				transform.rotation.flatten(entityDataBuffer + 4);
				transform.scaler.flatten(entityDataBuffer + 8);
				inherited.translation.flatten(entityDataBuffer + 12);
				inherited.rotation.flatten(entityDataBuffer + 16);
				inherited.scaler.flatten(entityDataBuffer + 20);
				//this crazy line puts a float directly into a GLuint, as GLSL expects a uint instead of a float
				*reinterpret_cast<GLuint*>(entityDataBuffer + 24) = static_cast<GLuint>(painter->getEntity()->getID());

				uniformBuffers.entityData.createStorage(MACE__ENTITY_DATA_BUFFER_SIZE, entityDataBuffer, MACE__ENTITY_DATA_STORAGE_FLAGS);

				uniformBuffers.entityData.setName(MACE_STRINGIFY_DEFINITION(MACE__ENTITY_DATA_NAME));
			}

			void Painter::createPainterData() {
				savedState = painter->getState();//create default state

				float painterDataBuffer[MACE__PAINTER_DATA_BUFFER_SIZE / sizeof(float)] = {0};
				savedState.data.flatten(painterDataBuffer);
				savedState.foregroundColor.flatten(painterDataBuffer + 4);
				savedState.foregroundTransform.flatten(painterDataBuffer + 8);
				savedState.backgroundColor.flatten(painterDataBuffer + 12);
				savedState.backgroundTransform.flatten(painterDataBuffer + 16);
				savedState.maskColor.flatten(painterDataBuffer + 20);
				savedState.maskTransform.flatten(painterDataBuffer + 24);
				savedState.filter.flatten(painterDataBuffer + 28);

				uniformBuffers.painterData.createStorage(MACE__PAINTER_DATA_BUFFER_SIZE, painterDataBuffer, MACE__PAINTER_DATA_STORAGE_FLAGS);

				uniformBuffers.painterData.setName(MACE_STRINGIFY_DEFINITION(MACE__PAINTER_DATA_NAME));
			}

			void Painter::destroy() {
				dispatch([this]() {
					Object* buffers[] = {
						&uniformBuffers.entityData,
						&uniformBuffers.painterData
					};

					UniformBuffer::destroy(buffers, 2);
				});
			}

			void Painter::begin() {}

			void Painter::end() {}

			void Painter::setTarget(const gfx::FrameBufferTarget& target) {
				dispatch([this, &target]() {
					renderer->setTarget(target);
				});
			}

			void Painter::clean() {
				dispatch([this]() {
					MACE_ASSERT(painter->getEntity()->isInit(), "Internal Error: Renderer: clean(): Painter Entity is not initializd.");

					const gfx::Metrics metrics = painter->getEntity()->getMetrics();

					if (metrics == savedMetrics) {
						return;
					}

					const Transformation& transform = metrics.transform;
					const Transformation& inherited = metrics.inherited;

					//now we set the data defining the transformations of the entity
					uniformBuffers.entityData.bind();

#define MACE__DATA_RANGE_ENTRY(name, offset, size) if(name != savedMetrics.name) {uniformBuffers.entityData.setDataRange(sizeof(float) * offset, sizeof(float) * size, name.begin());}
					MACE__DATA_RANGE_ENTRY(transform.translation, 0, 3);
					MACE__DATA_RANGE_ENTRY(transform.rotation, 4, 3);
					MACE__DATA_RANGE_ENTRY(transform.scaler, 8, 3);
					MACE__DATA_RANGE_ENTRY(inherited.translation, 12, 3);
					MACE__DATA_RANGE_ENTRY(inherited.rotation, 16, 3);
					MACE__DATA_RANGE_ENTRY(inherited.scaler, 20, 3);
#undef MACE__DATA_RANGE_ENTRY

					savedMetrics = metrics;

					checkGLError(__LINE__, __FILE__, "Internal Error: Failed to update GPU-side buffer in Entity clean");
				});
			}

			void Painter::loadSettings(const gfx::Painter::State& state) {
				dispatch([this, &state]() {
					if (state == savedState) {
						return;
					}

					uniformBuffers.painterData.bind();

#define MACE__DATA_RANGE_ENTRY(name, offset, size, cond) if(state.name != savedState.name && cond){uniformBuffers.painterData.setDataRange(sizeof(float) * offset, sizeof(float) * size, state.name.begin());}
					MACE__DATA_RANGE_ENTRY(data, 0, 4, true);
					MACE__DATA_RANGE_ENTRY(foregroundColor, 4, 4, true);
					MACE__DATA_RANGE_ENTRY(foregroundTransform, 8, 4, MACE__HAS_RENDER_FEATURE(state.renderFeatures, TEXTURE_TRANSFORM));
					MACE__DATA_RANGE_ENTRY(backgroundColor, 12, 4, true);
					MACE__DATA_RANGE_ENTRY(backgroundTransform, 16, 4, MACE__HAS_RENDER_FEATURE(state.renderFeatures, TEXTURE_TRANSFORM));
					MACE__DATA_RANGE_ENTRY(maskColor, 20, 4, true);
					MACE__DATA_RANGE_ENTRY(maskTransform, 24, 4, MACE__HAS_RENDER_FEATURE(state.renderFeatures, TEXTURE_TRANSFORM));
					MACE__DATA_RANGE_ENTRY(filter, 28, 16, MACE__HAS_RENDER_FEATURE(state.renderFeatures, FILTER));
#undef MACE__DATA_RANGE_ENTRY

					savedState = state;
				});
			}

			void Painter::draw(const gfx::Model& m, const gfx::Painter::Brush brush) {
				dispatch([this, &m, brush]() {
					renderer->bindProtocol(this, {brush, savedState.renderFeatures});

					m.draw();

					checkGLError(__LINE__, __FILE__, "Internal Error: An error occured while drawing a model");
				});
			}
		}//ogl
	}//internal
}//mc

#if defined(_MSC_VER) && _MSC_VER >= 1400 
 //pop the disable of warning 4996 which is a useless warning in our scenario
#	pragma warning( pop ) 
#endif 