#pragma once
#include "core.h"
#include "object.h"
#include "third_party/assimp/Importer.hpp"
#include "third_party/assimp/scene.h"
#include "third_party/assimp/postprocess.h"
#include "mesh/mesh.h"
#include "renderer.h"
#include "texture.h"
#include "shader.h"
namespace GL_APPLICATION
{
	using namespace GLframework;
	class AssimpLoader
	{
	public:
		static std::shared_ptr<Object> load(const std::string& path, std::shared_ptr<Renderer> renderer);

	private:
		static void processNode(std::shared_ptr<Renderer> renderer,aiNode* ainode,std::shared_ptr<Object> parent,const aiScene* scene,const std::string& rootPath);
		static glm::mat4 getMat4f(aiMatrix4x4 value);
		static std::shared_ptr<Mesh> processMesh(std::shared_ptr<Renderer> renderer,aiMesh* aimesh,const aiScene* scene, const std::string& rootPath);
		static std::shared_ptr<Texture> processTexture(
			const aiMaterial* aiMat,
			const aiTextureType & type,
			const aiScene* scene,
			const std::string& rootPath
		);
		
	};
}
