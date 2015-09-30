#include "Model.h"
#include "Utils.h"
#include "QErrorMessage"
#include "fstream"

// GLM
#include "gtc\matrix_transform.hpp"
// Assimp
#include "Resources\assimp\include\assimp\Importer.hpp"
#include "Resources\assimp\include\assimp\scene.h"
#include "Resources\assimp\include\assimp\postprocess.h"
// DevIL
#include "IL\il.h"
#include "IL\ilut.h"

Model::Model() :
  _modelMatrixOutOfDate(true),
  _initialized(false),
  _numVertices(0),
  _modelMatrix(glm::mat4()),
  _translationMatrix(glm::mat4()),
  _scaleMatrix(glm::mat4()),
  _rotationMatrix(glm::mat4())
{
    // Initialize IL so we can import images
    ilInit(); 
    checkILError();
    initializeOpenGLFunctions();
}

Model::Model(string fileName) : Model() {
    _fileName = fileName;

    loadFile(fileName);
}

Model::~Model() {
    for(Texture tex : _textures) {
        glDeleteTextures(1, &tex.texId);
        ilDeleteImages(1, &tex.ilTexId);
    }
}

bool Model::loadFile(string fileName) {
    // Create the Assimp importer to import the file data
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName,
        //aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if(!scene)
        return false; // file could not be read

    // Recursively load each node in this model, starting with the root node
    loadNode(scene->mRootNode, scene);

    // Load the materials for this model
    if(scene->HasMaterials()) {
        loadTextures(scene);
    }

    // Find BBox of the model as a whole
    Mesh model; // represents complete model mesh
    model.vertices = _vertices;
    findBoundingBox(model);

    // Center the model
    translate( 
        -(model.minX + model.maxX) / 2.0, 
        -(model.minY + model.maxY) / 2.0, 
        -(model.minZ + model.maxZ) / 2.0
    );

    // We will set the bounding sphere diameter to be the distance between two corners of the bounding box
    // This is a rough estimate, but it works well for the purpose of fitting the model to the viewport
    _boundingSphereRadius = 0.5 * distanceBetweenTwoPoints(
        glm::vec3(model.minX, model.minY, model.minZ),
        glm::vec3(model.maxX, model.maxY, model.maxZ)
    );
    _initialized = true;

    return true;
}

void Model::loadNode(aiNode* node, const aiScene* scene) {
    // Load all the meshes in this node
    for(int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        loadMesh(mesh);
    }

    // Recursively load each child node 
    for(int i = 0; i < node->mNumChildren; ++i)
        loadNode(node->mChildren[i], scene);
}

void Model::loadMesh(aiMesh* mesh) {
    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    Mesh m;
    m.matIndex = mesh->mMaterialIndex;
    m.numFaces = mesh->mNumFaces;

    for(int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for(int j = 0; j < face.mNumIndices; ++j) {
            int index = face.mIndices[j];

            // Vertices
            if(mesh->HasPositions()) {
                glm::vec3 vertex(
                    mesh->mVertices[index].x, 
                    mesh->mVertices[index].y, 
                    mesh->mVertices[index].z
                );
                _vertices.push_back(vertex);
                m.vertices.push_back(vertex);
            }

            // Normals
            if(mesh->HasNormals()) {
                glm::vec3 normal(
                    mesh->mNormals[index].x,
                    mesh->mNormals[index].y,
                    mesh->mNormals[index].z
                );
                m.normals.push_back(normal);
            }

            // UVs
            if(mesh->HasTextureCoords(0) && mesh->mTextureCoords[0]) {
                aiVector3D texCoord = mesh->mTextureCoords[0][index];
                glm::vec2 uv(texCoord.x, texCoord.y);
                m.uvs.push_back(uv);
            }
        }
    }

    _numVertices += mesh->mNumVertices; // add to total number of vertices
    findBoundingBox(m); 
    // Add this mesh to our vector of meshes
    _meshes.push_back(m);
}

