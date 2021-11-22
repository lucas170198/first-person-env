#ifndef GROUND_HPP_
#define GROUND_HPP_

#include "abcg.hpp"
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>

class Ground {
 public:
  void initializeGL(GLuint program);
  void paintGL(); 
  void terminateGL();
  const glm::vec3 m_envLimits{5.0f, 1.0f, 5.0f};
  const float floorLevel{0.5f};
 
 


 private:
  GLuint m_VAO{};
  GLuint m_VBO{};

  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};
};

#endif