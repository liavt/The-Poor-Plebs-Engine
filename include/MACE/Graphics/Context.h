/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#pragma once
#ifndef MACE__GRAPHICS_CONTEXT_H
#define MACE__GRAPHICS_CONTEXT_H

#include <MACE/Core/Constants.h>
#include <MACE/Core/Interfaces.h>
#include <MACE/Graphics/Window.h>
#include <MACE/Utility/Color.h>
#include <memory>
#include <map>
#include <string>
#include <functional>

#ifdef MACE_OPENCV
#	include <opencv2/opencv.hpp>
#endif//MACE_OPENCV

namespace mc {
	namespace gfx {
		class Renderer;

		/**
		Thrown when an error occured trying to read or write an image
		*/
		MACE__DECLARE_ERROR(BadImage);

		/**
		Thrown when a format is invalid
		*/
		MACE__DECLARE_ERROR(BadFormat);

		enum class PixelStorage: Byte {
			ALIGNMENT,
			ROW_LENGTH
		};

		enum class ImageFormat: Byte {
			//each enum is equal to how many components in that type of image
			//the image load/save functions use swizzle masks to differentiate
			//between LUMINANCE and R, LUMINANCE_ALPHA and RG
			LUMINANCE = 1,
			LUMINANCE_ALPHA = 2,
			INTENSITY = 1,
			R = 1,
			RG = 2,
			RGB = 3,
			RGBA = 4,
			DONT_CARE = 0
		};

		enum class PrimitiveType: short int {
			//these draw modes were chosen because they exist in both OpenGL 3.3 and Direct3D
			POINTS = 0,
			LINES = 1,
			LINES_ADJACENCY = 2,
			LINES_STRIP = 3,
			LINES_STRIP_ADJACENCY = 4,
			TRIANGLES = 5,
			TRIANGLES_ADJACENCY = 6,
			TRIANGLES_STRIP = 7,
			TRIANGLES_STRIP_ADJACENCY = 8,
			//TRIANGLES_FAN does not exist in DirectX 10+ because of performance issues. It is not included in this list
		};

		enum class TextureSlot: Byte {
			FOREGROUND = 0,
			BACKGROUND = 1,
			MASK = 2
		};

		class MACE_NOVTABLE ModelImpl: public Initializable, public Bindable {
			friend class Model;
		public:
			virtual void init() override = 0;
			virtual void destroy() override = 0;

			virtual void bind() const override = 0;
			virtual void unbind() const override = 0;

			virtual void draw() const = 0;

			virtual void loadTextureCoordinates(const unsigned int dataSize, const float* data) = 0;
			virtual void loadVertices(const unsigned int verticeSize, const float* vertices) = 0;
			virtual void loadIndices(const unsigned int indiceNum, const unsigned int* indiceData) = 0;

			virtual bool isCreated() const = 0;

			bool operator==(const ModelImpl& other) const;
			bool operator!=(const ModelImpl& other) const;
		protected:
			PrimitiveType primitiveType = PrimitiveType::TRIANGLES;
		};

		class Model: public Initializable, public Bindable {
		public:
			static Model& getQuad();

			Model();
			Model(const std::shared_ptr<ModelImpl> mod);
			Model(const Model& other);
			~Model() = default;

			void init() override;
			void destroy() override;

			void bind() const override;
			void unbind() const override;

			void createTextureCoordinates(const unsigned int dataSize, const float* data);
			template<const unsigned int N>
			void createTextureCoordinates(const float(&data)[N]) {
				createTextureCoordinates(N, data);
			}

			void createVertices(const unsigned int verticeSize, const float* vertices, const PrimitiveType& prim);
			template<const unsigned int N>
			void createVertices(const float(&vertices)[N], const PrimitiveType& prim) {
				createVertices(N, vertices, prim);
			}

			void createIndices(const unsigned int indiceNum, const unsigned int* indiceData);
			template<const unsigned int N>
			void createIndices(const unsigned int(&indiceData)[N]) {
				createIndices(N, indiceData);
			}

			PrimitiveType getPrimitiveType();
			const PrimitiveType getPrimitiveType() const;

