#pragma once
#include "material.h"
#include "texture.h"

namespace GLframework
{
	class CubeSphereMaterial :public Material
	{
	public:
		CubeSphereMaterial()
		{

			this->setMaterialType(GLframework::MaterialType::CubeSphereMaterial);
			this->setDepthWrite(false);
			//为了防止与清除深度状态的1出现zFighting问题，检测方法改为小于等于
			this->setDepthFunc(GL_LEQUAL);
		}
		~CubeSphereMaterial() {}
	public:
		std::shared_ptr<GLframework::Texture> mDiffuse{ nullptr };
	};

}
