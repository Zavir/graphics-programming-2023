#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;

out vec4 FragColor;

uniform vec4 Color;
uniform vec2 ColorTextureScale;

uniform vec2 Range_01;
uniform vec2 Range_02;
uniform vec2 Range_03;

uniform sampler2D GrassTexture;
uniform sampler2D SnowTexture;
uniform sampler2D DirtTexture;
uniform sampler2D RockTexture;

float interpolate(vec2 range, float height){
	return clamp((height - range.x) / (range.y - range.x), 0, 1);
}

void main()
{
	vec4 color0 = texture(DirtTexture, TexCoord * ColorTextureScale);
	vec4 color1 = texture(GrassTexture, TexCoord * ColorTextureScale);
	vec4 color2 = texture(RockTexture, TexCoord * ColorTextureScale);
	vec4 color3 = texture(SnowTexture, TexCoord * ColorTextureScale);

	vec4 color = color0;
	color = mix(color, color1, interpolate(Range_01, Height));
	color = mix(color, color2, interpolate(Range_02, Height));
	color = mix(color, color3, interpolate(Range_03, Height));

	FragColor = Color * color;
}
