
#include "camera.h"
#include "glad/glad.h"
#include "glfw3/glfw3.h"
#include "shader.h"
#include "stb/stb_image.h"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <mesh.h>
#include <model.h>
#include <vector>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void renew_msaa_buffer(unsigned int &fbo, unsigned int &colorBuf,
                       unsigned int &rbo);
void renew_framebuffer(unsigned int &fbo, unsigned int &colorBuf,
                       unsigned int &rbo);

Camera camera(glm::vec3(0., 0., 10.));
float prevTime = 0., deltaTime = 0.;
bool recompileFlag = false;
std::pair<int, int> resolution = {800, 600};

glm::vec3 n = glm::vec3(0., 0., 1.);
glm::vec3 t = glm::vec3(0., 1., 0.);
std::vector<Vertex> quadVertices = {
    Vertex(glm::vec3(-1., 1., 0.), n, t, glm::vec2(0., 1.)),
    Vertex(glm::vec3(-1., -1., 0.), n, t, glm::vec2(0., 0.)),
    Vertex(glm::vec3(1., -1., 0.), n, t, glm::vec2(1., 0.)),
    Vertex(glm::vec3(1., 1., 0.), n, t, glm::vec2(1., 1.)),
};

std::vector<unsigned int> quadIndices = {0, 1, 3, 1, 2, 3};

// clang-format off
std::vector<Vertex> cubeVertices = {
  Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.,0.,-1.), glm::vec3(0.,1.,0.), glm::vec2(0., 0.)),
  Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.,0.,-1.), glm::vec3(0.,1.,0.), glm::vec2(0., 1.)),
  Vertex(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.,0.,-1.), glm::vec3(0.,1.,0.), glm::vec2(1., 1.)),
  Vertex(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.,0.,-1.), glm::vec3(0.,1.,0.), glm::vec2(1., 0.)),

  Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(0., 0.)),
  Vertex(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(0., 1.)),
  Vertex(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(1., 1.)),
  Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(1., 0.)),

  Vertex(glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(0., 1.)),
  Vertex(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(0., 0.)),
  Vertex(glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(1., 0.)),
  Vertex(glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1., 0.,0.), glm::vec3(0.,1.,0.), glm::vec2(1., 1.)),

  Vertex(glm::vec3(-1.0f,  1.0f, 1.0f), glm::vec3(0., 0., 1.), glm::vec3(0.,1.,0.), glm::vec2(0., 1.)),
  Vertex(glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(0., 0., 1.), glm::vec3(0.,1.,0.), glm::vec2(0., 0.)),
  Vertex(glm::vec3( 1.0f, -1.0f, 1.0f), glm::vec3(0., 0., 1.), glm::vec3(0.,1.,0.), glm::vec2(1., 0.)),
  Vertex(glm::vec3( 1.0f,  1.0f, 1.0f), glm::vec3(0., 0., 1.), glm::vec3(0.,1.,0.), glm::vec2(1., 1.)),

  Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.),  glm::vec2(0., 0.)),
  Vertex(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.), glm::vec2(0., 1.)),
  Vertex(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.), glm::vec2(1., 1.)),
  Vertex(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.), glm::vec2(1., 0.)),

  Vertex(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0., -1., 0.), glm::vec3(0., 0., 1.), glm::vec2(0., 1.)),
  Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0., -1., 0.), glm::vec3(0., 0., 1.), glm::vec2(0., 0.)),
  Vertex(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0., -1., 0.), glm::vec3(0., 0., 1.), glm::vec2(1., 0.)),
  Vertex(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0., -1., 0.), glm::vec3(0., 0., 1.), glm::vec2(1., 1.)),
};

std::vector<unsigned int> cubeIndices = {
  0,1,3,
  1,2,3,

  4,5,7,
  5,6,7,

  8,9,11,
  9,10,11,

  12,13,15,
  13,14,15,

  16,17,19,
  17,18,19,

  20,21,23,
  21,22,23
};
// clang-format on

unsigned int msFbo;
unsigned int msColorBuf;
unsigned int msRbo;

unsigned int fbo;
unsigned int colorBuf;
unsigned int rbo;

std::vector<Texture> fullScreenQuadTextures;
Mesh quadMesh;

