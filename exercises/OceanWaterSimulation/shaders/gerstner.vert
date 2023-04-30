layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec3 VertexBitangent;
layout (location = 4) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec3 WorldTangent;
out vec3 WorldBitangent;
out vec2 TexCoord;
out vec3 WavePosition;

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
void gerstner_wave(vec2 grid_position, inout vec3 wave_position, GerstnerWave wave, inout vec3 wave_tangent, inout vec3 wave_bitangent, float time) 
{
	float phase_const = wave.speed * wave.frequency * time;

	float alpha = dot(grid_position, wave.direction) * wave.frequency + phase_const;
	float QA = wave.steepness * wave.amplitude;

	float height = wave.amplitude * sin(alpha);
	float width = QA * cos(alpha);

	// Compute wave position
	wave_position.x += width * wave.direction.x;
	wave_position.y += height;
	wave_position.z += width * wave.direction.y;

	float beta = wave.frequency * dot(wave_position.xz, wave.direction) + phase_const; 
	float WA = wave.frequency * wave.amplitude;

	// Compute tangent vector
	wave_tangent.x -= wave.steepness * wave.direction.x * wave.direction.y * WA * sin(beta);
	wave_tangent.y += wave.direction.y * WA * cos(beta);
	wave_tangent.z -= wave.steepness * pow(wave.direction.y, 2.0) * WA * sin(beta);

	// Compute bitangent vector
	wave_bitangent.x -= wave.steepness * pow(wave.direction.x, 2.0) * WA * sin(beta);
	wave_bitangent.y += wave.direction.x * WA * cos(beta);
	wave_bitangent.z -= wave.steepness * wave.direction.x * wave.direction.y * WA * sin(beta); 
}

void main()
{
	const float scale = 50.0f;

	TexCoord = VertexTexCoord * scale;
	WorldPosition = (WorldMatrix * vec4(VertexPosition * scale, 1.0)).xyz;

	// Define n = 6 Gerstner waves based on their direction, amplitude, steepness, speed and frequency
	GerstnerWave waves[6] = GerstnerWave[6]
	(
		GerstnerWave(vec2(-1, 0), 0.6, 0.2, 4, 0.6),
		GerstnerWave(vec2(1, -0.6), 0.6, 0.3, 5, 0.5),
		GerstnerWave(vec2(1, -1.3), 0.5, 0.3, 4, 0.6),
		GerstnerWave(vec2(-1, -1.6), 0.6, 0.2, 2, 0.4),
		GerstnerWave(vec2(-1, -1.1), 0.6, 0.3, 2, 0.4),
		GerstnerWave(vec2(1, -1.1), 0.6, 0.3, 3, 0.4)
	);

	// z and x component set to 1 to account for the 1 - ... in the formula for N
	vec3 wave_tangent = vec3(0, 0, 1);
	vec3 wave_bitangent = vec3(1, 0, 0);

	// y component used to represent the height of the waves
	vec3 wave_position = vec3(WorldPosition.x, 0, WorldPosition.z);

	for(int i = 0; i < waves.length; i++) 
	{
		gerstner_wave(WorldPosition.xz, wave_position, waves[i], wave_tangent, wave_bitangent, ElapsedTime);
	}

	WorldPosition += wave_position;

	vec3 wave_normal = cross(wave_tangent, wave_bitangent);

	WorldNormal = (WorldMatrix * vec4(wave_normal, 0.0)).xyz;
	WorldTangent = (WorldMatrix * vec4(wave_tangent, 0.0)).xyz;
	WorldBitangent = (WorldMatrix * vec4(wave_bitangent, 0.0)).xyz;

	WavePosition = wave_position;

	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
