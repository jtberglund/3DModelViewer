#include "Model.h"

// GLM
#include "gtc\matrix_transform.hpp"
// Assimp
#include "Resources\assimp\include\assimp\Importer.hpp"
#include "Resources\assimp\include\assimp\scene.h"
#include "Resources\assimp\include\assimp\postprocess.h"

Model::Model() :
  _modelMatrixOutOfDate(true),
  _initialized(false),
  _numVertices(0),
  _modelMatrix(glm::mat4()),
  _translationMatrix(glm::mat4()),
  _scaleMatrix(glm::mat4()),
  _rotationMatrix(glm::mat4())
{}

Model::Model(string fileName) : Model() {
    _fileName = fileName;

    loadFile(fileName);
}

Model::~Model() {}

bool Model::loadFile(string fileName) {
    // Create the Assimp importer to import the file data
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName,
            aiProcess_CalcTangentSpace | 
            aiProcess_Triangulate | 
            aiProcess_JoinIdenticalVertices | 
            aiProcess_SortByPType);

    if(!scene)
        return false; // file could not be read

    Mesh model; // represents complete model mesh

    // Iterate through each mesh in the model
    for(int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        Mesh curMesh;
        // Bounding box for this mesh
        double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0; 

        vector<glm::vec3> v;
        for(int j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace* face = &mesh->mFaces[j];

            // Add each vertex of this mesh to our vertices
            for(int k = 0; k < face->mNumIndices; ++k) {
                int index = face->mIndices[k];  
                aiVector3D temp = mesh->mVertices[index];
                glm::vec3 tempV(temp.x, temp.y, temp.z);

                isPartOfBoundingBox(curMesh, tempV);
                isPartOfBoundingBox(model, tempV);

                _vertices.push_back(glm::vec3(temp.x, temp.y, temp.z));
                v.push_back(glm::vec3(temp.x, temp.y, temp.z));
            }
        }

        // Add this mesh
        curMesh.vertices = v;
        _meshes.push_back(Mesh(v));
    }

    calculateBoundingBox(_boundingBox, model.minX, model.maxX, model.minY, model.maxY, model.minZ, model.maxZ);
    
    // Center the model
    translate(-(model.minX + model.maxX) / 2.0, -(model.minY + model.maxY) / 2.0, -(model.minZ + model.maxZ) / 2.0);
    
    // We will set the bounding sphere diameter to be the distance between two corners of the bounding box
    _boundingSphereRadius = distanceBetweenTwoPoints(glm::vec3(model.minX, model.minY, model.minZ), glm::vec3(model.maxX, model.maxY, model.maxZ)) / 2.0;

    _numVertices = _vertices.size();
    _initialized = true;

    return true;
}

double Model::distanceBetweenTwoPoints(glm::vec3 p1, glm::vec3 p2) {
    return hypot(hypot(p1.x - p2.x, p1.y - p2.y), p1.z - p2.z);
}

bool Model::isPartOfBoundingBox(Mesh& mesh, glm::vec3& vertex) {
    if(vertex.x > mesh.maxX)
        mesh.maxX = vertex.x;
    if(vertex.y > mesh.maxY)
        mesh.maxY = vertex.y;
    if(vertex.z > mesh.maxZ)
        mesh.maxZ = vertex.z;

    if(vertex.x < mesh.minX)
        mesh.minX = vertex.x;
    if(vertex.y < mesh.minY)
        mesh.minY = vertex.y;
    if(vertex.z < mesh.minZ)
        mesh.minZ = vertex.z;

    return true;
}

void Model::calculateBoundingBox(vector<glm::vec3>& bBox, double minX, double maxX, double minY, double maxY, double minZ, double maxZ) {
    double x[] = {minX, maxX};
    double y[] = {minY, maxY};
    double z[] = {minZ, maxZ};

    //for(int i = 0; i < 2; ++i) {
       bBox.push_back(glm::vec3(x[0], y[0], z[0]));
       bBox.push_back(glm::vec3(x[0], y[0], z[1]));
       bBox.push_back(glm::vec3(x[0], y[1], z[0]));
       bBox.push_back(glm::vec3(x[0], y[1], z[1]));
       bBox.push_back(glm::vec3(x[1], y[0], z[0]));
       bBox.push_back(glm::vec3(x[1], y[0], z[1]));
       bBox.push_back(glm::vec3(x[1], y[1], z[0]));
       bBox.push_back(glm::vec3(x[1], y[1], z[1]));
    //}
}

void Model::fitToScreen(double zPos, double fovDegrees) {
    if(!_initialized)
        return;

    // This is the maximum radius we can have for this FOV
    auto rMax = zPos * sin(glm::radians(fovDegrees));
    // How much we need to scale by to fit the model in the screen
    double scaleFactor = rMax / _boundingSphereRadius;
    scale(scaleFactor);
}

vector<glm::vec3> Model::getVertices() {
    return _vertices;
}

int Model::getNumVertices() {
    return _numVertices;
}

glm::mat4 Model::getModelMatrix() {
    _modelMatrix = _scaleMatrix * _rotationMatrix * _translationMatrix;
    return _modelMatrix;
}

void Model::rotateDeg(double angle, double x, double y, double z) {
    rotateRad(glm::degrees(angle), x, y, z);
}

void Model::rotateRad(double angle, double x, double y, double z) {
    _rotationMatrix = glm::rotate(_rotationMatrix, float(angle), glm::vec3(x, y, z));

    _modelMatrixOutOfDate = true;
}

void Model::translate(double x, double y, double z) {
    _translationMatrix = glm::translate(_translationMatrix, glm::vec3(x, y, z));

    _modelMatrixOutOfDate = true;
}

void Model::scale(double factor) {
    scale(factor, factor, factor);
}

void Model::scale(double x, double y, double z) {
    _scaleMatrix = glm::scale(_scaleMatrix, glm::vec3(x, y, z));

    _modelMatrixOutOfDate = true;
}

bool Model::isModelMatrixOutOfDate() {
    return _modelMatrixOutOfDate;
}