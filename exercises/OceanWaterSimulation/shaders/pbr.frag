//Inputs
in vec3 WorldPosition;
in vec3 WorldNormal;
in vec3 WorldTangent;
in vec3 WorldBitangent;
in vec2 TexCoord;
in vec3 WavePosition;

//Outputs
out vec4 FragColor;

//Uniforms
uniform vec3 Color;
uniform sampler2D NormalTexture;

uniform vec3 CameraPosition;

void main()
{
	// Use three different wave speeds and directions for calculating offsets
	mat3x2 offsets;
	offsets[0] = 4.0 * vec2(-1, 0);
	offsets[1] = 4.0 * vec2(1, -1.3);
	offsets[2] = 2.0 * vec2(-1, -1.1);

	vec3 normal = vec3(0, 0, 0);

	// Sample the same normal map three times and blend them
	for(int i = 0; i < 3; i++) 
	{
		normal += SampleNormalMap(NormalTexture, offsets[i] + TexCoord, normalize(WorldNormal), normalize(WorldTangent), normalize(WorldBitangent));
	}

	normal = normalize(normal);

	SurfaceData data;
	data.normal = normal;
	data.albedo = Color * 0.4; // Darker albedo looks better
	data.ambientOcclusion = min(1 + WavePosition.y * 0.65, 1); // Occlusion in the lowest part of the wave with scale 0.65
	data.roughness = 0.08f;
	data.metalness = 0.0f; // Metalness excluded from surface data

	vec3 position = WorldPosition;
	vec3 viewDir = GetDirection(position, CameraPosition);
	vec3 color = ComputeLighting(position, data, viewDir, true);
	FragColor = vec4(color.rgb, 1.0);
}