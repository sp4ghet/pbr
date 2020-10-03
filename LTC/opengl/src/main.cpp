
#include "camera.h"
#include "glad/glad.h"
#include "glfw3/glfw3.h"
#include "ltc.h"
#include "shader.h"
#include "stb/stb_image.h"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <mesh.h>
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

Camera camera(glm::vec3(0., 2., 10.));
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

std::vector<glm::vec3> lightVerts = {
    glm::vec3(-1., -1., 0.),
    glm::vec3(1., -1., 0.),
    glm::vec3(1., 1., 0.),
    glm::vec3(-1., 1., 0.),
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
Mesh fullScreenQuad;

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
  Shader screenShader("./shaders/fullscreen_quad.vert",
                      "./shaders/postprocess.frag");
  Shader lightShader("./shaders/simple.vert", "./shaders/unlit.frag");
  printf("Shaders loaded...\n");

  // generate LTC textures
  unsigned int ltcMatId;
  float ltc_mat[64 * 64 * 4];
  std::copy(std::begin(g_ltc_mat), std::end(g_ltc_mat), std::begin(ltc_mat));
  // for (int y = 0; y < 64; ++y) {
  //   for (int x = 0; x < 64; ++x)
  //     for (int i = 0; i < 4; ++i) {
  //       ltc_mat[y * 64 * 4 + x * 4 + i] =
  //           g_ltc_mat[(63 - y) * 64 * 4 + x * 4 + i];
  //     }
  // }
  glGenTextures(1, &ltcMatId);
  glBindTexture(GL_TEXTURE_2D, ltcMatId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT,
               (void *)(&ltc_mat));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned int ltcMagId;
  float ltc_mag[64 * 64];
  std::copy(std::begin(g_ltc_mag), std::end(g_ltc_mag), std::begin(ltc_mag));
  // for (int y = 0; y < 64; ++y) {
  //   for (int x = 0; x < 64; ++x)
  //     ltc_mag[y * 64 + x] = g_ltc_mag[(63 - y) * 64 + x];
  // }
  glGenTextures(1, &ltcMagId);
  glBindTexture(GL_TEXTURE_2D, ltcMagId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 64, 64, 0, GL_RED, GL_FLOAT,
               (void *)(&ltc_mag));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glm::vec3 lightCol = glm::vec3(1.);
  std::vector<Texture> lightTextures{};
  Mesh lightMesh = Mesh(quadVertices, quadIndices, lightTextures);

  unsigned int crateTexId = TextureFromFile("container.jpg", "./textures");
  std::vector<Texture> planeTextures{
      Texture(crateTexId, GL_TEXTURE_2D, "texture_diffuse1"),
      Texture(ltcMatId, GL_TEXTURE_2D, "ltc_mat"),
      Texture(ltcMagId, GL_TEXTURE_2D, "ltc_mag")};
  Mesh planeMesh = Mesh(quadVertices, quadIndices, planeTextures);

#ifdef DEBUG
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum vertex attributes supported: %d\n", nrAttributes);
#endif

  renew_framebuffer(fbo, colorBuf, rbo);
  renew_msaa_buffer(msFbo, msColorBuf, msRbo);
  fullScreenQuadTextures =
      std::vector<Texture>{Texture(colorBuf, GL_TEXTURE_2D, "renderBuffer")};
  fullScreenQuad = Mesh(quadVertices, quadIndices, fullScreenQuadTextures);

  while (!glfwWindowShouldClose(window)) {
    deltaTime = (float)glfwGetTime() - prevTime;
    prevTime = (float)glfwGetTime();
    if (recompileFlag) {
      shader.recompile();
      screenShader.recompile();
      lightShader.recompile();
      recompileFlag = false;
    }
    // input
    processInput(window);
    glm::mat4 proj = glm::perspective(
        glm::radians(camera.Zoom),
        (float)resolution.first / (float)resolution.second, 0.001f, 100.f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 mvp = proj * view;

    // rendering commands

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // render to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 lightModel = glm::mat4(1.);
    lightModel = glm::translate(lightModel, glm::vec3(0., 2., 0.));
    lightModel = glm::rotate(lightModel, glm::radians(prevTime * 20.f),
                             glm::vec3(0., 1., 0.));
    lightModel = glm::scale(lightModel, glm::vec3(2., 1., 1.));

    lightShader.use();
    lightShader.setMat4("model", lightModel);
    mvp = proj * view * lightModel;
    lightShader.setMat4("MVP", mvp);
    lightShader.setVec3("color", lightCol);
    lightMesh.Draw(lightShader);

    std::vector<glm::vec3> lightVertices;
    for (int i = 0; i < 4; ++i) {
      glm::vec4 worldLight = lightModel * glm::vec4(lightVerts[i], 1.);
      lightVertices.push_back(glm::vec3(worldLight) / worldLight.w);
      // printf("%d: %f %f %f\n", i, lightVertices[i].x, lightVertices[i].y,
      //        lightVertices[i].z);
    }

    glm::mat4 planeModel = glm::mat4(1.);
    planeModel = glm::translate(planeModel, glm::vec3(0., 0., 0.));
    planeModel =
        glm::rotate(planeModel, glm::radians(-90.f), glm::vec3(1., 0., 0.));
    planeModel = glm::scale(planeModel, glm::vec3(10.));

    shader.use();
    shader.setMat4("model", planeModel);
    mvp = proj * view * planeModel;
    shader.setMat4("MVP", mvp);
    shader.setVec3("camPos", camera.Position);
    shader.setBool("hasDiffuse", true);
    shader.setBool("hasSpecular", false);
    shader.setBool("hasNormal", false);
    shader.setBool("hasRoughness", false);
    shader.setVec3("lightColor", lightCol);
    shader.setVec3Array("lightVerts", lightVertices);
    planeMesh.Draw(shader);

    // draw MSAA texture to intermediate buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, resolution.first, resolution.second, 0, 0,
                      resolution.first, resolution.second, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    screenShader.use();
    fullScreenQuad.Draw(screenShader);

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
  fullScreenQuad = Mesh(quadVertices, quadIndices, fullScreenQuadTextures);
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
