#version 330 

// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

uniform float DRAW_LASER;
in vec3 vert;
in vec3 normal;
in vec2 textureCoordinate;
in vec3 laser;

out vec2 varyingTextureCoordinate;
out vec3 varyingNormal;
out vec3 varyingLightDirection;
out vec3 varyingViewerDirection;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;
uniform mat4 viewMatrix;
uniform mat4 perspMatrix;

void main()
{	
    vec4 vertex;
	if (DRAW_LASER > 0.5){
		vec4 vertex = vec4(laser, 1.0);
	    varyingTextureCoordinate = textureCoordinate;

	    vec4 eyeVertex = mvMatrix * vertex;
	    eyeVertex /= eyeVertex.w;
	    varyingNormal = normalMatrix * normal;
	    varyingLightDirection = lightPosition - eyeVertex.xyz;
	    varyingViewerDirection = -eyeVertex.xyz;
	    gl_Position = mvpMatrix * vertex;

	}
	else {
		vertex = vec4(vert, 1.0);
	    varyingTextureCoordinate = textureCoordinate;

	    vec4 eyeVertex = mvMatrix * vertex;
	    eyeVertex /= eyeVertex.w;
	    varyingNormal = normalMatrix * normal;
	    varyingLightDirection = lightPosition - eyeVertex.xyz;
	    varyingViewerDirection = -eyeVertex.xyz;
	    gl_Position = mvpMatrix * vertex;
	}
}
