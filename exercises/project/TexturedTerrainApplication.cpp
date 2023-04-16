#include "TexturedTerrainApplication.h"

#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/texture/Texture2DObject.h>

#include <glm/gtx/transform.hpp>  // for matrix transformations

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <numbers>  // for PI constant

#include <imgui.h>

TexturedTerrainApplication::TexturedTerrainApplication()
    : Application(1024, 1024, "Textures demo")
    , m_gridX(300), m_gridY(300)
    , m_vertexShaderLoader(Shader::Type::VertexShader)
    , m_fragmentShaderLoader(Shader::Type::FragmentShader)
    , m_cameraPosition(-30, 35, -30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
{
}

void TexturedTerrainApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    // Build textures and keep them in a list
    InitializeTextures();

    // Build materials and keep them in a list
    InitializeMaterials();

    // Build meshes and keep them in a list
    InitializeMeshes();

    InitializeCamera();

    //Enable depth test
    GetDevice().EnableFeature(GL_DEPTH_TEST);

    //Enable wireframe
    GetDevice().SetWireframeEnabled(false);

    m_ambientColor = glm::vec3(0.3f);
    m_lightColor = glm::vec3(1.0f);
    m_lightIntensity = 1.5f;
    m_lightPosition = glm::vec3(-25.0f, 30.0f, 10.0f);
    m_specularExponent = 100.0f;
}

void TexturedTerrainApplication::Update()
{
    Application::Update();

    m_waterMaterial->SetUniformValue("ElapsedTime", GetCurrentTime());

    m_waterMaterial->SetUniformValue("AmbientColor", m_ambientColor);
    m_waterMaterial->SetUniformValue("LightColor", m_lightColor * m_lightIntensity);
    m_waterMaterial->SetUniformValue("SpecularExponent", m_specularExponent);
    m_waterMaterial->SetUniformValue("CameraPosition", m_cameraPosition);
    m_waterMaterial->SetUniformValue("LightDirection", m_lightPosition);

    UpdateCamera();

    std::cout << m_cameraPosition.x << " " << m_cameraPosition.y << " " << m_cameraPosition.z << std::endl;
}

void TexturedTerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(1.0f, 0.5f, 0.5f, 1.0f), true, 1.0f);

    // Water patches
    DrawObject(m_waterPatch, *m_waterMaterial, glm::translate(glm::vec3(0, -1.5, 0)) * glm::scale(glm::vec3(10.0f))); // bottom right
    DrawObject(m_waterPatch, *m_waterMaterial, glm::translate(glm::vec3(-10, -1.5, 0)) * glm::scale(glm::vec3(10.0f))); // bottom left
    DrawObject(m_waterPatch, *m_waterMaterial, glm::translate(glm::vec3(0, -1.5, -10)) * glm::scale(glm::vec3(10.0f))); // top right
    DrawObject(m_waterPatch, *m_waterMaterial, glm::translate(glm::vec3(-10, -1.5, -10)) * glm::scale(glm::vec3(10.0f))); // top left

    RenderGUI();
}

void TexturedTerrainApplication::InitializeTextures()
{
    m_waterTexture = LoadTexture("textures/water.png");
}

void TexturedTerrainApplication::InitializeMaterials()
{
    Shader waterVS = m_vertexShaderLoader.Load("shaders/water.vert");
    Shader waterFS = m_fragmentShaderLoader.Load("shaders/water.frag");
    std::shared_ptr<ShaderProgram> waterShaderProgram = std::make_shared<ShaderProgram>();
    waterShaderProgram->Build(waterVS, waterFS);
    m_waterMaterial = std::make_shared<Material>(waterShaderProgram);
    m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f, 1.0f, 1.0f, 0.4));
    m_waterMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.04));
    m_waterMaterial->SetUniformValue("ColorTexture", m_waterTexture);
    m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_waterMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);

    m_waterMaterial->SetUniformValue("AmbientReflection", 1.0f);
    m_waterMaterial->SetUniformValue("DiffuseReflection", 1.0f);
    m_waterMaterial->SetUniformValue("SpecularReflection", 0.6f);
}

void TexturedTerrainApplication::InitializeMeshes()
{
    CreateTerrainMesh(m_waterPatch, m_gridX, m_gridY);
}

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::LoadTexture(const char* path)
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 0;
    int height = 0;
    int components = 0;
    
    unsigned char* data = stbi_load(path, &width, &height, &components, 4);

    texture->Bind();
    texture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, std::span<const unsigned char>(data, width * height * 4));
    texture->GenerateMipmap();

    stbi_image_free(data);

    return texture;
}

void TexturedTerrainApplication::DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix)
{
    material.Use();

    ShaderProgram& shaderProgram = *material.GetShaderProgram();
    ShaderProgram::Location locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
    material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
    ShaderProgram::Location locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");
    material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

    mesh.DrawSubmesh(0);
}

void TexturedTerrainApplication::CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY)
{
    // Define the vertex structure
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 texCoord)
            : position(position), normal(normal), texCoord(texCoord) {}
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    // List of vertices (VBO)
    std::vector<Vertex> vertices;

    // List of indices (EBO)
    std::vector<unsigned int> indices;

    // Grid scale to convert the entire grid to size 1x1
    glm::vec2 scale(1.0f / (gridX - 1), 1.0f / (gridY - 1));

    // Number of columns and rows
    unsigned int columnCount = gridX;
    unsigned int rowCount = gridY;

    // Iterate over each VERTEX
    for (unsigned int j = 0; j < rowCount; ++j)
    {
        for (unsigned int i = 0; i < columnCount; ++i)
        {
            // Vertex data for this vertex only
            glm::vec3 position(i * scale.x, 0.0f, j * scale.y);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec2 texCoord(i, j);
            vertices.emplace_back(position, normal, texCoord);

            // Index data for quad formed by previous vertices and current
            if (i > 0 && j > 0)
            {
                unsigned int top_right = j * columnCount + i; // Current vertex
                unsigned int top_left = top_right - 1;
                unsigned int bottom_right = top_right - columnCount;
                unsigned int bottom_left = bottom_right - 1;

                //Triangle 1
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    mesh.AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}

void TexturedTerrainApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

void TexturedTerrainApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    ImGui::ColorEdit3("Ambient Color", &m_ambientColor[0]);
    ImGui::Separator();
    ImGui::DragFloat("Light Intensity", &m_lightIntensity);
    ImGui::ColorEdit3("Light Color", &m_lightColor[0]);
    ImGui::DragFloat3("Light Position", &m_lightPosition[0]);
    ImGui::Separator();
    ImGui::DragFloat("Specular Exponent", &m_specularExponent);

    m_imGui.EndFrame();
}

void TexturedTerrainApplication::UpdateCamera()
{
    Window& window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
    {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0, 1, 0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

    // Update view matrix
    m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
