#version 330

// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

uniform float USE_LIGHTING;
uniform float USE_FACE_COLOUR;
uniform vec3 faceColour;

uniform vec4 ambientColour;
uniform vec4 diffuseColour;
uniform vec4 specularColour;
uniform float ambientReflection;
uniform float diffuseReflection;
uniform float specularReflection;
uniform float shininess;
uniform sampler2D texture;

in vec2 varyingTextureCoordinate;
in vec3 varyingNormal;
in vec3 varyingLightDirection;
in vec3 varyingViewerDirection;

out vec4 finalColour;

void main()
{
    if (USE_FACE_COLOUR > 0.5){
        finalColour = vec4(faceColour, 1.0);
    } 
    else if (USE_LIGHTING > 0.5) {
    	vec3 normal = normalize(varyingNormal);
        vec3 lightDirection = normalize(varyingLightDirection);
        vec3 viewerDirection = normalize(varyingViewerDirection);
        vec4 ambientIllumination = ambientReflection * ambientColour;
        vec4 diffuseIllumination = diffuseReflection * max(0.0, dot(lightDirection, normal)) * diffuseColour;
        vec4 specularIllumination = specularReflection * pow(max(0.0, 
                                                                 dot(-reflect(lightDirection, normal), viewerDirection)
                                                                 ), shininess) * specularColour;
        //finalColour = texture2D(texture, varyingTextureCoordinate);
        finalColour = texture2D(texture, varyingTextureCoordinate)*						(ambientIllumination + diffuseIllumination) + 							specularIllumination;
    }
    else if (USE_LIGHTING < 0.5){
        finalColour = texture2D(texture, varyingTextureCoordinate);
    }
}