			void draw() const;

			bool isCreated() const;

			bool operator==(const Model& other) const;
			bool operator!=(const Model& other) const;
		private:
			std::shared_ptr<ModelImpl> model;
		};

		struct TextureDesc {
			enum class Type: short int {
				UNSIGNED_BYTE,
				BYTE,
				UNSIGNED_SHORT,
				SHORT,
				UNSIGNED_INT,
				INT,
				FLOAT,
				UNSIGNED_BYTE_3_3_2,
				UNSIGNED_BYTE_2_3_3_REV,
				UNSIGNED_SHORT_5_6_5,
				UNSIGNED_SHORT_5_6_5_REV,
				UNSIGNED_SHORT_4_4_4_4,
				UNSIGNED_SHORT_4_4_4_4_REV,
				UNSIGNED_SHORT_5_5_5_1,
				UNSIGNED_SHORT_1_5_5_5_REV,
				UNSIGNED_INT_8_8_8_8,
				UNSIGNED_INT_8_8_8_8_REV,
				UNSIGNED_INT_10_10_10_2,
				UNSIGNED_INT_2_10_10_10_REV,
			};

			enum class Format: short int {
				RED,
				RG,
				RGB,
				RGBA,
				BGR,
				BGRA,
				RED_INTEGER,
				RG_INTEGER,
				RGB_INTEGER,
				BGR_INTEGER,
				RGBA_INTEGER,
				BGRA_INTEGER,
				STENCIL,
				DEPTH,
				DEPTH_STENCIL,
				LUMINANCE,
				LUMINANCE_ALPHA,
				INTENSITY
			};

			enum class InternalFormat: Enum {
				DEPTH,
				DEPTH_STENCIL,
				RED,
				RG,
				RGB,
				RGBA,
				R8,
				R16,
				RGB8,
				RGBA8,
				SRGB,
				SRGB8,
				SRGB_ALPHA
			};

			enum class Filter: Byte {
				//MIPMAP_LINEAR and MIPMAP_NEAREST automatically
				//generate mipmaps in OpenGL renderer.
				//They cant be used for the magnication filter,
				//only the minification filter
				MIPMAP_LINEAR,
				MIPMAP_NEAREST,
				NEAREST,
				LINEAR,
				ANISOTROPIC
			};

			enum class Wrap: Byte {
				CLAMP,
				WRAP,
				MIRROR,
				BORDER
			};

			TextureDesc() = default;
			TextureDesc(const unsigned int w, const unsigned int h, const Format form = Format::RGBA);

			Filter minFilter = Filter::LINEAR;
			Filter magFilter = Filter::LINEAR;
			Type type = Type::FLOAT;
			Format format = Format::RGBA;
			InternalFormat internalFormat = InternalFormat::RGBA;

			unsigned int width = 0, height = 0;

			Wrap wrapS = Wrap::CLAMP;
			Wrap wrapT = Wrap::CLAMP;
			Color borderColor = Colors::INVISIBLE;
		};

		class MACE_NOVTABLE TextureImpl: public Bindable {
			friend class Texture;
		public:
			TextureImpl(const TextureDesc& t);
			virtual ~TextureImpl() = default;

			virtual void bind() const override = 0;
			virtual void bind(const TextureSlot slot) const = 0;
			virtual void unbind() const override = 0;

			virtual bool isCreated() const = 0;

			virtual void setData(const void* data, const int mipmap) = 0;

			virtual void setUnpackStorageHint(const PixelStorage hint, const int value) = 0;
			virtual void setPackStorageHint(const PixelStorage hint, const int value) = 0;

			virtual void readPixels(void* data) const = 0;
		protected:
			const TextureDesc desc;
		};

		class Texture: public Bindable {
		public:
			static Texture create(const Color& col, const unsigned int width = 1, const unsigned int height = 1);
			static Texture createFromFile(const std::string& file, const ImageFormat format = ImageFormat::DONT_CARE, const TextureDesc::Wrap wrap = TextureDesc::Wrap::CLAMP);
			static Texture createFromFile(const char* file, const ImageFormat format = ImageFormat::DONT_CARE, const TextureDesc::Wrap wrap = TextureDesc::Wrap::CLAMP);
			static Texture createFromMemory(const unsigned char* c, const int size);

