#include "Model.h"
#include "fstream"

// GLM
#include "gtc\matrix_transform.hpp"
// Assimp
#include "Resources\assimp\include\assimp\Importer.hpp"
#include "Resources\assimp\include\assimp\scene.h"
#include "Resources\assimp\include\assimp\postprocess.h"
// DevIL
#include "IL\ilu.h"

Model::Model() :
  _modelMatrixOutOfDate(true),
  _initialized(false),
  _numVertices(0),
  _modelMatrix(glm::mat4()),
  _translationMatrix(glm::mat4()),
  _scaleMatrix(glm::mat4()),
  _rotationMatrix(glm::mat4())
{
    ilInit(); // Initialize IL so we can import images
}

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

        curMesh.name = mesh->mName.data;

        // Load vertices
        for(int j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace* face = &mesh->mFaces[j];

            // Add each vertex of this mesh to our vertices
            for(int k = 0; k < face->mNumIndices; ++k) {
                int index = face->mIndices[k];
                aiVector3D temp = mesh->mVertices[index];
                glm::vec3 vertex(temp.x, temp.y, temp.z);

                _vertices.push_back(vertex);
                curMesh.vertices.push_back(vertex);
            }
        }

        // Load texture coordinates
        for(int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
            const aiVector3D* coords = mesh->mTextureCoords[j];
            if(!coords)
                continue;

            for(int k = 0; k < mesh->mNumVertices; ++k) {
                aiVector3D temp = coords[k];
                curMesh.uvs.push_back(glm::vec3(temp.x, temp.y, temp.z));
            }
        }

        // Check if this mesh has a material and load the material (and
        // relevant textures) if not previously loaded
        if(scene->HasMaterials()) {
            loadTextures(scene, mesh->mMaterialIndex);
        }

        // Find BBox and then add this mesh
        findBoundingBox(curMesh);
        _meshes.push_back(curMesh);
    }

    // Find BBox of the model as a whole
    model.vertices = _vertices;
    findBoundingBox(model);

    // Center the model
    translate(-(model.minX + model.maxX) / 2.0, -(model.minY + model.maxY) / 2.0, -(model.minZ + model.maxZ) / 2.0);

    // We will set the bounding sphere diameter to be the distance between two corners of the bounding box
    // This is a rough estimate, but it works well for the purpose of fitting the model to the viewport
    _boundingSphereRadius = 0.5 * distanceBetweenTwoPoints(
        glm::vec3(model.minX, model.minY, model.minZ),
        glm::vec3(model.maxX, model.maxY, model.maxZ)
        );
    _numVertices = _vertices.size();
    _initialized = true;

    return true;
}

void Model::loadTextures(const aiScene* scene, int matIndex) {
    const aiMaterial* materials = scene->mMaterials[matIndex];
    const aiMaterial* curMaterial = &materials[0];
    aiString name;
    aiString path;

    curMaterial->Get(AI_MATKEY_NAME, name);
    curMaterial->Get(AI_MATKEY_OPACITY, _opacity);

    if(std::find(_materials.begin(), _materials.end(), string(name.data)) != _materials.end()) {
        return; // material already loaded
    }

    _materials.push_back(name.data);

    // Check for textures and load them if found
    for(int j = 0; j < NUM_AI_TEXTURE_TYPES; ++j) {
        curMaterial->Get(AI_MATKEY_TEXTURE((aiTextureType)j, matIndex), path);

        if(path.length > 0) {
            loadTexture(string(path.data));
            path.Clear();
        }
    }

    //for(int i = 0; i < scene->mNumTextures; ++i) {
    //    aiTexture* tex = scene->mTextures[i];
    //    aiTexel* texel = tex->pcData;
    //    int w = tex->mWidth;
    //    int h = tex->mHeight;
    //    aiTexel ait = texel[i];
    //}
}

void Model::loadTexture(string fileName) {
    string fileNameWithPath = string(".\\").append(getPathFromFileName(_fileName).append(getFileNameFromPath(fileName)));

    ILenum error;
    bool success = ilLoadImage(fileNameWithPath.c_str());
    if(!success) {
        throw std::runtime_error("Could not read file");
    }

    ILubyte* data = ilGetData();
    int w = ilGetInteger(IL_IMAGE_WIDTH);
    int h = ilGetInteger(IL_IMAGE_HEIGHT);

    _textures.push_back(Texture(fileName, w, h, (char*)data));
}

double Model::distanceBetweenTwoPoints(glm::vec3 p1, glm::vec3 p2) {
    return hypot(hypot(p1.x - p2.x, p1.y - p2.y), p1.z - p2.z);
}

void Model::findBoundingBox(Mesh& mesh) {
    // Find the max/min x, y, and z values for this mesh
    for(auto vertex : mesh.vertices) {
        if(vertex.x > mesh.maxX)
            mesh.maxX = vertex.x;
        else if(vertex.x < mesh.minX)
            mesh.minX = vertex.x;

        if(vertex.y > mesh.maxY)
            mesh.maxY = vertex.y;
        else if(vertex.y < mesh.minY)
            mesh.minY = vertex.y;

        if(vertex.z > mesh.maxZ)
            mesh.maxZ = vertex.z;
        else if(vertex.z < mesh.minZ)
            mesh.minZ = vertex.z;
    }

    // Now, given these max/min values, find the vertices that compose the bounding box
    // These vertices are simply composed of every permutation of the max/min values for this mesh
    mesh.boundingBox.push_back(glm::vec3(mesh.minX, mesh.minY, mesh.minZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.minX, mesh.maxY, mesh.minZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.minX, mesh.minY, mesh.maxZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.minX, mesh.maxY, mesh.maxZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.maxX, mesh.minY, mesh.minZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.maxX, mesh.maxY, mesh.minZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.maxX, mesh.minY, mesh.maxZ));
    mesh.boundingBox.push_back(glm::vec3(mesh.maxX, mesh.maxY, mesh.maxZ));
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

string Model::getPathFromFileName(string fileName) {
    return fileName.substr(0, fileName.find_last_of("/\\") + 1);
}

string Model::getFileNameFromPath(string fileName) {
    return fileName.substr(fileName.find_last_of("/\\") + 1);
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

bool Model::initialized() {
    return _initialized;
}