
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
Camera camera(glm::vec3(0., 0., 10.));
float prevTime = 0., deltaTime = 0.;
bool recompileFlag = false;

glm::vec3 n = glm::vec3(1.);
vector<Vertex> quadVertices = {
    Vertex(glm::vec3(-1., 1., 0.), n, glm::vec2(0., 1.)),
    Vertex(glm::vec3(-1., -1., 0.), n, glm::vec2(0., 0.)),
    Vertex(glm::vec3(1., -1., 0.), n, glm::vec2(1., 0.)),
    Vertex(glm::vec3(1., 1., 0.), n, glm::vec2(1., 1.)),
};

vector<unsigned int> quadIndices = {0, 1, 3, 1, 2, 3};
vector<Texture> quadTextures;

int main(int, char **) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
  printf("Shaders loaded...\n");

  Mesh quadMesh(quadVertices, quadIndices, quadTextures);
  Model backpack("./resources/backpack/backpack.obj");
  printf("Model loaded...\n");
#ifdef DEBUG
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum vertex attributes supported: %d\n", nrAttributes);
#endif

  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE,
               NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture, 0);

  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("ERROR::FRAMEBUFFER:: Frame buffer is not complete!\n");
  }

  while (!glfwWindowShouldClose(window)) {
    deltaTime = (float)glfwGetTime() - prevTime;
    prevTime = (float)glfwGetTime();
    if (recompileFlag) {
      shader.recompile();
      recompileFlag = false;
    }
    // input
    processInput(window);

    // rendering commands

    // render to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj =
        glm::perspective(glm::radians(camera.Zoom), 4.f / 3.f, 0.001f, 100.f);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(.1);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1., 0., 0.));
    glm::mat4 mvp = proj * view * model;
    shader.use();
    shader.setMat4("MVP", mvp);
    shader.setMat4("model", model);
    shader.setVec3("camPos", camera.Position.x, camera.Position.y,
                   camera.Position.z);

    backpack.Draw(shader);

    // restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
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
  if (lastX < 0. || lastY < 0.) {
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
