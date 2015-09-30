#pragma once

#include "glm.hpp"
#include "QOpenGLFunctions_3_3_Core"
#include "IL\ilu.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class aiScene;
class aiNode;
class aiMesh;
class aiMaterial;

#define NUM_AI_TEXTURE_TYPES 0xC

class Model : protected QOpenGLFunctions_3_3_Core {

public:

    struct Material {
        string fileName;
    };

    struct Texture {
        string name;
        string fileName;
        int width;
        int height;
        char* data;
        float opacity;
        GLuint texId;
        ILuint ilTexId;
    };

    struct Mesh {
        string name;
        vector<glm::vec3> vertices;
        vector<glm::vec3> normals;
        vector<glm::vec2> uvs;

        GLuint vertexBuffer;
        GLuint uvBuffer;
        GLuint normalBuffer;

        int matIndex;
        int numFaces;
        int numVertices;
        int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
        vector<glm::vec3> boundingBox;

        Texture diffuseTexture;
        Texture specularTexture;
    };

    Model();
    Model(string fileName);
    ~Model();

    bool loadFile(string fileName);

    bool isModelMatrixOutOfDate();
    bool initialized();

    vector<glm::vec3> getVertices();
    //vector<glm::vec2> getTextureUVs();
    vector<Texture> getTextures();
    vector<Mesh> getMeshes();
    int getNumVertices();
    glm::mat4 getModelMatrix();

    void rotateRad(double angle, double x, double y, double z);
    void rotateDeg(double angle, double x, double y, double z);
    void translate(double x, double y, double z);
    void scale(double factor); // overload Model::scale(double, double, double)
    void scale(double x, double y, double z);

    // Call this to scale the model to fit within the screen upon load
    void fitToScreen(double zPos, double fovDegrees);
    void reset();

private:
    string _fileName;
    vector<string> _materials; // holds file names of relevent material files
    vector<Texture> _textures;
    glm::mat4 _modelMatrix;
    glm::mat4 _scaleMatrix;
    glm::mat4 _rotationMatrix;
    glm::mat4 _translationMatrix;

    vector<glm::vec3> _vertices;
    vector<glm::vec2> _uvs; // Texture UV coordinates
    vector<glm::vec3> _boundingBox;
    double _boundingSphereRadius;
    vector<Mesh> _meshes;
    int _numVertices;
    float _opacity;

    ILenum _ilError;

    bool _modelMatrixOutOfDate;
    bool _initialized;

    void loadNode(aiNode* node, const aiScene* scene);
    void loadMesh(aiMesh* mesh);
    void loadTextures(const aiScene* scene);
    void loadTexture(string fileName, Texture& texture);

    void findBoundingBox(Mesh& mesh);
    double distanceBetweenTwoPoints(glm::vec3 p1, glm::vec3 p2);

    void checkILError();
};

