#include "ModelViewer.h"

#include "gtc\matrix_transform.hpp"
#include "gtc\type_ptr.hpp"
#include "gtx\rotate_vector.hpp"

#include <fstream>

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
  _pendingMVPChange(false),
  _modelLoaded(false),
  _viewMode(ModelView),
  _file("")
{
    // Set OpenGL version
    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
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
    glDeleteBuffers(1, &_colorBuffer);
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteProgram(_programId);
}

void ModelViewer::initializeGL() {
    // Initialize OpenGL for Qt
    initializeOpenGLFunctions();

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Sample lighting?
    glm::vec4 lightPos(0, 0, 10, 1.0);
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(lightPos));

    // Load our shaders
    _programId = glCreateProgram();
    loadShader("shaders\\vertex.shader", GL_VERTEX_SHADER, _programId);
    loadShader("shaders\\fragment.shader", GL_FRAGMENT_SHADER, _programId);

    glUseProgram(_programId);

    // Get uniform handles
    _uniformMVPHandle = glGetUniformLocation(_programId, "mvp");

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);

    // Set model vertices
    //static const GLfloat data[] = {
    //    -0.5f, -0.5f, -0.5f, // triangle 1 : begin
    //    -0.5f, -0.5f, 0.5f,
    //    -0.5f, 0.5f, 0.5f, // triangle 1 : end
    //    0.5f, 0.5f, -0.5f, // triangle 2 : begin
    //    -0.5f, -0.5f, -0.5f,
    //    -0.5f, 0.5f, -0.5f, // triangle 2 : end
    //    0.5f, -0.5f, 0.5f,
    //    -0.5f, -0.5f, -0.5f,
    //    0.5f, -0.5f, -0.5f,
    //    0.5f, 0.5f, -0.5f,
    //    0.5f, -0.5f, -0.5f,
    //    -0.5f, -0.5f, -0.5f,
    //    -0.5f, -0.5f, -0.5f,
    //    -0.5f, 0.5f, 0.5f,
    //    -0.5f, 0.5f, -0.5f,
    //    0.5f, -0.5f, 0.5f,
    //    -0.5f, -0.5f, 0.5f,
    //    -0.5f, -0.5f, -0.5f,
    //    -0.5f, 0.5f, 0.5f,
    //    -0.5f, -0.5f, 0.5f,
    //    0.5f, -0.5f, 0.5f,
    //    0.5f, 0.5f, 0.5f,
    //    0.5f, -0.5f, -0.5f,
    //    0.5f, 0.5f, -0.5f,
    //    0.5f, -0.5f, -0.5f,
    //    0.5f, 0.5f, 0.5f,
    //    0.5f, -0.5f, 0.5f,
    //    0.5f, 0.5f, 0.5f,
    //    0.5f, 0.5f, -0.5f,
    //    -0.5f, 0.5f, -0.5f,
    //    0.5f, 0.5f, 0.5f,
    //    -0.5f, 0.5f, -0.5f,
    //    -0.5f, 0.5f, 0.5f,
    //    0.5f, 0.5f, 0.5f,
    //    -0.5f, 0.5f, 0.5f,
    //    0.5f, -0.5f, 0.5f
    //};

    //vector<GLfloat> vertices = vector<GLfloat>(data, data + sizeof data / sizeof data[0]);

    //_mainModel->setVertices(verts);

    //static const GLfloat colors[] = {
    //    0.583f, 0.771f, 0.014f,
    //    0.609f, 0.115f, 0.436f,
    //    0.327f, 0.483f, 0.844f,
    //    0.822f, 0.569f, 0.201f,
    //    0.435f, 0.602f, 0.223f,
    //    0.310f, 0.747f, 0.185f,
    //    0.597f, 0.770f, 0.761f,
    //    0.559f, 0.436f, 0.730f,
    //    0.359f, 0.583f, 0.152f,
    //    0.483f, 0.596f, 0.789f,
    //    0.559f, 0.861f, 0.639f,
    //    0.195f, 0.548f, 0.859f,
    //    0.014f, 0.184f, 0.576f,
    //    0.771f, 0.328f, 0.970f,
    //    0.406f, 0.615f, 0.116f,
    //    0.676f, 0.977f, 0.133f,
    //    0.971f, 0.572f, 0.833f,
    //    0.140f, 0.616f, 0.489f,
    //    0.997f, 0.513f, 0.064f,
    //    0.945f, 0.719f, 0.592f,
    //    0.543f, 0.021f, 0.978f,
    //    0.279f, 0.317f, 0.505f,
    //    0.167f, 0.620f, 0.077f,
    //    0.347f, 0.857f, 0.137f,
    //    0.055f, 0.953f, 0.042f,
    //    0.714f, 0.505f, 0.345f,
    //    0.783f, 0.290f, 0.734f,
    //    0.722f, 0.645f, 0.174f,
    //    0.302f, 0.455f, 0.848f,
    //    0.225f, 0.587f, 0.040f,
    //    0.517f, 0.713f, 0.338f,
    //    0.053f, 0.959f, 0.120f,
    //    0.393f, 0.621f, 0.362f,
    //    0.673f, 0.211f, 0.457f,
    //    0.820f, 0.883f, 0.371f,
    //    0.982f, 0.099f, 0.879f
    //};

    //vector<GLfloat> v = _mainModel->getVertices();
    //glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof GLfloat, v.data(), GL_STATIC_DRAW);

    //glGenBuffers(1, &_colorBuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
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
    //glEnable(GL_POLYGON_SMOOTH);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glUseProgram(_programId);
    glBindVertexArray(_vertexArray);

    // First attribute buffer - vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Second attribute buffer - colors
    //glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Set MVP
    glUniformMatrix4fv(_uniformMVPHandle, 1, GL_FALSE, glm::value_ptr(_mvp));

    glDrawArrays(_viewMode, 0, _mainModel->getNumVertices());

    // Clean up
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

    _mainModel = unique_ptr<Model>(new Model(_file));

    // Send the vertex data to the gpu
    vector<glm::vec3> vertices = _mainModel->getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        _mainModel->getNumVertices() * sizeof glm::vec3, 
        vertices.data(),
        GL_STATIC_DRAW
    );

    _modelLoaded = true;

    repaint();

    // Scale the model to fit within screen dimensions
    _mainModel->fitToScreen(_zPos, _fov);

    return true;
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
    _model = glm::mat4(1.0);
    _xPos = _yPos = 0.0;
    _zPos = 3.0;
    recalculateMVP();
}

void ModelViewer::recalculateMVP() {
    //_view = glm::lookAt(_camPosition, _camDirection, _camUp);
    if(!_mainModel)
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
    int infoLogLength; // length of info log

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
    //fprintf(stdout, "%s\n", &errorMessage[0]);

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
    //fprintf(stdout, "%s\n", &programErrorMessage[0]);

    glDeleteShader(shaderId);

    shaderFile.close();
}