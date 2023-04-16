#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture;
uniform vec2 ColorTextureScale;

uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform vec3 LightDirection;

vec3 GetSpecularReflection(vec3 lightVector, vec3 viewVector, vec3 normalVector) {
	vec3 halfVector = normalize(lightVector + viewVector);
	return SpecularReflection * LightColor * pow(max(dot(halfVector, normalVector), 0.0f), SpecularExponent);
}

vec3 GetDiffuseReflection(vec3 lightVector, vec3 normalVector, vec3 objectColor) {
	return DiffuseReflection * LightColor * objectColor *  max(dot(lightVector, normalVector), 0.0f);
}

vec3 GetAmbientReflection(vec3 objectColor) 
{
	return AmbientColor * AmbientReflection * objectColor;
}

vec3 GetBlinnPhongReflection(vec3 lightVector, vec3 normalVector, vec3 viewVector, vec3 objectColor) 
{
	return GetAmbientReflection(objectColor) + GetDiffuseReflection(lightVector, normalVector, objectColor) + GetSpecularReflection(lightVector, viewVector, normalVector);
}

void main()
{
	vec4 objectColor = Color * texture(ColorTexture, TexCoord * ColorTextureScale);
	vec3 viewVector = normalize(CameraPosition - WorldPosition);
	vec3 lightVector = normalize(LightDirection);
	vec3 normalVector = normalize(WorldNormal);

	FragColor = vec4(GetBlinnPhongReflection(lightVector, normalVector, viewVector, objectColor.rgb), 1.0f);
}
