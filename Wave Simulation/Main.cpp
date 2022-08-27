#define STB_IMAGE_IMPLEMENTATION

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <Camera.h>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <vendor/stb_image.h>
#include <imgui.h>

static unsigned int WINDOW_WIDTH  = 1920;
static unsigned int WINDOW_HEIGHT = 1080;
static float FOV = 45.0f;
static float NEAR_CLIP = 0.1f;
static float FAR_CLIP = 400.0f;

static const std::string RESOURCES_PATH = "C:/Users/lenna/OneDrive/Documents/C++ Projekte/Wave Simulation/Wave Simulation/resources/";

static float mouseSensitivity = 0.1f;

float lastX = (float)WINDOW_WIDTH / 2.0f;
float lastY = (float)WINDOW_HEIGHT / 2.0f;

glm::mat4 proj_mat;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool usingCursor = false;
bool pressedTab = false;
bool firstMouse = true;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static GLFWwindow* Init(int win_x, int win_y);
void setupCube(unsigned int* cubeVAO, unsigned int* cubeVBO, unsigned int* cubeEBO);
void setupWater(unsigned int* waterVAO, unsigned int* waterVBO, unsigned int* waterEBO, float xMin, float zMin, float xMax, float zMax, int waterRes);
void setupBowl(unsigned int* bowlVAO, unsigned int* bowlVBO, unsigned int* bowlEBO, float xMin, float zMin, float xMax, float zMax, int res);
void setupFullscreenQuad(unsigned int* screenVAO, unsigned int* screenVBO, unsigned int* screenEBO);

glm::vec4 wavePointPosition(float alpha, float beta, float gravity, float time, float waveParams[][4], float waveOctaves,
                            glm::vec4* tangent, glm::vec4* bitangent, glm::vec4* normal);
void load2DTexture(unsigned int* texture, const char* texturePath);

void setShaderUniformInt(unsigned int shaderProgram, const char* uniformName, int value);
void setShaderUniformFloatArray(unsigned int shaderProgram, const char* uniformName, unsigned int count, float* value);
void setShaderUniformFloat(unsigned int shaderProgram, const char* uniformName, float value);
void setShaderUniformVec2Array(unsigned int shaderProgram, const char* uniformName, const unsigned int count, float* value);
void setShaderUniformVec2(unsigned int shaderProgram, const char* uniformName, glm::vec2 vector);
void setShaderUniformVec3(unsigned int shaderProgram, const char* uniformName, float vector[3]);
void setShaderUniformVec3(unsigned int shaderProgram, const char* uniformName, glm::vec3 vector);
void setShaderUniformMat4(unsigned int shaderProgram, const char* uniformName, glm::mat4 matrix);
void compileShader(unsigned int* shaderProgram, const char* vertexPath, const char* fragmentPath);