// TODO refactor this method
void Model::loadTextures(const aiScene* scene) {

    //for(int i = 0; i < scene->mNumMaterials; ++i) {
    for(Mesh &mesh : _meshes) {
        // Iterate through each kind of AI texture
        for(int j = 0; j < NUM_AI_TEXTURE_TYPES; ++j) {

            const aiMaterial* material = scene->mMaterials[mesh.matIndex];
            unsigned int numTex = material->GetTextureCount((aiTextureType)j);

            // Check for textures and load them if found
            for(int k = 0; k < numTex; ++k) {
                Texture curTex;
                aiString texPath;
                aiString texName;

                material->Get(AI_MATKEY_TEXTURE((aiTextureType)j, k), texPath);
                material->Get(AI_MATKEY_OPACITY, curTex.opacity);
                material->Get(AI_MATKEY_NAME, texName);

                if(texPath.length > 0) {
                    bool skip = false;
                    for(Texture t : _textures) {
                        if(t.fileName == texPath.data) {
                            skip = true;
                            mesh.diffuseTexture.texId = t.texId;
                        }
                    }
                    if(skip) {
                        continue;
                    }

                    curTex.fileName = texPath.data;
                    curTex.name = texName.data;

                    try {
                        loadTexture(curTex.fileName, curTex);
                    }
                    catch(std::runtime_error) {
                        QErrorMessage errorBox;
                        string msg = "Error: ";
                        msg.append(curTex.fileName).append(" could not be loaded");
                        errorBox.showMessage(msg.c_str());
                        continue;
                    }
                    mesh.diffuseTexture.texId = curTex.texId;
                    _textures.push_back(curTex);
                    break;
                }
            }
        }
    }
    //_materials.push_back(name.data);
}

void Model::loadTexture(string fileName, Texture& texture) {
    //string fileNameWithPath = getPathFromFileName(_fileName).append(getFileNameFromPath(fileName));
    string fileNameWithPath = Utils::getPathFromFileName(_fileName).append(Utils::getFileNameFromPath(fileName));

    // Create the DevIL texture id
    ilGenImages(1, &texture.ilTexId);
    checkILError();
    ilBindImage(texture.ilTexId);
    checkILError();

    // Load the image
    bool success = ilLoadImage(fileNameWithPath.c_str());
    if(!success) {
        throw std::runtime_error("Could not read file");
    }

    // Switch the renderer
    ilutRenderer(ILUT_OPENGL);
    checkILError();

    // This will generate an OpenGL texture and send the data to the gpu for us
    texture.texId = ilutGLBindTexImage();
    checkILError();

    glBindTexture(GL_TEXTURE_2D, texture.texId);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    texture.data = (char*)ilGetData();
    texture.width = ilGetInteger(IL_IMAGE_WIDTH);
    texture.height = ilGetInteger(IL_IMAGE_HEIGHT);
}

double Model::distanceBetweenTwoPoints(glm::vec3 p1, glm::vec3 p2) {
    return hypot(hypot(p1.x - p2.x, p1.y - p2.y), p1.z - p2.z);
}

// TODO find a better way to find bbox
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
    double rMax = zPos * sin(glm::radians(fovDegrees));
    // How much we need to scale by to fit the model in the screen
    double scaleFactor = rMax / _boundingSphereRadius;
    scale(scaleFactor);
}

vector<glm::vec3> Model::getVertices() {
    return _vertices;
}

vector<Model::Texture> Model::getTextures() {
    return _textures;
}

vector<Model::Mesh> Model::getMeshes() {
    return _meshes;
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

void Model::reset() {
    //_translationMatrix = glm::mat4(1.0);
    _rotationMatrix = glm::mat4(1.0);

    _modelMatrixOutOfDate = true;
}

void Model::checkILError() {
    _ilError = ilGetError();
    if(_ilError != IL_NO_ERROR) {
        throw std::exception(iluGetString(_ilError));
    }
}