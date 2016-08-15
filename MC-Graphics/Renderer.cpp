#define _MACE_ENTITY2D_EXPOSE_X_MACRO
#include <MC-Graphics/Renderer.h>

namespace mc {
namespace gfx {

const char* Renderer::vertexShader2D = R"(
		#version 330 core

		precision highp float; // Defines precision for float and float-derived (vector/matrix) types.

		layout(location = 0) in vec3 vertexPosition;
		layout(location = 1) in vec2 texCoord;

		uniform mediump mat4 rotation;
		uniform mediump vec3 translation;
		uniform mediump vec3 scale;

		out lowp vec2 textureCoord;

		void main(){
			gl_Position = vec4(translation,1.0)+(rotation * vec4(scale * vertexPosition,0.0));//MAD operations right here!

			textureCoord=texCoord;
		}
	)";

const char* Renderer::fragmentShader2D = R"(
		#version 330 core

		precision highp float; // Defines precision for float and float-derived (vector/matrix) types.

		in lowp vec2 textureCoord;

		out lowp vec4 color;

		uniform lowp vec4 paint;
		uniform lowp float opacity;

		uniform lowp sampler2D tex;

		void main (void)  
		{     
			color= mix(vec4(paint.rgb,1.0),texture(tex,textureCoord),paint.a);
			color.w=opacity;
		}       
	)";

const GLfloat Renderer::squareVertices[] = {
	-1.0f,-1.0f,0.0f,
	-1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,-1.0f,0.0f
};

const GLfloat Renderer::squareTextureCoordinates[] = {
	0.0f,1.0f,
	0.0f,0.0f,
	1.0f,0.0f,
	1.0f,1.0f,
};

const GLuint Renderer::squareIndices[] = {
	0,1,3,
	1,2,3
};

VAO Renderer::square = VAO();

ShaderProgram Renderer::shaders2D = ShaderProgram();


#define _MACE_ENTITY2D_UNIFORM_ENTRY(a,type) \
type Renderer::a##CurrentlyBound = type##();	

_MACE_ENTITY2D_UNIFORM_VALUES
#undef _MACE_ENTITY2D_UNIFORM_ENTRY

void Renderer::init()
{

	//vao loading
	square.loadVertices(4, squareVertices);
	square.loadTextureCoordinates(4, squareTextureCoordinates);
	square.loadIndices(6, squareIndices);

	//shader stuff
	shaders2D.createVertex(vertexShader2D);
	shaders2D.createFragment(fragmentShader2D);

	shaders2D.init();

#define _MACE_ENTITY2D_UNIFORM_ENTRY(a, type)	\
	shaders2D.createUniform(#a);				\

	_MACE_ENTITY2D_UNIFORM_VALUES
#undef	_MACE_ENTITY2D_UNIFORM_ENTRY

	//gl states
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	checkGLError();
}
void Renderer::prepare()
{
}
void Renderer::resize(const Size width, const Size height)
{
	glViewport(0,0,width,height);
}
void Renderer::draw(Entity2D * e)
{
	const Texture& tex = e->getTexture();

	tex.bind();
	square.bind();
	shaders2D.bind();

	const TransformMatrix& transform = e->getBaseTransformation();

	//setting uniform costs quite a bit of performance when done constantly. We cache the current setting and only change it if its different
	const float& opacity = tex.getOpacity();
	const Vector3f& translation = transform.translation, scale = transform.scaler;
	const Matrix4f& rotation = (math::rotate(transform.rotation));
	const Color& paint = tex.getPaint();

#define _MACE_ENTITY2D_UNIFORM_ENTRY(a,type) \
	if(a != a##CurrentlyBound){ \
		shaders2D.setUniform(#a,a); \
		a##CurrentlyBound = a; \
	}

	_MACE_ENTITY2D_UNIFORM_VALUES
#undef	_MACE_ENTITY2D_UNIFORM_ENTRY

		square.draw();

	checkGLError();
}
void Renderer::destroy()
{
	shaders2D.destroy();
}

}
}