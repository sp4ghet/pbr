#include "glad.h"
#include "glfw3.h"
#include "shader.h"
#include <cstdio>
#include <math.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

// clang-format off
float vertices[] = {
  0.5f, 0.5f, 0.0f,  1., 1., 0., // top right
  0.5f, -0.5f, 0.0f, 1., 0., 0., // bottom right
  -0.5f, -0.5f, 0.0f, 0., 0., 0., // bottom left
  -0.5f, 0.5f, 0.0f, 0., 1., 0., // top left
};
unsigned int indices[] = { // note that we start from 0!
  0, 1, 3, // first triangle
  1, 2, 3 // second triangle
};
// clang-format on

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

  Shader shader("../../src/simple.vert", "../../src/simple.frag");

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // shader attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

#ifdef DEBUG
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Maximum vertex attributes supported: %d\n", nrAttributes);
#endif

  while (!glfwWindowShouldClose(window)) {
    // input
    processInput(window);

    // rendering commands
    glClearColor(0.05f, 0.35f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    float timeValue = (float)glfwGetTime();
    float blue = (sin(timeValue) * .5) + .5;
    shader.setVec4("ourColor", 0., 0., blue, 1.);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // check and call events and swap the buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
