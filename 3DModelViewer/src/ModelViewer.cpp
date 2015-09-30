#include "ModelViewer.h"

#include "gtc\matrix_transform.hpp"
#include "gtc\type_ptr.hpp"
#include "gtx\rotate_vector.hpp"

#include <fstream>
#include "QSurface"

#include "Resources\assimp\include\assimp\Importer.hpp"
#include "Resources\assimp\include\assimp\scene.h"
#include "Resources\assimp\include\assimp\postprocess.h"

ModelViewer::ModelViewer(QWidget* parent) :
  QOpenGLWidget(parent),
  _xPos(0.0),
  _yPos(0.0),
  _zPos(3.0),
  _fov(45.0),
  _camPosition(glm::vec3(0.0, 0.0, 3.0)),
  _camDirection(glm::vec3(0.0, 0.0, 0.0)),
  _camUp(glm::vec3(0.0, 1.0, 0.0)),
  _lightColor(glm::vec3(1.0, 1.0, 1.0)),
  _lightPos(glm::vec3(0.0, 5.0, 0.0)),
  _pendingMVPChange(false),
  _modelLoaded(false),
  _lightingEnabled(true),
  _texturingEnabled(true),
  _viewMode(ModelView),
  _file("")
{
    // Set OpenGL format
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);

    makeCurrent();

    // Set up our matrices
    _projection = glm::perspective(_fov, double(width()) / double(height()), 0.1, 10000.0);
    _view = glm::lookAt(_camPosition, _camDirection, _camUp);
    _model = glm::mat4(1.0); // identity matrix for now - will later be replaced from our model

    recalculateMVP();

    // Allows this widget to take focus on mouse click
    setFocusPolicy(Qt::ClickFocus);
}

ModelViewer::~ModelViewer() {
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteProgram(_programId);
}

void ModelViewer::initializeGL() {

    // Create a logger for debugging purposes
    _logger = new QOpenGLDebugLogger(this);

    connect(_logger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

    if(_logger->initialize()) {
        _logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        _logger->enableMessages();
    }

    // Initialize OpenGL for Qt
    initializeOpenGLFunctions();

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Load and compile our shaders
    _programId = glCreateProgram();
    loadShader("shaders\\vertex.shader", GL_VERTEX_SHADER, _programId);
    loadShader("shaders\\fragment.shader", GL_FRAGMENT_SHADER, _programId);
    glUseProgram(_programId);

    // Get uniform handles
    _uniformMVPHandle = glGetUniformLocation(_programId, "mvp");
    _uniformTexSamplerHandle = glGetUniformLocation(_programId, "texSampler");
    _uniformModelHandle = glGetUniformLocation(_programId, "model");
    _uniformTexEnabledHandle = glGetUniformLocation(_programId, "texturingEnabled");
    _uniformLightingHandle = glGetUniformLocation(_programId, "lightColor");
    _uniformLightPosHandle = glGetUniformLocation(_programId, "lightPos");
    _uniformLightingEnabledHandle = glGetUniformLocation(_programId, "lightingEnabled");
    _uniformViewPosHandle = glGetUniformLocation(_programId, "viewPos");

    // Set some uniforms here; the others will be set upon render
    glUniform1f(_uniformTexEnabledHandle, 1.0f);
    glUniform3f(_uniformLightingHandle, 1.0f, 1.0f, 1.0f);
    glUniform3fv(_uniformLightPosHandle, 1, glm::value_ptr(_lightPos));
    glUniform1f(_uniformLightingEnabledHandle, 1.0f);
    glUniform4fv(_uniformModelHandle, 1, glm::value_ptr(_model));

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
}

void ModelViewer::paintGL() {

    if(!_modelLoaded)
        return;
    if(_mainModel->isModelMatrixOutOfDate())
        recalculateMVP();

    processCameraMovements();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Smooth out the lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Set polygon mode based on current viewing mode
    if(_viewMode == ViewMode::PointCloud)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if(_viewMode == ViewMode::WireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if(_viewMode == ViewMode::ModelView)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(_programId);
    glBindVertexArray(_vertexArray);

    // Set uniforms
    glUniformMatrix4fv(_uniformMVPHandle, 1, GL_FALSE, glm::value_ptr(_mvp));
    glUniform4fv(_uniformModelHandle, 1, glm::value_ptr(_model));
    glUniform3fv(_uniformLightPosHandle, 1, glm::value_ptr(_lightPos));

    for(Model::Mesh mesh : _meshes) {
        glActiveTexture(GL_TEXTURE0 + mesh.diffuseTexture.texId);
        glBindTexture(GL_TEXTURE_2D, mesh.diffuseTexture.texId);

        // Set texture sampler
        glUniform1i(_uniformTexSamplerHandle, mesh.diffuseTexture.texId);

        // First attribute buffer - vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Second attribute buffer - texture coordinates
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Third attribute buffer - normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, mesh.numFaces * 3);
    }

    // Clean up
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    // Swap buffers
    makeCurrent();
    context()->swapBuffers(context()->surface());
    update();
}

void ModelViewer::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);

    _projection = glm::perspective(45.0, double(width) / double(height), 0.1, 10000.0);

    recalculateMVP();
}

