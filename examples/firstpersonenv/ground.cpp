#include "ground.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>

void Ground::initializeGL(GLuint program) {
  // Unit quad on the xz plane
  // clang-format off
  std::array vertices{glm::vec3(-0.5f, 0.0f,  0.5f), 
                      glm::vec3(-0.5f, 0.0f, -0.5f),
                      glm::vec3( 0.5f, 0.0f,  0.5f),
                      glm::vec3( 0.5f, 0.0f, -0.5f)};
  // clang-format on                      

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create VAO and bind vertex attributes
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  const GLint posAttrib{abcg::glGetAttribLocation(program, "inPosition")};
  abcg::glEnableVertexAttribArray(posAttrib);
  abcg::glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);

  // Save location of uniform variables
  m_modelMatrixLoc = abcg::glGetUniformLocation(program, "modelMatrix");
  m_colorLoc = abcg::glGetUniformLocation(program, "color");
}

void Ground::paintGL() {
  // Draw a grid of tiles centered on the xz plane
  abcg::glBindVertexArray(m_VAO);
  for (const auto z : iter::range(-m_envLimits.z, m_envLimits.z + 1)) {
    for (const auto x : iter::range(-m_envLimits.x, m_envLimits.x + 1)) {
      // Set model matrix
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 0.0f, z));
      abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

      abcg::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  }
  abcg::glBindVertexArray(0);
}

void Ground::terminateGL() {
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}