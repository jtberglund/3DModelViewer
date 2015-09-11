#pragma once

#include "glm.hpp"
#include "QOpenGLFunctions_3_3_Core"
#include <vector>
#include <string>

using std::vector;
using std::string;

class Model : protected QOpenGLFunctions_3_3_Core {

    struct Mesh {
        string name;
        vector<glm::vec3> vertices;
        vector<glm::vec3> boundingBox;
        double minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;

        Mesh() {}

        Mesh(vector<glm::vec3> vertices) : vertices(vertices) {}

    };

public:
    Model();
    Model(string fileName);
    ~Model();

    bool loadFile(string fileName);

    bool isModelMatrixOutOfDate();

    vector<glm::vec3> getVertices();
    int getNumVertices();
    glm::mat4 getModelMatrix();

    void rotateRad(double angle, double x, double y, double z);
    void rotateDeg(double angle, double x, double y, double z);
    void translate(double x, double y, double z);
    void scale(double factor); // overload Model::scale(double, double, double)
    void scale(double x, double y, double z);

    // Call this to scale the model to fit within the screen upon load
    void fitToScreen(double zPos, double fovDegrees);

private:
    string _fileName;
    glm::mat4 _modelMatrix;
    glm::mat4 _scaleMatrix;
    glm::mat4 _rotationMatrix;
    glm::mat4 _translationMatrix;

    vector<glm::vec3> _vertices;
    vector<glm::vec3> _boundingBox;
    double _boundingSphereRadius;
    vector<Mesh> _meshes;
    int _numVertices;

    bool _modelMatrixOutOfDate;
    bool _initialized;

    void findBoundingBox(Mesh& mesh);
    double distanceBetweenTwoPoints(glm::vec3 p1, glm::vec3 p2);
};

