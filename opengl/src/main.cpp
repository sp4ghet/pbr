
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

unsigned int loadCubemap(vector<string> textures_faces);
void renew_msaa_buffer(unsigned int &fbo, unsigned int &colorBuf,
                       unsigned int &rbo);
void renew_framebuffer(unsigned int &fbo, unsigned int &colorBuf,
                       unsigned int &rbo);

Camera camera(glm::vec3(0., 0., 10.));
float prevTime = 0., deltaTime = 0.;
bool recompileFlag = false;
std::pair<int, int> resolution = {800, 600};

glm::vec3 n = glm::vec3(1.);
vector<Vertex> quadVertices = {
    Vertex(glm::vec3(-1., 1., 0.), n, glm::vec2(0., 1.)),
    Vertex(glm::vec3(-1., -1., 0.), n, glm::vec2(0., 0.)),
    Vertex(glm::vec3(1., -1., 0.), n, glm::vec2(1., 0.)),
    Vertex(glm::vec3(1., 1., 0.), n, glm::vec2(1., 1.)),
};

vector<unsigned int> quadIndices = {0, 1, 3, 1, 2, 3};
vector<Texture> quadTextures;

vector<string> cubemap_faces = {"./resources/textures/skybox/right.jpg",
                                "./resources/textures/skybox/left.jpg",
                                "./resources/textures/skybox/top.jpg",
                                "./resources/textures/skybox/bottom.jpg",
                                "./resources/textures/skybox/front.jpg",
                                "./resources/textures/skybox/back.jpg"};

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

unsigned int msFbo;
unsigned int msColorBuf;
unsigned int msRbo;

unsigned int fbo;
unsigned int colorBuf;
unsigned int rbo;

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

  Shader shader("./shaders/simple.vert", "./shaders/simple.frag");
  Shader outline("./shaders/simple.vert", "./shaders/fill.frag");
  Shader screenShader("./shaders/fullscreen_quad.vert",
                      "./shaders/postprocess.frag");
  Shader skyboxShader("./shaders/skybox.vert", "./shaders/skybox.frag");
  Shader shadowShader("./shaders/simpleShadow.vert",
                      "./shaders/simpleShadow.frag");
  printf("Shaders loaded...\n");

  Mesh quadMesh(quadVertices, quadIndices, quadTextures);
  Model backpack("./resources/backpack/backpack.obj");
  printf("Model loaded...\n");

#ifdef DEBUG
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum vertex attributes supported: %d\n", nrAttributes);
#endif

  renew_framebuffer(fbo, colorBuf, rbo);
  renew_msaa_buffer(msFbo, msColorBuf, msRbo);

  unsigned int cubemapId = loadCubemap(cubemap_faces);
  printf("Cubemap loaded...\n");
  unsigned int skyboxVAO;
  glGenVertexArrays(1, &skyboxVAO);
  unsigned int skyboxVBO;
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);

  glm::vec3 lightPos = glm::vec3(-2., 4., -2.);
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  const int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glm::mat4 proj, view;

  auto RenderScene = [&](Shader useShader) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glm::mat4 lightView =
        glm::lookAt(lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0., 1., 0.));
    float shadowNear = 1., shadowFar = 15.f;
    glm::mat4 lightProj =
        glm::ortho(-10.f, 10.f, -10.f, 10.f, shadowNear, shadowFar);
    useShader.setMat4("lightVP", lightProj * lightView);
    useShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.y);
    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    useShader.setInt("shadow_map", 14);
    glActiveTexture(GL_TEXTURE0);

    glm::mat4 model = glm::mat4(.1);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1., 0., 0.));

    glm::mat4 mvp = proj * view * model;
    useShader.use();
    useShader.setMat4("MVP", mvp);
    useShader.setMat4("model", model);
    useShader.setVec3("camPos", camera.Position.x, camera.Position.y,
                      camera.Position.z);

    backpack.Draw(useShader);

    glm::mat4 cubeModel = glm::mat4(1.);
    cubeModel = glm::translate(cubeModel, glm::vec3(3., 0., 3.));

    glBindVertexArray(skyboxVAO);
    useShader.setMat4("model", cubeModel);
    mvp = proj * view * cubeModel;
    useShader.setMat4("MVP", mvp);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 planeModel = glm::mat4(10.);
    planeModel = glm::translate(planeModel, glm::vec3(0., -1., 0.));
    planeModel =
        glm::rotate(planeModel, glm::radians(-90.f), glm::vec3(1., 0., 0.));
    planeModel = glm::scale(planeModel, glm::vec3(10.));
    useShader.setMat4("model", planeModel);
    mvp = proj * view * planeModel;
    useShader.setMat4("MVP", mvp);
    quadMesh.Draw(useShader);
  };

  while (!glfwWindowShouldClose(window)) {
    deltaTime = (float)glfwGetTime() - prevTime;
    prevTime = (float)glfwGetTime();
    if (recompileFlag) {
      shader.recompile();
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

    shadowShader.use();
    RenderScene(shadowShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, resolution.first, resolution.second);

    // render to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shader.use();
    RenderScene(shader);

    // render cubemap
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    skyboxShader.setMat4("projection", proj);
    skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);
    skyboxShader.setInt("skybox", 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
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

    screenShader.use();
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuf);
    screenShader.setInt("renderBuffer", 0);
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

unsigned int loadCubemap(vector<std::string> faces) {
  stbi_set_flip_vertically_on_load(false);
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return textureID;

  stbi_set_flip_vertically_on_load(true);
}

void renew_msaa_buffer(unsigned int &msFbo, unsigned int &msColorBuf,
                       unsigned int &msRbo) {
  glGenFramebuffers(1, &msFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, msFbo);

  glGenTextures(1, &msColorBuf);
  glBindTexture(GL_TEXTURE_2D, msColorBuf);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.first, resolution.second, 0,
               GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.first, resolution.second, 0,
               GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
