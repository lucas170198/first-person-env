#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w) {
      const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
      m_dollySpeed = keyboardState[SDL_SCANCODE_LCTRL]
                         ? 3.0f
                         : 1.0f;  // press space key to run
    }
    if (ev.key.keysym.sym == SDLK_SPACE && m_jumpSpeed == 0.0f)
      m_jumpSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
  }
  if (ev.type == SDL_KEYUP) {
    if ((ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "lookat.vert",
                                    getAssetsPath() + "lookat.frag");

  m_ground.initializeGL(m_program);

  // Load model
  loadModelFromFile(getAssetsPath() + "lowpolytree.obj");

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  const GLint positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  // setup Trees
  Object firstTree{.position = glm::vec3(0.0f, 0.8f, -1.0f),
                   .scale = glm::vec3(0.5f)};
  Object secondTree{.position = glm::vec3(4.0f, 0.8f, 0.0f),
                    .scale = glm::vec3(0.5f)};
  Object thirdTree{.position = glm::vec3(-4.0f, 0.8f, 1.0f),
                   .scale = glm::vec3(0.5f)};

  m_objects.push_back(firstTree);
  m_objects.push_back(secondTree);
  m_objects.push_back(thirdTree);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::loadModelFromFile(std::string_view path) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over indices
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      const tinyobj::index_t index{shape.mesh.indices.at(offset)};

      // Vertex position
      const int startIndex{3 * index.vertex_index};
      const float vx{attrib.vertices.at(startIndex + 0)};
      const float vy{attrib.vertices.at(startIndex + 1)};
      const float vz{attrib.vertices.at(startIndex + 2)};

      Vertex vertex{};
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables (could be precomputed)
  const GLint viewMatrixLoc{
      abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{
      abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_projMatrix[0][0]);

  abcg::glBindVertexArray(m_VAO);

  glm::mat4 model{1.0f};

  // green objects
  glm::vec4 green{0.48f, 0.99f, 0.0f, 1.0f};
  abcg::glUniform4f(colorLoc, green.x, green.y, green.z, green.a);

  // drawing objects
  for (auto& obj : m_objects) {
    model = glm::mat4(1.0);
    model = glm::translate(model, obj.position);
    model = glm::scale(model, obj.scale);

    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                         nullptr);
  }

  abcg::glBindVertexArray(0);

  // Draw ground
  m_ground.paintGL();
  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() { abcg::OpenGLWindow::paintUI(); }

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
  m_ground.terminateGL();

  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

bool OpenGLWindow::checkForColisions(glm::vec3 nextPosAfterMov){

  for(auto &obj : m_objects){
    glm::mat4 model{1.0f};
    model = glm::translate(model, obj.position);
    model = glm::scale(model, obj.scale);
    glm::vec3 eyePos = model * glm::vec4(nextPosAfterMov, 1);
    fmt::print("{} {}\n", eyePos.x, eyePos.z);
    if((eyePos.x >= -0.3f && eyePos.x <= 0.3f) || (eyePos.z >= -0.3f || eyePos.z <= 0.3f))
      return false;
  }

  return false;
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  // Check for jump limits
  if (m_jumpSpeed > 0 && m_camera.m_at.y >= m_ground.m_envLimits.y) {
    m_jumpSpeed = -1.0f;
  }

  if (m_jumpSpeed < 0 && m_camera.m_eye.y <= m_ground.floorLevel) {
    m_jumpSpeed = 0.0f;
  }

  // Check if the character is inside the env before dolly
  if (m_dollySpeed != 0) {
    glm::vec3 nextFowardPos{m_dollySpeed * deltaTime * m_camera.getForward()};
    glm::vec3 nextEyePos = m_camera.m_eye + nextFowardPos;
    // check for limits
    if (nextEyePos.x >= m_ground.m_envLimits.x ||
        nextEyePos.x <= -m_ground.m_envLimits.x ||
        nextEyePos.z >= m_ground.m_envLimits.z ||
        nextEyePos.z <= -m_ground.m_envLimits.z){
      m_dollySpeed = 0.0f;
    }

    if (checkForColisions(nextEyePos)){
      m_dollySpeed = 0.0f;
    }
  }

  if (m_truckSpeed != 0) {
    glm::vec3 nextFowardPos{m_truckSpeed * deltaTime * m_camera.getLeft()};
    glm::vec3 nextEyePos = m_camera.m_eye - nextFowardPos;
    // check for limits
    if (nextEyePos.x >= m_ground.m_envLimits.x ||
        nextEyePos.x <= -m_ground.m_envLimits.x ||
        nextEyePos.z >= m_ground.m_envLimits.z ||
        nextEyePos.z <= -m_ground.m_envLimits.z){
      m_truckSpeed = 0.0f;
    }

    if (checkForColisions(nextEyePos)){
      m_truckSpeed = 0.0f;
    }
  }
  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
  m_camera.jump(m_jumpSpeed * deltaTime);
}