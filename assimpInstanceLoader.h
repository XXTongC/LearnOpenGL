#pragma once
#include "core.h"
#include "object.h"
#include "third_party/assimp/Importer.hpp"
#include "third_party/assimp/scene.h"
#include "third_party/assimp/postprocess.h"
#include "mesh/instancedMesh.h"
#include "renderer.h"
#include "texture.h"
#include "shader.h"
namespace GL_APPLICATION
{
	
	class AssimpInstanceLoader
	{
	public:
		static std::shared_ptr<GLframework::Object> load(
			const std::string& path, 
			std::shared_ptr<GLframework::Renderer> renderer,
			const unsigned int& instanceCount
		);
		static void setInstanceMatrix(std::shared_ptr<GLframework::Object> obj, unsigned int index, glm::mat4 matrix);
		static void updateInstanceMatrix(std::shared_ptr<GLframework::Object> obj);

	private:
		static void processNode(
			std::shared_ptr<GLframework::Renderer> renderer, 
			aiNode* ainode, 
			std::shared_ptr<GLframework::Object> parent,
			const aiScene* scene, 
			const std::string& rootPath, 
			const unsigned int& instanceCount
		);

		static glm::mat4 getMat4f(aiMatrix4x4 value);

		static std::shared_ptr<GLframework::InstancedMesh> processInstanceMesh(
			std::shared_ptr<GLframework::Renderer> renderer, 
			aiMesh* aimesh, const aiScene* scene, 
			const std::string& rootPath, 
			const unsigned int& instanceCount
		);

		static std::shared_ptr<GLframework::Texture> processTexture(
			const aiMaterial* aiMat,
			const aiTextureType& type,
			const aiScene* scene,
			const std::string& rootPath,
			const unsigned int& instanceCount
		);

	};
}