			static Texture& getSolidColor();
			/**
			- Vertical gradient
			- Darker part on bottom
			- Height is 100
			*/
			static Texture& getGradient();

			Texture();
			Texture(const TextureDesc& d);
			Texture(const Color& col);
			Texture(const std::shared_ptr<TextureImpl> tex, const Color& col = Color(0.0f, 0.0f, 0.0f, 0.0f));
			Texture(const Texture& tex, const Color& col = Color(0.0, 0.0f, 0.0f, 0.0f));

			void init(const TextureDesc& desc);
			void destroy();

			bool isCreated() const;

			const TextureDesc& getDesc() const;

			unsigned int getWidth();
			const unsigned int getWidth() const;

			unsigned int getHeight();
			const unsigned int getHeight() const;

#ifdef MACE_EXPOSE_OPENGL
			std::shared_ptr<TextureImpl> getImpl() {
				return texture;
			}

			const std::shared_ptr<TextureImpl> getImpl() const {
				return texture;
			}
#endif

			//this needs to be defined in the header file to prevent linker conflicts, because Entity.cpp does not have opencv included ever.
#			ifdef MACE_OPENCV
			Texture(cv::Mat mat) : Texture() {
				TextureDesc desc = TextureDesc(mat.rows, mat.cols);

				if (mat.channels() == 1) {
					desc.format = TextureDesc::Format::RED;
					desc.internalFormat = TextureDesc::InternalFormat::RED;
				} else if (mat.channels() == 2) {
					desc.format = TextureDesc::Format::RG;
					desc.internalFormat = TextureDesc::InternalFormat::RG;
				} else if (mat.channels() == 3) {
					desc.format = TextureDesc::Format::BGR;
					desc.internalFormat = TextureDesc::InternalFormat::RGB;
				} else if (mat.channels() == 4) {
					desc.format = TextureDesc::Format::BGRA;
					desc.internalFormat = TextureDesc::InternalFormat::RGBA;
				}

				desc.type = TextureDesc::Type::UNSIGNED_BYTE;
				if (mat.depth() == CV_8S) {
					desc.type = TextureDesc::Type::BYTE;
				} else if (mat.depth() == CV_16U) {
					desc.type = TextureDesc::Type::UNSIGNED_SHORT;
				} else if (mat.depth() == CV_16S) {
					desc.type = TextureDesc::Type::SHORT;
				} else if (mat.depth() == CV_32S) {
					desc.type = TextureDesc::Type::INT;
				} else if (mat.depth() == CV_32F) {
					desc.type = TextureDesc::Type::FLOAT;
				} else if (mat.depth() == CV_64F) {
					MACE__THROW(BadFormat, "Unsupported cv::Mat depth: CV_64F");
				}

				init(desc);

				resetPixelStorage();
				setPackStorageHint(PixelStorage::ALIGNMENT, (mat.step & 3) ? 1 : 4);
				setPackStorageHint(PixelStorage::ROW_LENGTH, static_cast<int>(mat.step / mat.elemSize()));

				setData(mat.ptr());
			}

			cv::Mat toMat() {
				cv::Mat img(getHeight(), getWidth(), CV_8UC3);

				resetPixelStorage();
				setPackStorageHint(PixelStorage::ALIGNMENT, (img.step & 3) ? 1 : 4);
				setPackStorageHint(PixelStorage::ROW_LENGTH, static_cast<int>(img.step / img.elemSize()));

				bind();

				readPixels(img.data);

				cv::flip(img, img, 0);

				return img;
			}
#			endif//MACE_OPENCV

			Color & getHue();
			const Color & getHue() const;
			void setHue(const Color & col);

			Vector<float, 4> & getTransform();
			const Vector<float, 4> & getTransform() const;
			void setTransform(const Vector<float, 4> & trans);

			void bind() const override;
			void bind(const TextureSlot slot) const;
			void unbind() const override;

			void resetPixelStorage();

