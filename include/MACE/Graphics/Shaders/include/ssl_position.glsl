R"(

#include <ssl_window>

#ifndef SSL_POSITION_LIBRARY
#define SSL_POSITION_LIBRARY

layout(location = 15) in vec3 ssl_VertexPosition;

layout(std140) uniform ssl_EntityData{
	vec3 ssl_Translation;
	float ssl_StretchX;
	vec3 ssl_Scale;
	float ssl_StretchY;
	vec3 ssl_InheritedTranslation;
	vec3 ssl_InheritedScale;
	mat4 ssl_Rotation;
	mat4 ssl_InheritedRotation;
	
};

vec4 sslGetEntityPosition(){
	//we do ssl_Scale first, then rotate, then translate.
	vec4 ssl_Position = (vec4(ssl_Translation,1.0)+(ssl_Rotation * (vec4(ssl_Scale*ssl_VertexPosition,0.0))));//MAD operations right here!
	
	ssl_Position.xy -= 0.5f;//because we moved the origin to be the bottom left, we need to move it back to the center for ssl_Rotation
	
	//applying the parent properties now
	ssl_Position = (vec4(ssl_InheritedTranslation,1.0)+((ssl_InheritedRotation * (ssl_Position * vec4(ssl_InheritedScale,0.0)))));//what a MAD man!
		
	ssl_Position = ((ssl_Position)*vec4(2.0f,2.0f,1.0f,1.0f))+vec4(-0.5f,-0.5f,0,0);//we need to convert it to be 0.0 to 1.0 coordinates and the origin to be the bottom left.
	
	ssl_Position += vec4(ssl_Scale/2,0.0f);
	
	vec2 ssl_SizeModifier = ssl_OriginalSize/ssl_CurrentSize;
	
	if(ssl_StretchX==0){
		ssl_Position.x*=ssl_SizeModifier.x;
	}
	if(ssl_StretchY==0){
		ssl_Position.y*=ssl_SizeModifier.y;
	}
	
	/*
	For those who didn't get the MAD joke:
	MAD means multipy, add, then divide. Doing math in that sequence will be done in 1 gpu cycle instead of 3, which saves a lot of processing power.  Thats what we are doing up there.
	*/
	
	return ssl_Position;
}

#endif

)"