void ModelViewer::updateGL() {
    repaint();
}

bool ModelViewer::loadFile(string fileName) {

    _file = fileName;

    if(!_mainModel.get())
        _mainModel = unique_ptr<Model>(new Model(_file));

    _modelLoaded = true;

    // Send the vertex data to the gpu
    loadVertices();

    // Scale the model to fit within screen dimensions
    _mainModel->fitToScreen(_zPos, _fov);

    return true;
}

void ModelViewer::loadVertices() {
    if(!_modelLoaded)
        return; // model has not yet been created

    _meshes = _mainModel->getMeshes();

    for(Model::Mesh& mesh : _meshes) {

        // Send vertex data to gpu
        glGenBuffers(1, &mesh.vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            mesh.vertices.size() * sizeof(glm::vec3),
            mesh.vertices.data(),
            GL_STATIC_DRAW
        );

        // Send uv data to gpu
        glGenBuffers(1, &mesh.uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            mesh.numFaces * 3 * sizeof(glm::vec2),
            mesh.uvs.data(),
            GL_STATIC_DRAW
        );

        // Send vertex normal data to gpu
        glGenBuffers(1, &mesh.normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBuffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            mesh.numFaces * 3 * sizeof(glm::vec3),
            mesh.normals.data(), 
            GL_STATIC_DRAW
        );
    }
}

void ModelViewer::processCameraMovements() {

    if(_keysPressed.empty())
        return; // no keys pressed

    // Arrow keys
    if(isKeyPressed(Qt::Key_Left)) {
        translate(-0.1, 0.0, 0.0);
    }
    if(isKeyPressed(Qt::Key_Right)) {
        translate(0.1, 0.0, 0.0);
    }
    if(isKeyPressed(Qt::Key_Up)) {
        translate(0.0, 0.1, 0.0);
    }
    if(isKeyPressed(Qt::Key_Down)) {
        translate(0.0, -0.1, 0.0);
    }

    // R: reset view
    if(isKeyPressed(Qt::Key_R)) {
        resetView();
    }
}

bool ModelViewer::isKeyPressed(int key) {
    return std::find(_keysPressed.begin(), _keysPressed.end(), key) != _keysPressed.end();
}

QSize ModelViewer::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize ModelViewer::sizeHint() const {
    return QSize(400, 400);
}

glm::vec3 ModelViewer::getArcballVector(int x, int y) {
    // Convert the point to [-1, 1] coordinates (where 0,0 is the center of the screen)
    glm::vec3 p = glm::vec3(
        float(x) / float(width()) * 2.0 - 1.0,
        -(float(y) / float(height()) * 2.0 - 1.0),
        0
    );

    // Now use the pythagorean theorem to check the length of OP and compute z coordinate
    float opSquared = p.x * p.x + p.y * p.y;
    if(opSquared <= 1)
        p.z = sqrt(1 - opSquared); // Pythagorean
    else
        p = glm::normalize(p); // nearest point

    return p;
}

void ModelViewer::translate(double x, double y, double z) {
    // Convert from camera to object coords
    glm::vec3 vec = glm::vec3(x, y, z);
    glm::vec3 translation = camToObj(vec);

    _mainModel->translate(translation.x, translation.y, translation.z);
}

void ModelViewer::mouseMoveEvent(QMouseEvent* event) {

    // We use the ArcBall approach to model rotations
    if(event->buttons() & Qt::LeftButton && _mainModel) {
        // Vector representing current orientation
        glm::vec3 va = getArcballVector(_lastPos.x(), _lastPos.y());
        // Vector representing desired orientation
        glm::vec3 vb = getArcballVector(event->x(), event->y());
        // The dot product of va and vb gives us a unit perpendicular vector 
        // (capped at 1.0 to protect against floating point errors)
        float angle = acos(std::min(1.0f, glm::dot(va, vb)));

        // Convert from camera coordinates to object coordinates
        glm::vec3 axisInCamCoords = glm::cross(va, vb);
        glm::vec3 rot = camToObj(axisInCamCoords);

        _mainModel->rotateRad(angle, rot.x, rot.y, rot.z);
    }

    _lastPos = event->pos();
}

