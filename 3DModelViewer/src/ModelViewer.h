#pragma once

#include "QtOpenGL"
#include "QOpenGLFunctions_3_3_Core"

#include "Model.h"

#include "glm.hpp"

#include "memory"

using std::vector;
using std::string;
using std::unique_ptr;

class ModelViewer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    enum ViewMode {
        PointCloud = GL_POINT,      // View model as point cloud
        WireFrame = GL_LINE_STRIP,  // View model as wireframe/mesh
        ModelView = GL_TRIANGLES    // Normal viewing mode
    };

    ModelViewer(QWidget* parent = 0);
    ~ModelViewer();

    bool loadFile(string fileName);

    // Reset the position of the model in the view
    void resetView();
    void zoom(double numSteps);
    void setViewMode(ViewMode mode);
    void setTexturingEnabled(bool enabled);
    void setLightingEnabled(bool enabled);
    ViewMode getViewMode();

public slots:
    void onMessageLogged(QOpenGLDebugMessage message);

protected:
    // Set up OpenGL (create program, gen buffers, etc)
    void initializeGL() override;
    // Perform the actual rendering on the scene
    void paintGL() override;
    // Called on window resize to resize the viewport
    void resizeGL(int width, int height) override;
    // Performs any non-drawing operations once per frame
    void updateGL();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    // Event handlers
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    // OpenGL IDs
    GLuint _programId;
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    vector<GLuint> _texIds;

    unique_ptr<Model> _mainModel;
    vector<Model::Mesh> _meshes;
    string _file;
    ViewMode _viewMode;
    QOpenGLDebugLogger* _logger;

    // Uniform handles
    GLuint _uniformMVPHandle;
    GLuint _uniformTexSamplerHandle;
    GLuint _uniformModelHandle;
    GLuint _uniformTexEnabledHandle;
    GLuint _uniformLightingHandle;
    GLuint _uniformLightPosHandle;
    GLuint _uniformLightingEnabledHandle;
    GLuint _uniformViewPosHandle;

    // Lighting
    glm::vec3 _lightColor;
    glm::vec3 _lightPos;

    // MVP matrices
    glm::mat4 _projection;
    glm::mat4 _view;
    glm::mat4 _model;
    glm::mat4 _mvp;

    glm::vec3 _camPosition; // Camera position in world space
    glm::vec3 _camDirection; // Direction the camera is looking
    glm::vec3 _camUp; // Up vector

    double _xPos;
    double _yPos;
    double _zPos;
    double _fov;

    // Signifies that the MVP matrix must be recalculated this frame
    bool _pendingMVPChange; 
    // False until a model has been loaded using ModelViewer::loadFile(string)
    bool _modelLoaded; 
    bool _lightingEnabled;
    bool _texturingEnabled;

    QPoint _lastPos; // Last mouse position
    // Holds all keys currently being pressed
    vector<int> _keysPressed; 

    // Handles all camera movements each frame
    void processCameraMovements(); 
    // Recalculates MVP matrix (projection * view * model)
    void recalculateMVP();
    // Compile shader
    void loadShader(char* shaderSource, GLenum shaderType, GLuint &programId);
    // Called to load the model vertices into memory
    void loadVertices();
    // Returns true if _keysPressed contains the key passed in 
    bool isKeyPressed(int key);
    // Translate the model
    void translate(double x, double y, double z);
    // Returns a normalized vector from the center of the arcball to the point at (x, y)
    glm::vec3 getArcballVector(int x, int y);
    // Convert from camera coordinates to object coordinates
    glm::vec3 camToObj(glm::vec3 axisInCamCoords);

    // Used for debugging opengl only 
    inline void getError() {
        const GLenum error = glGetError();
        if(error != GL_NO_ERROR)
            printf("----------------------------- %i ----------------------", error);
    }
};

