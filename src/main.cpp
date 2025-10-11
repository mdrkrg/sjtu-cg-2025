#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.hpp"
#include "shader.h"
#include "camera.h"
#include "texture.hpp"

#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // !STB_IMAGE_IMPLEMENTATION

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// 窗口设置
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// 摄像机设置
Camera camera(glm::vec3(0.0f, 0.3f, 3.3f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间设置
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 光照设置
glm::vec3 lightPos(0.0f, 0.75f, 1.65f);
glm::vec3 cubePos(0.0f, 0.3f, 2.0f);
glm::vec3 tablePos(0.0f, -0.2f, 2.0f);

int main() {
  // 初始化并配置glfw
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  stbi_set_flip_vertically_on_load(true);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw创建窗口
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // 告诉 GLFW 捕获我们的鼠标
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad：加载所有 OpenGL 函数指针
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // 配置全局 OpenGL 状态
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // 编译shader操作
  // ------------------------------------
  Shader lightingShader("shaders/lighting.vs.glsl", "shaders/lighting.fs.glsl");
  Shader modelShader("shaders/model.vs.glsl", "shaders/model.fs.glsl");
  Shader lightCubeShader("shaders/lightcube.vs.glsl",
                         "shaders/lightcube.fs.glsl");
  Shader windowShader("shaders/window.vs.glsl", "shaders/window.fs.glsl");

  unsigned int windowDiffuseMap = loadTexture(
      std::filesystem::path("resources/textures/window.png").c_str());

  // 统一设置用到的坐标信息(每一行前三个数字为点的坐标，后三个为法向量)
  // ------------------------------------------------------------------
  // WARN: normal dir
  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, //

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //

      -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //
      -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //
      -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //

      0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //

      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //

      -0.5f, 0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 1.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
  };

  // 获取各个平面的数据
  //  ------------------------------------------------------------------
  float CeilingVertices[48];
  std::copy(vertices + 240, vertices + 288, CeilingVertices);
  float FloorVertices[48];
  std::copy(vertices + 192, vertices + 240, FloorVertices);
  float RWallVertices[48];
  std::copy(vertices + 144, vertices + 192, RWallVertices);
  float LWallVertices[48];
  std::copy(vertices + 96, vertices + 144, LWallVertices);
  float FWallVertices[48];
  std::copy(vertices + 0, vertices + 48, FWallVertices);

  // 载入天花板的顶点信息
  // ------------------------------------------------------------------
  unsigned int VBO1, CeilingVAO;
  {
    glGenVertexArrays(1, &CeilingVAO);
    glGenBuffers(1, &VBO1);

    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CeilingVertices), CeilingVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(CeilingVAO);

    // 载入位置
    // xyz
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    // 载入法向量
    // rgb
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  // 载入地板的顶点信息
  //  ------------------------------------------------------------------
  unsigned int VBO2, FloorVAO;
  {
    glGenVertexArrays(1, &FloorVAO);
    glGenBuffers(1, &VBO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FloorVertices), FloorVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(FloorVAO);

    // 载入位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    // 载入法向量
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  // 载入左墙的顶点信息
  //  ------------------------------------------------------------------
  unsigned int VBO3, LWallVAO;
  {
    glGenVertexArrays(1, &LWallVAO);
    glGenBuffers(1, &VBO3);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LWallVertices), LWallVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(LWallVAO);

    // 载入位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    // 载入法向量
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  // 载入右墙的顶点信息
  //  ------------------------------------------------------------------
  unsigned int VBO4, RWallVAO;
  {
    glGenVertexArrays(1, &RWallVAO);
    glGenBuffers(1, &VBO4);

    glBindBuffer(GL_ARRAY_BUFFER, VBO4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RWallVertices), RWallVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(RWallVAO);

    // 载入位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    // 载入法向量
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  // 载入前墙的顶点信息
  //  ------------------------------------------------------------------
  unsigned int VBO5, FWallVAO;
  {
    glGenVertexArrays(1, &FWallVAO);
    glGenBuffers(1, &VBO5);

    glBindBuffer(GL_ARRAY_BUFFER, VBO5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FWallVertices), FWallVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(FWallVAO);

    // 载入位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    // 载入法向量
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
  }

  // 载入方块灯的顶点信息
  unsigned int VBO6, lightCubeVAO;
  {
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO6);

    glBindBuffer(GL_ARRAY_BUFFER, VBO6);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO6);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
  }

  Model tableModel(std::filesystem::path("resources/objects/table/table3.obj"));

  // 渲染循环
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // 时间逻辑
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // 输入
    // -----
    processInput(window);

    // 开始渲染
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 确保在设置 Uniforms/Drawing 对象时激活 Shader
    //---------------------------------------------------------------------
    lightingShader.use();
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 lightColor{1.0, 1.0, 1.0};

    // view/projection
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);

    lightingShader.setVec3("viewPos", camera.Position);

    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
    glm::vec3 specularColor = glm::vec3(1.f);

    lightingShader.setVec3("light.ambient", ambientColor);
    lightingShader.setVec3("light.diffuse", diffuseColor);
    lightingShader.setVec3("light.specular", specularColor);
    lightingShader.setVec3("light.position", lightPos);

    { // top
      // material
      lightingShader.setVec3("material.ambient", {0.2, 0.2, 0.2});
      lightingShader.setVec3("material.diffuse", {1.0, 1.0, 1.0});
      lightingShader.setVec3("material.specular", {0.1f, 0.1f, 0.1f});
      lightingShader.setFloat("material.shininess", 64.0f);

      // model
      model = glm::translate(model, cubePos);
      model = glm::scale(model, {2.0f, 1.0f, 1.0f});
      lightingShader.setMat4("model", model);

      // draw
      glBindVertexArray(CeilingVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    { // bottom
      // material
      lightingShader.setVec3("material.ambient", {0.2, 0.2, 0.2});
      lightingShader.setVec3("material.diffuse", {0.5, 0.5, 0.5});
      lightingShader.setVec3("material.specular", {0.3, 0.3, 0.3});
      lightingShader.setFloat("material.shininess", 64.0f);

      // 世界坐标变换
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePos);
      model = glm::scale(model, {2.0f, 1.0f, 1.0f});
      lightingShader.setMat4("model", model);

      // 渲染
      glBindVertexArray(FloorVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    { // left
      // material
      lightingShader.setVec3("material.ambient", {0.2, 0.2, 0.2});
      lightingShader.setVec3("material.diffuse", {1.0, 0.0, 0.31});
      lightingShader.setVec3("material.specular", {1.0, 0.0, 0.31});
      lightingShader.setFloat("material.shininess", 64.0f);

      // 世界坐标变换
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePos);
      model = glm::translate(model, {-0.5, 0, 0});
      model = glm::scale(model, glm::vec3(1.0f));
      lightingShader.setMat4("model", model);

      // 渲染
      glBindVertexArray(LWallVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    { // right
      // 设置光照参数
      // material
      lightingShader.setVec3("material.ambient", {0.2, 0.2, 0.2});
      lightingShader.setVec3("material.diffuse", {1.0, 0.0, 0.31});
      lightingShader.setVec3("material.specular", {1.0, 0.0, 0.31});
      lightingShader.setFloat("material.shininess", 64.0f);

      // 世界坐标变换
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePos);
      model = glm::translate(model, {0.5, 0, 0});
      model = glm::scale(model, glm::vec3(1.0f));
      lightingShader.setMat4("model", model);

      // 渲染
      glBindVertexArray(RWallVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    { // front
      windowShader.use();

      // 设置光照参数
      // lightingShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
      windowShader.setVec3("objectColor", 0.3f, 0.3f, 0.3f);

      windowShader.setVec3("light.ambient", ambientColor);
      windowShader.setVec3("light.diffuse", diffuseColor);
      windowShader.setVec3("light.specular", specularColor);
      windowShader.setVec3("light.position", lightPos);

      windowShader.setVec3("material.ambient", {0.5, 0.25, 0.});
      windowShader.setVec3("material.diffuse", {0.5, 0.25, 0.});
      windowShader.setVec3("material.specular", {0.5, 0.25, 0.});
      windowShader.setFloat("material.shininess", 64.0f);

      windowShader.setVec3("viewPos", camera.Position);
      windowShader.setVec3("lightPos", lightPos);

      windowShader.setMat4("projection", projection);
      windowShader.setMat4("view", view);

      // 世界坐标变换
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePos);
      model = glm::scale(model, {2.0f, 1.0f, 1.0f});
      windowShader.setMat4("model", model);
      // FIXME: ugly
      windowShader.setVec2("texScale", {0.3, 0.6});
      windowShader.setVec2("texOffset", {0, 0.2});
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, windowDiffuseMap);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      // 渲染
      glBindVertexArray(FWallVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    {
      modelShader.use();
      glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
      glm::vec3 ambientColor = lightColor * glm::vec3(0.2f);
      modelShader.setVec3("light.position", lightPos);
      modelShader.setVec3("viewPos", camera.Position);

      modelShader.setVec3("light.ambient", ambientColor);
      modelShader.setVec3("light.diffuse", diffuseColor);
      modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

      modelShader.setFloat("light.constant", 1.0f);
      modelShader.setFloat("light.linear", 0.09f);
      modelShader.setFloat("light.quadratic", 0.032f);

      modelShader.setFloat("material.shininess", 32.0f);

      model = glm::mat4(1.0f);
      model = glm::translate(model, tablePos);
      model = glm::scale(model, glm::vec3(0.1f));
      modelShader.setMat4("model", model);
      modelShader.setMat4("projection", projection);
      modelShader.setMat4("view", view);
      tableModel.Draw(modelShader);
    }

    // 绘制灯方块
    {
      lightCubeShader.use();
      lightCubeShader.setMat4("projection", projection);
      lightCubeShader.setMat4("view", view);
      model = glm::mat4(1.0f);
      model = glm::translate(model, lightPos);
      model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
      lightCubeShader.setMat4("model", model);

      glBindVertexArray(lightCubeVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // glfw：交换缓冲区和轮询 IO 事件（按下/释放键、移动鼠标等）
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // （可选）一旦资源超出其用途，就取消分配所有资源：
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &CeilingVAO);
  glDeleteVertexArrays(1, &FloorVAO);
  glDeleteVertexArrays(1, &RWallVAO);
  glDeleteVertexArrays(1, &LWallVAO);
  glDeleteVertexArrays(1, &FWallVAO);
  glDeleteVertexArrays(1, &lightCubeVAO);
  glDeleteBuffers(1, &VBO1);
  glDeleteBuffers(1, &VBO2);
  glDeleteBuffers(1, &VBO3);
  glDeleteBuffers(1, &VBO4);
  glDeleteBuffers(1, &VBO5);
  glDeleteBuffers(1, &VBO6);

  // glfw：终止，清除所有以前分配的 GLFW 资源。
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// 查询 GLFW 是否按下/释放了该帧的相关键并做出相应的反应
//  ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw：每当窗口大小发生变化（通过操作系统或用户调整大小）时，此回调函数都会执行
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // 确保视区与新的窗口尺寸匹配;请注意，width和height将明显大于 Retina
  // 显示屏上指定的高度
  glViewport(0, 0, width, height);
}

// glfw: 每当鼠标移动时，该回调都会被调用
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // 反转，因为 y 坐标从下到上

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw:每当鼠标滚轮滚动时，该回调都会被调用
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
