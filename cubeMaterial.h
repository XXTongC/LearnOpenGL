#pragma once
#include "material.h"
#include "texture.h"

namespace GLframework
{
	class CubeMaterial:public Material
	{
	public:
		CubeMaterial()
		{

			this->setMaterialType(GLframework::MaterialType::CubeMaterial);
			this->setDepthWrite(false);
			//Ϊ�˷�ֹ��������״̬��1����zFighting���⣬��ⷽ����ΪС�ڵ���
			this->setDepthFunc(GL_LEQUAL);
		}
		~CubeMaterial(){}
	public:
		std::shared_ptr<GLframework::Texture> mDiffuse{ nullptr };
	};

}
