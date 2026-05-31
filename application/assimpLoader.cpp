#include "assimpLoader.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../framework/object.h"
#include "../mesh/mesh.h"
#include "../renderer/renderer.h"
#include "third_party/assimp/Importer.hpp"
#include "third_party/assimp/postprocess.h"
#include "third_party/assimp/scene.h"
#include "tools/tools.h"

using namespace GLframework;
using namespace GL_APPLICATION;

namespace
{
	glm::mat4 getMat4f(aiMatrix4x4 value);

	std::shared_ptr<Mesh> processMesh(
		std::shared_ptr<Renderer> renderer,
		aiMesh* aimesh,
		const aiScene* scene,
		const std::string& rootPath,
		const AssimpMaterialImportOptions& materialOptions
	);

	void processNode(
		std::shared_ptr<Renderer> renderer,
		aiNode* ainode,
		std::shared_ptr<Object> parent,
		const aiScene* scene,
		const std::string& rootPath,
		const AssimpMaterialImportOptions& materialOptions
	);
}

std::shared_ptr<Object> GL_APPLICATION::AssimpLoader::load(
	const std::string& path,
	std::shared_ptr<Renderer> renderer
)
{
	return load(path, renderer, {});
}

std::shared_ptr<Object> GL_APPLICATION::AssimpLoader::loadPBR(
	const std::string& path,
	std::shared_ptr<Renderer> renderer
)
{
	AssimpMaterialImportOptions materialOptions{};
	materialOptions.mode = AssimpMaterialImportMode::PBRMetallicRoughness;
	return load(path, renderer, materialOptions);
}

std::shared_ptr<Object> GL_APPLICATION::AssimpLoader::load(
	const std::string& path,
	std::shared_ptr<Renderer> renderer,
	const AssimpMaterialImportOptions& materialOptions
)
{
	//取出模型所在目录
	std::size_t lastIndex = path.find_last_of("//");
	auto rootPath = path.substr(0, lastIndex + 1);
	std::shared_ptr<Object> rootNode = std::make_shared<Object>();
	Assimp::Importer importer;

	//aiProcess_Triangulatë	:	四边面三角化
	//aiProcess_GenNormals	：	为面生成法线
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

	//验证读取是否正确
	if(!scene||scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode)
	{
		std::cerr << "Error: Model Read Failed!" << std::endl;
		return nullptr;
	}

	processNode(renderer, scene->mRootNode, rootNode, scene, rootPath, materialOptions);

	return rootNode;
}

namespace
{
void processNode(
	std::shared_ptr<Renderer> renderer,
	aiNode* ainode,
	std::shared_ptr<Object> parent,
	const aiScene* scene,
	const std::string& rootPath,
	const AssimpMaterialImportOptions& materialOptions
)
{
	std::shared_ptr<Object> node = std::make_shared<Object>();
	parent->addChild(node);
	glm::mat4 localMatrix = getMat4f(ainode->mTransformation);
	// 解析位置、旋转、缩放的信息
	glm::vec3 position, eulerAngle, scale;
	Tools::decompose(localMatrix, position, eulerAngle, scale);

	node->setPosition(position);
	node->setAngleX(eulerAngle.x);
	node->setAngleY(eulerAngle.y);
	node->setAngleZ(eulerAngle.z);
	node->setScale(scale);

	// 检查并解析mesh
	for(unsigned int i = 0;i<ainode->mNumMeshes;i++)
	{
		// 记住ainode中的mesh储存的是mesh的ID
		// 而aiscene中的mesh才是真正的mesh，这使得同一个mesh可以让多个node使用
		int meshID = ainode->mMeshes[i];
		aiMesh* aimesh = scene->mMeshes[meshID];
		//然后将aimesh转换为我们自己的mesh
		auto mesh = processMesh(renderer, aimesh, scene, rootPath, materialOptions);
		node->addChild(mesh);
	}

	for(unsigned int i = 0;i<ainode->mNumChildren;i++)
	{
		processNode(renderer, ainode->mChildren[i], node, scene, rootPath, materialOptions);
	}
}

std::shared_ptr<Mesh> processMesh(
	std::shared_ptr<Renderer> renderer,
	aiMesh* aimesh,
	const aiScene* scene,
	const std::string& rootPath,
	const AssimpMaterialImportOptions& materialOptions
)
{
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;

	for(unsigned int i = 0;i<aimesh->mNumVertices;i++)
	{
		// position
		positions.push_back(aimesh->mVertices[i].x);
		positions.push_back(aimesh->mVertices[i].y);
		positions.push_back(aimesh->mVertices[i].z);

		// normal
		normals.push_back(aimesh->mNormals[i].x);
		normals.push_back(aimesh->mNormals[i].y);
		normals.push_back(aimesh->mNormals[i].z);

		// uv
		// 关注第0套uv，一般情况下是贴图uv
		if(aimesh->mTextureCoords[0]) {
			uvs.push_back(aimesh->mTextureCoords[0][i].x);
			uvs.push_back(aimesh->mTextureCoords[0][i].y);
		}else
		{
			uvs.push_back(0.0f);
			uvs.push_back(0.0f);

		}

	}
	for(unsigned int i = 0;i<aimesh->mNumFaces;++i)
	{
		for(unsigned int j = 0;j<aimesh->mFaces[i].mNumIndices;++j)
		{
			indices.push_back(aimesh->mFaces[i].mIndices[j]);
		}
	}

	auto material = AssimpMaterialImporter::createMaterial(aimesh, scene, rootPath, materialOptions);
	auto geometry = std::make_shared<Geometry>(renderer->getShader(material->getMaterialType()), positions, normals, uvs, indices);
	return std::make_shared<Mesh>(geometry, material);
}

//transform Assimp::Mat4 to glm::Mat4
glm::mat4 getMat4f(aiMatrix4x4 value)
{
	glm::mat4 res(
		value.a1, value.a2, value.a3, value.a4,
		value.b1, value.b2, value.b3, value.b4,
		value.c1, value.c2, value.c3, value.c4,
		value.d1, value.d2, value.d3, value.d4
	);
	return res;
}
}
