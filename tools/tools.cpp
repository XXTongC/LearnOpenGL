#include "tools.h"
#include "../mesh/mesh.h"
using namespace GLframework;

glm::vec3 Tools::getCubemapFaceDirection(unsigned int face)
{
	switch (face)
	{
	case 0: return glm::vec3(1.0f, 0.0f, 0.0f); // ��
	case 1: return glm::vec3(-1.0f, 0.0f, 0.0f); // ��
	case 2: return glm::vec3(0.0f, 1.0f, 0.0f); // ��
	case 3: return glm::vec3(0.0f, -1.0f, 0.0f); // ��
	case 4: return glm::vec3(0.0f, 0.0f, 1.0f); // ǰ
	case 5: return glm::vec3(0.0f, 0.0f, -1.0f); // ��
	default: return glm::vec3(0.0f);
	}
}

glm::vec3 Tools::getCubemapFaceUp(unsigned int face)
{
	switch (face)
	{
	case 0: return glm::vec3(0.0f, -1.0f, 0.0f); // ��
	case 1: return glm::vec3(0.0f, -1.0f, 0.0f); // ��
	case 2: return glm::vec3(0.0f, 0.0f, 1.0f); // ��
	case 3: return glm::vec3(0.0f, 0.0f, -1.0f); // ��
	case 4: return glm::vec3(0.0f, -1.0f, 0.0f); // ǰ
	case 5: return glm::vec3(0.0f, -1.0f, 0.0f); // ��
	default: return glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

std::vector<glm::vec4> Tools::getFrustumCornersWorldSpace(const glm::mat4& projView)
{
	const auto inv = glm::inverse(projView);

	std::vector<glm::vec4> corners;
	corners.reserve(8);
	for(int x = 0;x<2;++x)
	{
		for(int y = 0;y<2;++y)
		{
			for(int z = 0;z<2;++z)
			{
				glm::vec4 ndc = glm::vec4(2.0f * x - 1, 2.0f * y - 1, 2.0f * z - 1, 1.0f);
				glm::vec4 p_middle = inv * ndc;
				glm::vec4 p_world = p_middle / p_middle.w;
				corners.push_back(p_world);
			}
		}
	}
	return corners;
}


void Tools::decompose(
	glm::mat4	matrix, 
	glm::vec3&	position, 
	glm::vec3&	eulerAngle,
	glm::vec3&	scale)
{
	glm::quat quaternion;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, quaternion, position, skew, perspective);

	//��Ԫ�� ---> ŷ����
	glm::mat4 rotation = glm::toMat4(quaternion);
	glm::extractEulerAngleXYX(rotation, eulerAngle.x, eulerAngle.y, eulerAngle.z);

	//�⹹�ĽǶ��ǻ����ƣ�������Ҫ����ת��Ϊ�Ƕ�
	eulerAngle.x = glm::degrees(eulerAngle.x);
	eulerAngle.y = glm::degrees(eulerAngle.y);
	eulerAngle.z = glm::degrees(eulerAngle.z);

}

void Tools::setModelBlend(std::shared_ptr<Object> obj,bool blend, float opacity)
{
	if(obj->getType()==ObjectType::Mesh)
	{
		std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(obj);
		std::shared_ptr<Material> material = mesh->getMaterial();
		material->setColorBlendState(blend);
		material->setOpacity(opacity);
		material->setDepthWrite(false);
	}
	auto children = obj->getChildren();
	for(int i = 0;i<children.size();++i)
	{
		setModelBlend(children[i],blend,opacity);
	}

}
