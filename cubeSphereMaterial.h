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
			//Ϊ�˷�ֹ��������״̬��1����zFighting���⣬��ⷽ����ΪС�ڵ���
			this->setDepthFunc(GL_LEQUAL);
		}
		~CubeSphereMaterial() {}
	public:
		std::shared_ptr<GLframework::Texture> mDiffuse{ nullptr };
	};

}
