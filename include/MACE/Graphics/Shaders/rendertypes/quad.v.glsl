//when the preprocessor copy and pastes this file, the newlines will be syntax errors. we need to specify that this is a multiline string. if you want syntax highlighting, make sure to configure your editor to ignore this line
R""(
//VERTEX SHADER

#include <mc_core>
#include <mc_vertex>

precision mediump float; // Defines precision for float and float-derived (vector/matrix) types.

vec4 mc_vert_main(vec4 pos){	
	return pos;
}

)""