int main(int, char **) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "Learn OpenGL", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }

  glViewport(0, 0, 800, 600);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);

  stbi_set_flip_vertically_on_load(true);

  // load images
  Shader shader("./shaders/simple.vert", "./shaders/simple.frag");
  Shader outline("./shaders/simple.vert", "./shaders/fill.frag");
  Shader screenShader("./shaders/fullscreen_quad.vert",
                      "./shaders/postprocess.frag");
  Shader skyboxShader("./shaders/skybox.vert", "./shaders/skybox.frag");
  Shader shadowShader("./shaders/simpleShadow.vert",
                      "./shaders/simpleShadow.frag");
  printf("Shaders loaded...\n");

  Model backpack("./resources/backpack/backpack.obj");
  printf("Model loaded...\n");

  std::vector<std::string> cubemap_faces = {
      "./resources/textures/skybox/right.jpg",
      "./resources/textures/skybox/left.jpg",
      "./resources/textures/skybox/top.jpg",
      "./resources/textures/skybox/bottom.jpg",
      "./resources/textures/skybox/front.jpg",
      "./resources/textures/skybox/back.jpg"};
  unsigned int cubemapId = loadCubemap(cubemap_faces);
  printf("Cubemap loaded...\n");

  std::vector<Texture> skyboxTextures = {
      Texture(cubemapId, GL_TEXTURE_CUBE_MAP, "skybox")};
  Mesh skybox = Mesh(cubeVertices, cubeIndices, skyboxTextures);

  unsigned int boxTex = TextureFromFile("container.jpg", "./textures");
  Texture crateTex(boxTex, GL_TEXTURE_2D, "texture_diffuse1");
  Mesh cubeMesh =
      Mesh(cubeVertices, cubeIndices, std::vector<Texture>{crateTex});

  Mesh planeMesh =
      Mesh(quadVertices, quadIndices, std::vector<Texture>{crateTex});

#ifdef DEBUG
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum vertex attributes supported: %d\n", nrAttributes);
#endif

  renew_framebuffer(fbo, colorBuf, rbo);
  renew_msaa_buffer(msFbo, msColorBuf, msRbo);
  fullScreenQuadTextures =
      std::vector<Texture>{Texture(colorBuf, GL_TEXTURE_2D, "renderBuffer")};
  quadMesh = Mesh(quadVertices, quadIndices, fullScreenQuadTextures);

  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1., 1., 1., 1.};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glm::vec3 lightPos = glm::vec3(-4., 4., -4.);
  std::vector<glm::vec3> lightColors;
  lightColors.push_back(glm::vec3(3.f));
  lightColors.push_back(glm::vec3(0.5f, 0.0f, 0.0f));
  lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.5f));
  lightColors.push_back(glm::vec3(0.0f, 0.5f, 0.0f));
  std::vector<glm::vec3> lightPositions;
  lightPositions.push_back(glm::vec3(-4., 4., -4.));
  lightPositions.push_back(glm::vec3(4., 1., 4.));
  lightPositions.push_back(glm::vec3(-4., 1., 4.));
  lightPositions.push_back(glm::vec3(4., 1., -4.));

  glm::mat4 proj, view;

  auto RenderScene = [&](bool shadowPass = false) {
    Shader useShader = shader;
    if (shadowPass) {
      useShader = shadowShader;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glm::mat4 lightView =
        glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0., 1., 0.));
    float shadowNear = .5, shadowFar = 20.f;
    glm::mat4 lightProj =
        glm::ortho(-10.f, 10.f, -10.f, 10.f, shadowNear, shadowFar);
    useShader.setMat4("lightVP", lightProj * lightView);
    useShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    useShader.setInt("shadow_map", 14);
    glActiveTexture(GL_TEXTURE0);
    useShader.setVec3Array("lightPositions", lightPositions);
    useShader.setVec3Array("lightColors", lightColors);

    glm::mat4 model = glm::mat4(1.);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1., 0., 0.));

    glm::mat4 mvp = proj * view * model;
    useShader.use();
    useShader.setMat4("MVP", mvp);
    useShader.setMat4("model", model);
    useShader.setVec3("camPos", camera.Position.x, camera.Position.y,
                      camera.Position.z);

    useShader.setBool("hasDiffuse", true);
    useShader.setBool("hasSpecular", true);
    useShader.setBool("hasNormal", true);
    useShader.setBool("hasRoughness", true);
    backpack.Draw(useShader);

    glm::mat4 lightCubeModel = glm::mat4(1.);
    lightCubeModel = glm::translate(lightCubeModel, lightPos);
    lightCubeModel = glm::scale(lightCubeModel, glm::vec3(0.1));

    useShader.setMat4("model", lightCubeModel);
    mvp = proj * view * lightCubeModel;
    useShader.setMat4("MVP", mvp);
    useShader.setBool("hasDiffuse", true);
    useShader.setBool("hasSpecular", false);
    useShader.setBool("hasNormal", false);
    useShader.setBool("hasRoughness", false);
    cubeMesh.Draw(useShader);

    glm::mat4 cubeModel = glm::mat4(1.);
    cubeModel = glm::translate(cubeModel, glm::vec3(3., 0., 3.));

    useShader.setMat4("model", cubeModel);
    mvp = proj * view * cubeModel;
    useShader.setMat4("MVP", mvp);

    useShader.setBool("hasDiffuse", true);
    useShader.setBool("hasSpecular", false);
    useShader.setBool("hasNormal", false);
    useShader.setBool("hasRoughness", false);
    cubeMesh.Draw(useShader);

    glm::mat4 planeModel = glm::mat4(10.);
    planeModel = glm::translate(planeModel, glm::vec3(0., -1., 0.));
    planeModel =
        glm::rotate(planeModel, glm::radians(-90.f), glm::vec3(1., 0., 0.));
    planeModel = glm::scale(planeModel, glm::vec3(10.));

    useShader.setMat4("model", planeModel);
    mvp = proj * view * planeModel;
    useShader.setMat4("MVP", mvp);
    useShader.setBool("hasDiffuse", true);
    useShader.setBool("hasSpecular", false);
    useShader.setBool("hasNormal", false);
    useShader.setBool("hasRoughness", false);
    planeMesh.Draw(useShader);
  };

  while (!glfwWindowShouldClose(window)) {
    deltaTime = (float)glfwGetTime() - prevTime;
    prevTime = (float)glfwGetTime();
    if (recompileFlag) {
      shader.recompile();
      screenShader.recompile();
      skyboxShader.recompile();
      shadowShader.recompile();
      recompileFlag = false;
    }
    // input
    processInput(window);

    proj = glm::perspective(glm::radians(camera.Zoom),
                            (float)resolution.first / (float)resolution.second,
                            0.001f, 100.f);
    view = camera.GetViewMatrix();

    // rendering commands

    // render to shadow map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);

    shadowShader.use();

    glCullFace(GL_FRONT);
    RenderScene(true);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, resolution.first, resolution.second);

    // render to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shader.use();
    RenderScene(false);

    // render cubemap
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    skyboxShader.use();
    skyboxShader.setMat4("projection", proj);
    skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));

    skybox.Draw(skyboxShader);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);

    // draw MSAA texture to intermediate buffer

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, resolution.first, resolution.second, 0, 0,
                      resolution.first, resolution.second, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    screenShader.use();
    quadMesh.Draw(screenShader);

    // check and call events and swap the buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