int main()
{
    GLFWwindow* window = Init(WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetWindowSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup projection Matrix
    proj_mat = glm::perspective(glm::radians(FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);
    // -------------------

    // Setup fog
    float fogRadius  = 0.7f; // 0 - 1
    float fogDensity = 0.2f; // 0 - 1
    // -------------------

    // Setup lights
    glm::vec3 sunDir(-1.0f);

    // Setup cube
    unsigned int cubeVAO, cubeVBO, cubeEBO;
    setupCube(&cubeVAO, &cubeVBO, &cubeEBO);
    unsigned int cubeShaderProgram;
    compileShader(&cubeShaderProgram, (RESOURCES_PATH + "shaders/cubeShader.vert").c_str(),
                                      (RESOURCES_PATH + "shaders/cubeShader.frag").c_str());
    glm::mat4 cube_model_mat = glm::identity<glm::mat4>();
    glm::vec3 cubePosition(0.0f, 0.0f, 0.0f);
    setShaderUniformMat4(cubeShaderProgram, "model_mat", cube_model_mat);
    setShaderUniformMat4(cubeShaderProgram, "proj_mat", proj_mat);
    // -------------------

    // Setup water plane
    int waterRes  =    1000;
    float xMin    = -200.0f;
    float zMin    = -200.0f;
    float xMax    =  200.0f;
    float zMax    =  200.0f;

    float waterColor[] = { 0.0f, 0.435f, 0.62f };

    float gravity = 9.8f;
    const unsigned int waveOctaves = 6;
    float waveParams[waveOctaves][4] =
    {
        //WA    WP       WD
        {0.758f, -2.894f, 0.076f, 0.099f},
        {0.369f, 0.5f, 0.197f, 0.157f},
        {0.212f, -0.673f, -0.446f, -0.1f},
        {0.096f, 3.399f, -0.828f, -0.546f},
        {0.843f, 0.9f, 0.035f, 0.162f},
        {0.843f, 0.9f, 0.035f, 0.162f}
    };

    unsigned int waveParamsTex;
    glGenTextures(1, &waveParamsTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, waveParamsTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    unsigned int waterVAO, waterVBO, waterEBO;
    setupWater(&waterVAO, &waterVBO, &waterEBO, xMin, zMin, xMax, zMax, waterRes);
    unsigned int waterShaderProgram;
    compileShader(&waterShaderProgram, (RESOURCES_PATH + "shaders/waterShader.vert").c_str(),
                                       (RESOURCES_PATH + "shaders/waterShader.frag").c_str());
    setShaderUniformMat4(waterShaderProgram, "model_mat", glm::identity<glm::mat4>());
    setShaderUniformMat4(waterShaderProgram, "proj_mat", proj_mat);
    setShaderUniformFloat(waterShaderProgram, "fogRadius", fogRadius);
    setShaderUniformFloat(waterShaderProgram, "fogDensity", fogDensity);
    setShaderUniformFloat(waterShaderProgram, "NEAR", NEAR_CLIP);
    setShaderUniformFloat(waterShaderProgram, "FAR", FAR_CLIP);
    setShaderUniformVec3(waterShaderProgram, "sunDir", sunDir);
    setShaderUniformVec2(waterShaderProgram, "tileSize", (1.0f / (float)waterRes) * glm::vec2(xMax - xMin, zMax - zMin));

    setShaderUniformInt(waterShaderProgram, "backgroundTexture", 0);
    setShaderUniformInt(waterShaderProgram, "waveParamsTex", 1);
    // -------------------

    // Setup bowl
    int bowlRes  =     200;
    xMin         = -300.0f;
    zMin         = -300.0f;
    xMax         =  300.0f;
    zMax         =  300.0f;

    unsigned int bowlVAO, bowlVBO, bowlEBO;
    setupBowl(&bowlVAO, &bowlVBO, &bowlEBO, xMin, zMin, xMax, zMax, bowlRes);
    unsigned int bowlShaderProgram;
    compileShader(&bowlShaderProgram, (RESOURCES_PATH + "shaders/bowlShader.vert").c_str(),
                                      (RESOURCES_PATH + "shaders/bowlShader.frag").c_str());
    setShaderUniformMat4(bowlShaderProgram, "model_mat", glm::identity<glm::mat4>());
    setShaderUniformMat4(bowlShaderProgram, "proj_mat", proj_mat);
    setShaderUniformFloat(bowlShaderProgram, "fogRadius", fogRadius);
    setShaderUniformFloat(bowlShaderProgram, "fogDensity", fogDensity);
    setShaderUniformFloat(bowlShaderProgram, "NEAR", NEAR_CLIP);
    setShaderUniformFloat(bowlShaderProgram, "FAR", FAR_CLIP);
    setShaderUniformVec3(bowlShaderProgram, "sunDir", sunDir);

    setShaderUniformInt(bowlShaderProgram, "backgroundTexture", 0);
    setShaderUniformInt(bowlShaderProgram, "grassTexture", 1);
    unsigned int grassTexture;
    load2DTexture(&grassTexture, "grass.jpg");

    // -------------------
    
    // Setup fullscreen plane
    unsigned int screenVAO, screenVBO, screenEBO;
    setupFullscreenQuad(&screenVAO, &screenVBO, &screenEBO);

    unsigned int backgroundShaderProgram;
    compileShader(&backgroundShaderProgram, (RESOURCES_PATH + "shaders/background.vert").c_str(),
                                            (RESOURCES_PATH + "shaders/background.frag").c_str());
    setShaderUniformMat4(backgroundShaderProgram, "proj_mat", proj_mat);
    setShaderUniformVec2(backgroundShaderProgram, "res", glm::vec2((float)WINDOW_WIDTH,(float)WINDOW_HEIGHT));
    setShaderUniformVec3(backgroundShaderProgram, "skyColor", glm::vec3(0.35f, 0.65f, 0.97f));
    setShaderUniformVec3(backgroundShaderProgram, "fogColor", glm::vec3(0.8f, 0.8f, 0.8f));
    setShaderUniformVec3(backgroundShaderProgram, "sunColor", glm::vec3(1.0f, 1.0f, 0.0f));
    setShaderUniformVec3(backgroundShaderProgram, "sunPos", glm::vec3(1.0f, 0.3f, 1.0f));

    // FBO
    unsigned int backgroundFBO;
    glGenFramebuffers(1, &backgroundFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
    unsigned int backgroundTexture;
    glGenTextures(1, &backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // -------------------

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (!usingCursor)
            camera.update(window, deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw Fullscreen Quad
        glUseProgram(backgroundShaderProgram);
        setShaderUniformMat4(backgroundShaderProgram, "view_mat", camera.getViewMatrix());
        setShaderUniformVec3(backgroundShaderProgram, "viewPos", camera.getCameraPos());
        glBindVertexArray(screenVAO);

        // Draw to background framebuffer
        glDisable(GL_DEPTH);
        glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // Draw to scene
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glEnable(GL_DEPTH);

        //// Render cube
        cube_model_mat[3] = wavePointPosition(cubePosition.x, cubePosition.z, gravity, currentFrame, waveParams, waveOctaves, // NULL, NULL, NULL);
                                              &cube_model_mat[2], &cube_model_mat[0], &cube_model_mat[1]);

        glUseProgram(cubeShaderProgram);
        setShaderUniformMat4(cubeShaderProgram, "model_mat", cube_model_mat);
        setShaderUniformMat4(cubeShaderProgram, "view_mat", camera.getViewMatrix());
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render bowl
        //glUseProgram(bowlShaderProgram);
        //setShaderUniformMat4(bowlShaderProgram, "view_mat", camera.getViewMatrix());
        //setShaderUniformFloat(bowlShaderProgram, "fogRadius", fogRadius);
        //setShaderUniformFloat(bowlShaderProgram, "fogDensity", fogDensity);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, grassTexture);
        //glBindVertexArray(bowlVAO);
        //glDrawElements(GL_TRIANGLES, 6 * (bowlRes + 1) * (bowlRes + 1), GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);

        // Render water
        glUseProgram(waterShaderProgram);
        setShaderUniformMat4(waterShaderProgram, "view_mat", camera.getViewMatrix());
        setShaderUniformFloat(waterShaderProgram, "time", currentFrame);
        setShaderUniformFloat(waterShaderProgram, "fogRadius", fogRadius);
        setShaderUniformFloat(waterShaderProgram, "fogDensity", fogDensity);
        setShaderUniformFloat(waterShaderProgram, "g", gravity);
        setShaderUniformVec3(waterShaderProgram, "waterColor", waterColor);

        // Set water Shader parameters
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, waveParamsTex);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16F, waveOctaves, 0, GL_RGBA, GL_FLOAT, &waveParams[0]);
        //setShaderUniformFloatArray(waterShaderProgram, "amplitudes", waveOctaves, &waveAmplitudes[0]);
        //setShaderUniformFloatArray(waterShaderProgram, "phases", waveOctaves, &wavePhases[0]);
        //setShaderUniformVec2Array(waterShaderProgram, "directions", waveOctaves, &waveDirections[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBindVertexArray(waterVAO);
        glDrawElements(GL_TRIANGLES, 6 * (waterRes + 1) * (waterRes + 1), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // UI
        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();
        //ImGui::Begin("Fog Options");
        //ImGui::SliderFloat("Fog Density", &fogDensity, 0.0f, 1.0f);
        //ImGui::SliderFloat("Fog Radius", &fogRadius, 0.0f, 1.0f);
        //ImGui::End();
        //ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (usingCursor)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Water Options");
            ImGui::SetWindowSize(ImVec2(400, ImGui::GetWindowHeight()));
            ImGui::SliderFloat("Gravity", &gravity, -2.0f, 10.0f);
            ImGui::ColorEdit3("Water Color", &waterColor[0]);
            for (int i = 0; i < waveOctaves; i++)
            {
                std::string index = std::to_string(i);
                ImGui::Text(("Octave " + index).c_str());
                ImGui::SliderFloat(("Amplitude " + index).c_str(), &waveParams[i][0], 0.0f, 1.0f);
                ImGui::SliderFloat(("Phase " + index).c_str(), &waveParams[i][1], -7.0f, 7.0f);
                ImGui::SliderFloat(("Direction X " + index).c_str(), &waveParams[i][2], -1.0f, 1.0f);
                ImGui::SliderFloat(("Direction Z " + index).c_str(), &waveParams[i][3], -1.0f, 1.0f);
            }
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
glm::vec4 wavePointPosition(float alpha, float beta, float gravity, float time, float waveParams[][4], float waveOctaves, 
                            glm::vec4* tangent, glm::vec4* bitangent, glm::vec4* normal)
{
    glm::vec3 position(0.0f);
    for (int i = 0; i < waveOctaves; i++)
    {
        float k = glm::length(glm::vec2(waveParams[i][2], waveParams[i][3]));
        float omega = sqrt(gravity * k); // For deep water
        //float omega = sqrt(g * k * tanh(k * meanDepth)); // For shallow water
        float delta = waveParams[i][2] * alpha + waveParams[i][3] * beta - omega * time - waveParams[i][1];

        position.x += (waveParams[i][2] / k) * (waveParams[i][0] /* / tanh(k * meanDepth) */) * sin(delta);
        position.z += (waveParams[i][3] / k) * (waveParams[i][0] /* / tanh(k * meanDepth) */) * sin(delta);
        position.y += waveParams[i][0] * cos(delta);
    }
    position = glm::vec3(alpha - position.x, position.y, beta - position.z);
    if (normal != NULL && tangent != NULL && bitangent != NULL)
    {
        float eps = 0.0001f;
        glm::vec3 deltaAlpha = glm::vec3(wavePointPosition(alpha + eps, beta, gravity, time, waveParams, waveOctaves, NULL, NULL, NULL)) - position;
        *tangent = glm::vec4(glm::normalize(deltaAlpha), 0.0f);

        glm::vec3 deltaBeta = glm::vec3(wavePointPosition(alpha, beta + eps, gravity, time, waveParams, waveOctaves, NULL, NULL, NULL)) - position;
        *bitangent = glm::vec4(glm::normalize(deltaBeta), 0.0f);

        *normal = glm::vec4(glm::normalize(glm::cross(glm::vec3(*tangent), glm::vec3(*bitangent))), 0.0f);

        *bitangent = glm::vec4(glm::normalize(glm::cross(glm::vec3(*tangent), glm::vec3(*normal))), 0.0f); // Recalculate bitangent because algorithm also changes the x and
                                                                                                           // z coordinates, so orthonormality isnt given.
    }
    return glm::vec4(position, 1.0f);
}

void load2DTexture(unsigned int* texture, const char* texturePath)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load((RESOURCES_PATH + texturePath).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}
void setShaderUniformInt(unsigned int shaderProgram, const char* uniformName, int value)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1i(uniformLoc, value);
}
void setShaderUniformFloatArray(unsigned int shaderProgram, const char* uniformName, const unsigned int count, float* value)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1fv(uniformLoc, count, value);
}
void setShaderUniformFloat(unsigned int shaderProgram, const char* uniformName, float value)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1f(uniformLoc, value);
}
void setShaderUniformVec2Array(unsigned int shaderProgram, const char* uniformName, const unsigned int count, float* value)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform2fv(uniformLoc, count, value);
}
void setShaderUniformVec2(unsigned int shaderProgram, const char* uniformName, glm::vec2 vector)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform2f(uniformLoc, vector.x, vector.y);
}
void setShaderUniformVec3(unsigned int shaderProgram, const char* uniformName, float vector[3])
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform3f(uniformLoc, vector[0], vector[1], vector[2]);
}
void setShaderUniformVec3(unsigned int shaderProgram, const char* uniformName, glm::vec3 vector)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform3f(uniformLoc, vector.x, vector.y, vector.z);
}
void setShaderUniformMat4(unsigned int shaderProgram, const char* uniformName, glm::mat4 matrix)
{
    glUseProgram(shaderProgram);
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &matrix[0][0]);
}
void compileShader(unsigned int* shaderProgram, const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    // shader Program
    *shaderProgram = glCreateProgram();
    glAttachShader(*shaderProgram, vertex);
    glAttachShader(*shaderProgram, fragment);
    glLinkProgram(*shaderProgram);
    // print linking errors if any
    glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
void setupCube(unsigned int* cubeVAO, unsigned int* cubeVBO, unsigned int* cubeEBO)
{
    float vertices[] = {
    //       Positions
        -0.5f, -0.5f,  0.5f, //0
         0.5f, -0.5f,  0.5f, //1
        -0.5f,  0.5f,  0.5f, //2
         0.5f,  0.5f,  0.5f, //3
        -0.5f, -0.5f, -0.5f, //4
         0.5f, -0.5f, -0.5f, //5
        -0.5f,  0.5f, -0.5f, //6
         0.5f,  0.5f, -0.5f  //7 
    };
    unsigned int indices[] = {
        //Top
        2, 6, 7,
        2, 3, 7,

        //Bottom
        0, 4, 5,
        0, 1, 5,

        //Left
        0, 2, 6,
        0, 4, 6,

        //Right
        1, 3, 7,
        1, 5, 7,

        //Front
        0, 2, 3,
        0, 1, 3,

        //Back
        4, 6, 7,
        4, 5, 7
    };
    glGenVertexArrays(1, cubeVAO);
    glGenBuffers(1, cubeVBO);
    glGenBuffers(1, cubeEBO);

    glBindVertexArray(*cubeVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, *cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void setupFullscreenQuad(unsigned int* screenVAO, unsigned int* screenVBO, unsigned int* screenEBO)
{
    float vertices[] = {
         1.0f,  1.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f  // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,                // first Triangle
        1, 2, 3                 // second Triangle
    };
    glGenVertexArrays(1, screenVAO);
    glGenBuffers(1, screenVBO);
    glGenBuffers(1, screenEBO);

    glBindVertexArray(*screenVAO);

    glBindBuffer(GL_ARRAY_BUFFER, *screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *screenEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void setupWater(unsigned int* waterVAO, unsigned int* waterVBO, unsigned int* waterEBO, float xMin, float zMin, float xMax, float zMax, int waterRes) {
    float stepSizeX = (xMax - xMin) / (float)waterRes;
    float stepSizeZ = (zMax - zMin) / (float)waterRes;

    std::vector<float> waterVertices;
    for (int z = 0; z < waterRes; z++)
    {
        for (int x = 0; x < waterRes; x++)
        {
            // Position
            waterVertices.push_back(xMin + x * stepSizeX);
            waterVertices.push_back(0);
            waterVertices.push_back(zMin + z * stepSizeZ);
            // Normal
            //waterVertices.push_back(0);
            //waterVertices.push_back(1);
            //waterVertices.push_back(0);
        }
    }
    std::vector<unsigned int> waterIndices;
    for (int z = 0; z < waterRes - 1; z++)
    {
        for (int x = 0; x < waterRes - 1; x++)
        {
            int i = x + (z * waterRes);
            waterIndices.push_back(i);
            waterIndices.push_back(i + waterRes);
            waterIndices.push_back(i + waterRes + 1);

            waterIndices.push_back(i);
            waterIndices.push_back(i + waterRes + 1);
            waterIndices.push_back(i + 1);
        }
    }

    glGenVertexArrays(1, waterVAO);
    glGenBuffers(1, waterVBO);
    glGenBuffers(1, waterEBO);

    glBindVertexArray(*waterVAO);

    glBindBuffer(GL_ARRAY_BUFFER, *waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (waterRes + 1) * (waterRes + 1) * 3, &waterVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * waterRes * waterRes * 6, &waterIndices[0], GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}
float gaussianRandom(float x, float z)
{
    float d = glm::length(glm::vec2(x, z));
    return (3.0f + -10.0f * glm::exp(-0.001f * d * d)) + 0.2f * glm::perlin<float>(glm::vec2(0.1f * x, 0.1f * z));
}

void setupBowl(unsigned int* bowlVAO, unsigned int* bowlVBO, unsigned int* bowlEBO, float xMin, float zMin, float xMax, float zMax, int res) {
    float stepSizeX = (xMax - xMin) / (float)res;
    float stepSizeZ = (zMax - zMin) / (float)res;

    std::vector<float> bowlVertices;
    for (int z = 0; z < res; z++)
    {
        for (int x = 0; x < res; x++)
        {
            float xCoord = xMin + x * stepSizeX;
            float zCoord = zMin + z * stepSizeZ;
            float depth = gaussianRandom(xCoord, zCoord);
            // Position
            bowlVertices.push_back(xCoord);
            bowlVertices.push_back(depth);
            bowlVertices.push_back(zCoord);

            glm::vec3 u = glm::vec3(xCoord + stepSizeX, gaussianRandom(xCoord + stepSizeX, zCoord), zCoord) - glm::vec3(xCoord, depth, zCoord);
            glm::vec3 v = glm::vec3(xCoord, gaussianRandom(xCoord, zCoord + stepSizeZ), zCoord + stepSizeZ) - glm::vec3(xCoord, depth, zCoord);
            glm::vec3 normal = glm::normalize(glm::cross(u, v));

            // Normal
            bowlVertices.push_back(normal.x);
            bowlVertices.push_back(normal.y);
            bowlVertices.push_back(normal.z);

            // TexCoords
            bowlVertices.push_back(x / (float)res);
            bowlVertices.push_back(z / (float)res);
        }
    }
    std::vector<unsigned int> bowlIndices;
    for (int z = 0; z < res - 1; z++)
    {
        for (int x = 0; x < res - 1; x++)
        {
            int i = x + (z * res);
            bowlIndices.push_back(i);
            bowlIndices.push_back(i + res);
            bowlIndices.push_back(i + res + 1);

            bowlIndices.push_back(i);
            bowlIndices.push_back(i + res + 1);
            bowlIndices.push_back(i + 1);
        }
    }

    glGenVertexArrays(1, bowlVAO);
    glGenBuffers(1, bowlVBO);
    glGenBuffers(1, bowlEBO);

    glBindVertexArray(*bowlVAO);

    glBindBuffer(GL_ARRAY_BUFFER, *bowlVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (res + 1) * (res + 1) * 8, &bowlVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *bowlEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * res * res * 6, &bowlIndices[0], GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER,0);

    glBindVertexArray(0);
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!pressedTab)
        {
            if (usingCursor)
            {
                usingCursor = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                usingCursor = true;
                firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            pressedTab = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
        pressedTab = false;

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.cameraSpeed += yoffset * 0.8f;
    if (camera.cameraSpeed <= 0.0f)
        camera.cameraSpeed = 0.0f;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (usingCursor) return;
    if (firstMouse) // initially set to true
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    camera.updateRotations(xOffset, yOffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    proj_mat = glm::perspective(glm::radians(FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);
    lastX = (float)WINDOW_WIDTH / 2.0f;
    lastY = (float)WINDOW_HEIGHT / 2.0f;
}
static GLFWwindow* Init(int win_x, int win_y)
{
    // Initialize OpenGL environment
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Setup and test window
    GLFWwindow* window = glfwCreateWindow(win_x, win_y, "Wave Simulation", NULL, NULL);
    //GLFWwindow* window = glfwCreateWindow(win_x, win_y, "Wave Simulation", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    // Setup and test GLAD
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    // Setup Viewport
    glViewport(0, 0, win_x, win_y);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}