void ModelViewer::mousePressEvent(QMouseEvent* event) {
    // Update the mouse's last position
    _lastPos = event->pos();
}

void ModelViewer::wheelEvent(QWheelEvent* event) {
    // Number of steps to zoom in/out
    zoom(event->delta() / 256.0);
    
}

void ModelViewer::keyPressEvent(QKeyEvent* event) {
    _keysPressed.push_back(event->key());
}

void ModelViewer::keyReleaseEvent(QKeyEvent* event) {
    _keysPressed.erase(std::find(_keysPressed.begin(), _keysPressed.end(), event->key()));
}

void ModelViewer::zoom(double numSteps) {
    _zPos -= numSteps;
    // Zoom factor is bound between 1.0 and 99.0
    _zPos = std::max(1.0, std::min(99.0, _zPos));

    // Recalculate view matrix
    _view = glm::lookAt(glm::vec3(_xPos, _yPos, _zPos), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    recalculateMVP();
}

glm::vec3 ModelViewer::camToObj(glm::vec3 vecInCamCoords) {
    glm::mat3 cam2Obj = glm::inverse(glm::mat3(_view) * glm::mat3(_model));

    return cam2Obj * vecInCamCoords;
}

void ModelViewer::resetView() {
    _view = glm::lookAt(
        _camPosition, 
        _camDirection, 
        _camUp
    );
    _mainModel->reset();
    _model = glm::mat4(1.0);
    _xPos = _yPos = 0.0;
    _zPos = 3.0;
    _lightPos = glm::vec3(0.0, 5.0, 0.0);
    recalculateMVP();
}

void ModelViewer::setViewMode(ViewMode mode) {
    _viewMode = mode;
}

ModelViewer::ViewMode ModelViewer::getViewMode() {
    return _viewMode;
}

void ModelViewer::recalculateMVP() {
    if(!_modelLoaded)
        return;

    _model = _mainModel->getModelMatrix(); // get model matrix from our model object
    _mvp = _projection * _view * _model;

    if(isInitialized()) {
        glUseProgram(_programId);
        glUniformMatrix4fv(_uniformMVPHandle, 1, GL_FALSE, glm::value_ptr(_mvp));
        glUseProgram(0);
    }

    _pendingMVPChange = false;
}

void ModelViewer::loadShader(char* shaderSource, GLenum shaderType, GLuint &programId) {
    GLuint shaderId = glCreateShader(shaderType);

    GLint result = GL_FALSE; // compilation result
    int infoLogLength;       // length of info log

    std::ifstream shaderFile(shaderSource);
    std::string shaderStr;
    const char* shader;

    if(!shaderFile.is_open()) {
        std::string error = "Error: could not read file ";
        throw std::exception(error.append(shaderSource).c_str());
    }

    // Read shader
    std::string buffer;
    while(std::getline(shaderFile, buffer)) {
        shaderStr += buffer + "\n";
    }

    shader = shaderStr.c_str();

    // Compile shader
    qDebug() << "Compiling shader";
    glShaderSource(shaderId, // Shader handle
                   1,        // Number of files
                   &shader,  // Shader source code
                   NULL);    // NULL terminated string
    glCompileShader(shaderId);

    // Check shader
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> errorMessage(infoLogLength);
    glGetShaderInfoLog(shaderId, infoLogLength, NULL, &errorMessage[0]);
    qDebug() << errorMessage.data();

    // Link the program
    qDebug() << "Linking program";
    glAttachShader(programId, shaderId);
    glLinkProgram(programId);

    // Check the program
    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> programErrorMessage(std::max(infoLogLength, int(1)));
    glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
    qDebug() << programErrorMessage.data();

    glDeleteShader(shaderId);

    shaderFile.close();
}

void ModelViewer::onMessageLogged(QOpenGLDebugMessage message) {
    // For logging OpenGL error messages
    if(message.severity() != QOpenGLDebugMessage::LowSeverity)
        qDebug() << message;
}

void ModelViewer::setLightingEnabled(bool enabled) {
    _lightingEnabled = enabled;

    glUseProgram(_programId);
    if(enabled)
        glUniform1f(_uniformLightingEnabledHandle, 1.0f);
    else
        glUniform1f(_uniformLightingEnabledHandle, 0.0f);
    glUseProgram(0);
}

void ModelViewer::setTexturingEnabled(bool enabled) {
    _texturingEnabled = enabled;

    glUseProgram(_programId);
    if(_texturingEnabled && isInitialized())
        glUniform1f(_uniformTexEnabledHandle, 1.0);
    else
        glUniform1f(_uniformTexEnabledHandle, 0.0);
    glUseProgram(0);
}