// -----------------------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  resolution = std::pair<int, int>(width, height);
  renew_framebuffer(fbo, colorBuf, rbo);
  renew_msaa_buffer(msFbo, msColorBuf, msRbo);
  fullScreenQuadTextures =
      std::vector<Texture>{Texture(colorBuf, GL_TEXTURE_2D, "renderBuffer")};
  quadMesh = Mesh(quadVertices, quadIndices, fullScreenQuadTextures);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.ProcessKeyBoard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.ProcessKeyBoard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.ProcessKeyBoard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.ProcessKeyBoard(RIGHT, deltaTime);
  }

  if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !recompileFlag) {
    recompileFlag = true;
  }
}

bool rightClick = false;
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    rightClick = true;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    rightClick = false;
  }
}

float lastX = -1., lastY = -1.;
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (!rightClick) {
    lastX = -1.;
    lastY = -1.;
    return;
  }
  float dx = xpos - lastX, dy = lastY - ypos;
  if (lastX < 0.f || lastY < 0.f) {
    dx = 0.;
    dy = 0.;
  }
  lastX = (float)xpos;
  lastY = (float)ypos;

  camera.ProcessMouseMovement(dx, dy);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll((float)yoffset);
}

void renew_msaa_buffer(unsigned int &msFbo, unsigned int &msColorBuf,
                       unsigned int &msRbo) {
  glGenFramebuffers(1, &msFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, msFbo);

  glGenTextures(1, &msColorBuf);
  glBindTexture(GL_TEXTURE_2D, msColorBuf);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resolution.first, resolution.second,
               0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         msColorBuf, 0);

  glGenRenderbuffers(1, &msRbo);
  glBindRenderbuffer(GL_RENDERBUFFER, msRbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
                                   resolution.first, resolution.second);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, msRbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("ERROR::FRAMEBUFFER:: Frame buffer is not complete!\n");
  }
}

void renew_framebuffer(unsigned int &fbo, unsigned int &colorBuf,
                       unsigned int &rbo) {

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &colorBuf);
  glBindTexture(GL_TEXTURE_2D, colorBuf);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resolution.first, resolution.second,
               0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         colorBuf, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.first,
                        resolution.second);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("ERROR::FRAMEBUFFER:: Frame buffer is not complete!\n");
  }
}