			void setData(const void* data, const int mipmap = 0);
			template<typename T, unsigned int W, unsigned int H>
			void setData(const T(&data)[W][H], const int mipmap = 0) {
				MACE_STATIC_ASSERT(W != 0, "Width of Texture can not be 0!");
				MACE_STATIC_ASSERT(H != 0, "Height of Texture can not be 0!");

				MACE_IF_CONSTEXPR(W != texture->desc.width || H != texture->desc.height) {
					MACE__THROW(AssertionFailed, "Input data is not equal to Texture width and height");
				}

				setData(static_cast<const void*>(data[0]), mipmap);
			}

			void setUnpackStorageHint(const PixelStorage hint, const int value);
			void setPackStorageHint(const PixelStorage hint, const int value);

			void readPixels(void* data) const;

			bool operator==(const Texture & other) const;
			bool operator!=(const Texture & other) const;
		private:
			std::shared_ptr<TextureImpl> texture;

			Color hue = Colors::INVISIBLE;

			Vector<float, 4> transform{0.0f, 0.0f, 1.0f, 1.0f};
		};//Texture

		struct RenderTargetDesc {
		public:
			unsigned int width = 128, height = 128;
		};

		class MACE_NOVTABLE RenderTargetImpl: public Initializable {

		};//RenderTargetImpl

		class GraphicsContext: public Initializable {
			friend class Texture;
			friend class Model;
		public:
			using TextureCreateCallback = std::function<Texture()>;
			using ModelCreateCallback = std::function<Model()>;

			GraphicsContext(gfx::WindowModule* win);
			//prevent copying
			GraphicsContext(const GraphicsContext& other) = delete;
			virtual ~GraphicsContext() = default;

			void init() override;
			void render();
			void destroy() override;

			virtual Renderer* getRenderer() const = 0;

			gfx::WindowModule* getWindow();
			const gfx::WindowModule* getWindow() const;

			Texture& createTexture(const std::string& name, const Texture& texture = Texture());
			Texture& getOrCreateTexture(const std::string& name, const TextureCreateCallback create);
			Texture& getOrCreateTextureFromFile(const std::string& name, const std::string& path);
			Model& createModel(const std::string& name, const Model& texture = Model());
			Model& getOrCreateModel(const std::string& name, const ModelCreateCallback create);

			bool hasTexture(const std::string& name) const;
			bool hasModel(const std::string& name) const;

			void setTexture(const std::string& name, const Texture& texture);
			Texture& getTexture(const std::string& name);
			const Texture& getTexture(const std::string& name) const;

			void setModel(const std::string& name, const Model& model);
			Model& getModel(const std::string& name);
			const Model& getModel(const std::string& name) const;

			std::map<std::string, Texture>& getTextures();
			const std::map<std::string, Texture>& getTextures() const;

			std::map<std::string, Model>& getModels();
			const std::map<std::string, Model>& getModels() const;
		protected:
			gfx::WindowModule* window;

			/*
			Reasoning behind returning a std::shared_ptr and not a std::unique_ptr...

			Each Model and Texture class needs to have it's own Impl.
			Each Impl acts as a pointer to a resource, whether it be in GPU memory
			or whatever. However, Model and Texture needs the ability to be moved,
			like so:

			Model model = Model();
			Model newModel = m;

			Both Model objects will share the same resource, as it is simply a
			pointer to the actual data. However, with std::unique_ptr, this move
			semantic because near impossible. Because multiple Model objects
			may have to own the same pointer to data, they have to use std::shared_ptr
			*/
			virtual std::shared_ptr<ModelImpl> createModelImpl() const = 0;
			virtual std::shared_ptr<TextureImpl> createTextureImpl(const TextureDesc& desc) const = 0;

			virtual void onInit(gfx::WindowModule* win) = 0;
			virtual void onRender(gfx::WindowModule* win) = 0;
			virtual void onDestroy(gfx::WindowModule* win) = 0;

		private:
			std::map<std::string, Texture> textures{};
			std::map<std::string, Model> models{};
		};
	}
}//mc

#endif//MACE__GRAPHICS_CONTEXT_H
