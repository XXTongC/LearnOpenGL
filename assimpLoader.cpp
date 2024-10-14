#include "assimpLoader.h"
#include "tools.h"
#include "phongMaterial.h"
using namespace GLframework;
using namespace GL_APPLICATION;

std::shared_ptr<Object> GL_APPLICATION::AssimpLoader::load(const std::string& path, std::shared_ptr<Renderer> renderer)
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
	if(!scene||scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode)
	{
		std::cerr << "Error: Model Read Failed!" << std::endl;
		return nullptr;
	}

	processNode(renderer,scene->mRootNode,rootNode,scene,rootPath);

	return rootNode;
}

void AssimpLoader::processNode(std::shared_ptr<Renderer> renderer,aiNode* ainode,std::shared_ptr<Object> parent,const aiScene* scene, const std::string& rootPath)
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
	for(int i = 0;i<ainode->mNumMeshes;i++)
	{
		// 记住ainode中的mesh储存的是mesh的ID
		// 而aiscene中的mesh才是真正的mesh，这使得同一个mesh可以让多个node使用
		int meshID = ainode->mMeshes[i];
		aiMesh* aimesh = scene->mMeshes[meshID];
		//然后将aimesh转换为我们自己的mesh
		auto mesh = processMesh(renderer,aimesh,scene,rootPath);
		node->addChild(mesh);
	}

	for(int i = 0;i<ainode->mNumChildren;i++)
	{
		processNode(renderer,ainode->mChildren[i],node,scene,rootPath);
	}
}

std::shared_ptr<Mesh> AssimpLoader::processMesh(std::shared_ptr<Renderer> renderer,aiMesh* aimesh,const aiScene* scene, const std::string& rootPath)
{
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;

	for(int i = 0;i<aimesh->mNumVertices;i++)
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
	for(int i = 0;i<aimesh->mNumFaces;++i)
	{
		for(int j = 0;j<aimesh->mFaces[i].mNumIndices;++j)
		{
			indices.push_back(aimesh->mFaces[i].mIndices[j]);
		}
	}
	auto material = std::make_shared<PhongMaterial>();
	//material->mDiffuse = std::make_shared<Texture>("Texture/box.png", 0);
	

	if(aimesh->mMaterialIndex>=0)
	{
		unsigned int unit = 0;
		std::shared_ptr<Texture> texture = nullptr;
		aiMaterial* aiMat = scene->mMaterials[aimesh->mMaterialIndex];
		// 1. load diffuse
		texture = processTexture(aiMat, aiTextureType_DIFFUSE, scene, rootPath);
		texture->setUnit(unit++);
		if(texture==nullptr)
		{
			material->mDiffuse = Texture::createTexture("Texture/defaultTexture.jpg", 0);

		}else
		{
			material->mDiffuse = texture;
		}
		// 2. load specular
		auto specularMask = processTexture(aiMat, aiTextureType_SPECULAR, scene, rootPath);
		specularMask->setUnit(unit++);
		if(specularMask==nullptr)
		{
			material->mSpecularMask = Texture::createTexture("Texture/defaultTexture.jpg", 0);
		}else
		{
			material->mSpecularMask = specularMask;
		}
	}
	
	auto geometry = std::make_shared<Geometry>(renderer->getShader(material->getMaterialType()), positions, normals, uvs, indices);
	return std::make_shared<Mesh>(geometry,material);
}

std::shared_ptr<Texture> AssimpLoader::processTexture(const aiMaterial* aiMat, const aiTextureType& type, const aiScene* scene, const std::string& rootPath)
{
	std::shared_ptr<Texture> texture = nullptr;
	//获取图片读取路径
	aiString aipath;
	aiMat->Get(AI_MATKEY_TEXTURE(type, 0), aipath);
	if(aipath.length==0)
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
glm::mat4 AssimpLoader::getMat4f(aiMatrix4x4 value)
{
	glm::mat4 res(
		value.a1, value.a2, value.a3, value.a4,
		value.b1, value.b2, value.b3, value.b4,
		value.c1, value.c2, value.c3, value.c4,
		value.d1, value.d2, value.d3, value.d4
	);
	return res;
}
