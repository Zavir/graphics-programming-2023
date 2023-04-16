#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <glm/mat4x4.hpp>
#include <vector>
#include <ituGL/utils/DearImGui.h>

class Texture2DObject;

class TexturedTerrainApplication : public Application
{
public:
    TexturedTerrainApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;

private:
    struct GerstnerWave {
        glm::ivec2 direction;
        float amplitude;
        float steepness;
        float speed;
        float frequency;
    };

    void InitializeTextures();
    void InitializeMaterials();
    void InitializeMeshes();
    void InitializeCamera();

    void DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix);

    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY);

    void UpdateCamera();

    void RenderGUI();

private:
    unsigned int m_gridX, m_gridY;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Mesh m_waterPatch;

    std::shared_ptr<Material> m_waterMaterial;
    std::shared_ptr<Texture2DObject> m_waterTexture;

    glm::vec2 m_mousePosition;

    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    DearImGui m_imGui;

    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;
    float m_specularExponent;
};
