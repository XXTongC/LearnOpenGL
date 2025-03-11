#include "assimpInstanceLoader.h"

#include "assimpInstanceLoader.h"
#include "tools.h"
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"

using namespace GLframework;
using namespace GL_APPLICATION;

void AssimpInstanceLoader::setInstanceMaterial(std::shared_ptr<GLframework::Object> obj, std::shared_ptr < GLframework::Material > material)
{
	if (obj->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(obj);
		im->setMaterial(material);
	}

	for (auto& child : obj->getChildren())
	{
		setInstanceMaterial(child, material);
	}
}

void AssimpInstanceLoader::setInstanceMatrix(std::shared_ptr<GLframework::Object> obj, unsigned index, glm::mat4 matrix)
{
	if(obj->getType()==ObjectType::InstancedMesh)
	{
		std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(obj);
		im->mInstanceMatrices[index] = matrix;
	}

	for(auto& child : obj->getChildren())
	{
		setInstanceMatrix(child,index,matrix);
	}
}

void AssimpInstanceLoader::updateInstanceMatrix(std::shared_ptr<GLframework::Object> obj)
{
	if (obj->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(obj);
		im->updateMatrices();
	}

	for (auto& child : obj->getChildren())
	{
		updateInstanceMatrix(child);
	}
}


std::shared_ptr<Object> GL_APPLICATION::AssimpInstanceLoader::load(
	const std::string& path,
	std::shared_ptr<Renderer> renderer,
	const unsigned int& instanceCount)
{
	//取出模型所在目录
	std::size_t lastIndex = path.find_last_of("//");
	auto rootPath = path.substr(0, lastIndex + 1);
	//std::cout << rootPath << std::endl;
	std::shared_ptr<Object> rootNode = std::make_shared<Object>();
	Assimp::Importer importer;

	//aiProcess_Triangulatë	:	四边面三角化
	//aiProcess_GenNormals	：	为面生成法线
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);


	//验证读取是否正确
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "Error: Model Read Failed!" << std::endl;
		return nullptr;
	}

	processNode(renderer, scene->mRootNode, rootNode, scene, rootPath,instanceCount);

	return rootNode;
}

void AssimpInstanceLoader::processNode(
	std::shared_ptr<Renderer> renderer,
	aiNode* ainode, 
	std::shared_ptr<Object> parent, 
	const aiScene* scene,
	const std::string& rootPath,
	const unsigned int& instanceCount)
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
	for (int i = 0; i < ainode->mNumMeshes; i++)
	{
		// 记住ainode中的mesh储存的是mesh的ID
		// 而aiscene中的mesh才是真正的mesh，这使得同一个mesh可以让多个node使用
		int meshID = ainode->mMeshes[i];
		aiMesh* aimesh = scene->mMeshes[meshID];
		//然后将aimesh转换为我们自己的mesh
		auto mesh = processInstanceMesh(renderer, aimesh, scene, rootPath,instanceCount);
		node->addChild(mesh);
	}

	for (int i = 0; i < ainode->mNumChildren; i++)
	{
		processNode(renderer, ainode->mChildren[i], node, scene, rootPath,instanceCount);
	}
}

std::shared_ptr<InstancedMesh> AssimpInstanceLoader::processInstanceMesh(
	std::shared_ptr<Renderer> renderer,
	aiMesh* aimesh,
	const aiScene* scene, 
	const std::string& rootPath,
	const unsigned int& instanceCount)
{
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;
	std::vector<float> colors;

	for (int i = 0; i < aimesh->mNumVertices; i++)
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
		if (aimesh->mTextureCoords[0]) {
			uvs.push_back(aimesh->mTextureCoords[0][i].x);
			uvs.push_back(aimesh->mTextureCoords[0][i].y);
		}
		else
		{
			uvs.push_back(0.0f);
			uvs.push_back(0.0f);
		}

		// color
		if(aimesh->HasVertexColors(0))
		{
			colors.push_back(aimesh->mColors[0][i].r);
			colors.push_back(aimesh->mColors[0][i].g);
			colors.push_back(aimesh->mColors[0][i].b);
		}

	}
	for (int i = 0; i < aimesh->mNumFaces; ++i)
	{
		aiFace aiface = aimesh->mFaces[i];
		for (int j = 0; j < aiface.mNumIndices; ++j)
		{
			indices.push_back(aiface.mIndices[j]);
		}
	}
	auto material = std::make_shared<GrassInstanceMaterial>();
	//material->mDiffuse = std::make_shared<Texture>("Texture/box.png", 0);
	//material->setDepthWrite(false);

	if (aimesh->mMaterialIndex >= 0)
	{
		unsigned int unit = 0;
		std::shared_ptr<Texture> texture = nullptr;
		aiMaterial* aiMat = scene->mMaterials[aimesh->mMaterialIndex];
		// 1. load diffuse
		texture = processTexture(aiMat, aiTextureType_DIFFUSE, scene, rootPath,instanceCount);

		if (texture == nullptr)
		{

			material->mDiffuse = Texture::createTexture("Texture/defaultTexture.jpg", 0);

		}
		else {
			texture->setUnit(unit++);
			material->mDiffuse = texture;
		}
		// 2. load specular
		auto specularMask = processTexture(aiMat, aiTextureType_SPECULAR, scene, rootPath,instanceCount);
		if (specularMask == nullptr)
		{
			material->mSpecularMask = Texture::createTexture("Texture/defaultTexture.jpg", 1);
		}
		else
		{
			specularMask->setUnit(unit++);
			material->mSpecularMask = specularMask;
		}
	}

	auto geometry = std::make_shared<Geometry>(renderer->getShader(material->getMaterialType()), positions, normals, uvs,colors, indices);
	return std::make_shared<InstancedMesh>(geometry, material,instanceCount);
}

std::shared_ptr<Texture> AssimpInstanceLoader::processTexture(
	const aiMaterial* aiMat,
	const aiTextureType& type,
	const aiScene* scene, 
	const std::string& rootPath,
	const unsigned int& instanceCount)
{
	std::shared_ptr<Texture> texture = nullptr;
	//获取图片读取路径
	aiString aipath;
	aiMat->Get(AI_MATKEY_TEXTURE(type, 0), aipath);
	if (aipath.length == 0)
	{
		return nullptr;
	}
	//判断是否是嵌入fbx的图片
	const aiTexture* aitexture = scene->GetEmbeddedTexture(aipath.C_Str());
	//相当于取一次内嵌，如果为空则不是内嵌...
	if (aitexture)
	{
		//内嵌
		unsigned char* dataIn = reinterpret_cast<unsigned char*>(aitexture->pcData);
		uint32_t widthIn = aitexture->mWidth;
		uint32_t heightIn = aitexture->mHeight;
		texture = Texture::createTextureFromMemory(aipath.C_Str(), 0, dataIn, widthIn, heightIn);

	}
	else
	{
		//非内嵌
		std::string fullPath = rootPath + aipath.C_Str();
		//std::cout << fullPath << std::endl;
		texture = Texture::createTexture(fullPath, 0);
	}
	return texture;
}

//transform Assimp::Mat4 to glm::Mat4
glm::mat4 AssimpInstanceLoader::getMat4f(aiMatrix4x4 value)
{
	glm::mat4 res(
		value.a1, value.a2, value.a3, value.a4,
		value.b1, value.b2, value.b3, value.b4,
		value.c1, value.c2, value.c3, value.c4,
		value.d1, value.d2, value.d3, value.d4
	);
	return res;
}
