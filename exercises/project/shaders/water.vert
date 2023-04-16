#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

uniform float ElapsedTime;

struct GerstnerWave
{
	vec2 direction;
	float amplitude;
	float steepness;
	float speed;
	float frequency;
};

// Based on https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
void gerstner_wave(vec2 position, inout vec3 wave_position, GerstnerWave wave, inout vec3 wave_normal, float time) 
{
	float phase_const = wave.frequency * wave.speed * time;

	float alpha = dot(position, wave.direction) * wave.frequency + phase_const;
	float QA = wave.steepness * wave.amplitude;

	float height = wave.amplitude * sin(alpha);
	float width = QA * cos(alpha);

	wave_position.x += width * wave.direction.x;
	wave_position.y += height;
	wave_position.z += width * wave.direction.y;

	// Compute normal vector
	float beta = wave.frequency * dot(wave_position.xz, wave.direction) + phase_const; 
	float WA = wave.frequency * wave.amplitude;

	float height_normal = wave.steepness * WA * sin(beta);
	float width_normal = WA * cos(beta);

	wave_normal.x -= width_normal * wave.direction.x;
	wave_normal.y -= height_normal;
	wave_normal.z -= width_normal * wave.direction.y;
}

void main()
{
	TexCoord = VertexTexCoord;
	WorldPosition = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;

	// Define n Gerstner waves based on their direction, amplitude, steepness, speed and frequency
	GerstnerWave waves[4] = GerstnerWave[4]
	(
		GerstnerWave(vec2(-1, -1), 0.4, 0.2, 3, 0.8),
		GerstnerWave(vec2(-1, -0.6), 0.5, 0.3, 3, 0.6),
		GerstnerWave(vec2(-1, -1.3), 0.6, 0.2, 3, 0.8),
		GerstnerWave(vec2(-1, -1.6), 0.7, 0.2, 3, 0.6)
	);

	// y component set to 1 to account for the 1 - ... in the formula for N
	vec3 wave_normal = vec3(0, 1, 0);

	// y component used to represent the height of the waves
	vec3 wave_position = vec3(WorldPosition.x, 0, WorldPosition.z);

	for(int i = 0; i < waves.length; i++) 
	{
		gerstner_wave(WorldPosition.xz, wave_position, waves[i], wave_normal, ElapsedTime);
	}

	WorldPosition += wave_position;
	WorldNormal = (WorldMatrix * vec4(wave_normal, 0.0)).xyz;

	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
