/*
The MIT License (MIT)

Copyright (c) 2016 Liav Turkia

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#pragma once
#include <GL/glew.h>
#include <MACE/System/Constants.h>

namespace mc {
namespace gfx{

template<GLenum bufferType>
class Buffer {
public:
	virtual ~Buffer() {
	};

	void bind() const{
		glBindBuffer(bufferType, id);
	};
	void unbind() const {
		glBindBuffer(bufferType, 0);
	};

	Index getID() const {
		return id;
	};

	GLboolean isBuffer() const {
		return glIsBuffer(id);
	};

	void init() {
		glGenBuffers(1, &id);
	};
	void destroy() {
		glDeleteBuffers(1,&id);
	};

	void setImmutableData(const GLsizeiptr dataSize, const GLvoid* data, GLbitfield flags) {
		glBufferStorage(bufferType,dataSize,data,flags);
	};
	void setData(const GLsizeiptr dataSize, const GLvoid* data, const GLenum drawType = GL_DYNAMIC_DRAW) const {
		glBufferData(bufferType, dataSize, data, drawType);
	};
	void setDataRange(const Index offset, const GLsizeiptr dataSize, const GLvoid* data) const {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, data);
	};

	template<GLenum otherType>
	void copyData(Buffer<otherType> other, GLsizeiptr size, Index readOffset = 0, Index writeOffset = 0) {
		glCopyBufferSubData(id,other.id,readOffset,writeOffset,size);
	};

	GLvoid* map(const GLenum access = GL_READ_WRITE) {
		return glMapBuffer(bufferType,access);
	};

	GLvoid* mapRange(const Index offset, const Size length, const GLbitfield access) {
		return glMapBufferRange(bufferType,offset,length,access);
	};

	GLboolean unmap() {
		return glUnmapBuffer(bufferType);
	};
protected:
	Index id=0;
};//Buffer

class UniformBuffer : public Buffer<GL_UNIFORM_BUFFER> {
public:
	void setLocation(const Index location);
	Index getLocation();
	const Index getLocation() const;

	void bindForRender(const Index offset = 0, const GLsizeiptr size = -1) const;

	void bindToUniformBlock(const Index programID, const char* blockName) const;

private:
	Index location = 0;
};//UniformBuffer

class RenderBuffer {
public:
	void init();
	void destroy();

	void bind() const;
	void unbind() const;

	void setStorage(const GLenum& format, const GLsizei& width, const GLsizei& height);
	void setStorageMultisampled(const GLsizei& samples, const GLenum& format, const GLsizei& width, const GLsizei& height);

	GLboolean isCreated() const;

	Index getLocation() const;
private:
	Index location = 0;
};//RenderBuffer

class FrameBuffer {
public:
	void init();
	void destroy();

	void bind(const GLenum& target = GL_FRAMEBUFFER) const;
	void unbind() const;

	void attachTexture(const GLenum& target, const GLenum& attachment, const GLuint& textureID, const GLint& level);
	void attachTexture1D(const GLenum& target, const GLenum& attachment, const GLenum& texTarget, const GLuint& textureID, const GLint& level);
	void attachTexture2D(const GLenum& target, const GLenum& attachment, const GLenum& texTarget, const GLuint& textureID, const GLint& level);
	void attachTextureLayer(const GLenum& target, const GLenum& attachment, const GLuint& texture, const GLint& level, const GLint& layer);

	void attachRenderbuffer(const GLenum& target, const GLenum& attachment, const RenderBuffer& buffer);

	void setParameter(const GLenum& paramName, const GLint& value);

	void setDrawBuffers(const Size& arrSize, const GLenum* buffers);

	GLboolean isCreated() const;

	GLenum checkStatus(const GLenum& target);

	Index getLocation() const;
private:
	Index location = 0;
};//FrameBuffer

}//gfx
}//mc