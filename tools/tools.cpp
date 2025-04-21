#include "tools.h"
#include "../mesh/mesh.h"
using namespace GLframework;

glm::vec3 Tools::getCubemapFaceDirection(unsigned int face)
{
	switch (face)
	{
	case 0: return glm::vec3(1.0f, 0.0f, 0.0f); // 右
	case 1: return glm::vec3(-1.0f, 0.0f, 0.0f); // 左
	case 2: return glm::vec3(0.0f, 1.0f, 0.0f); // 上
	case 3: return glm::vec3(0.0f, -1.0f, 0.0f); // 下
	case 4: return glm::vec3(0.0f, 0.0f, 1.0f); // 前
	case 5: return glm::vec3(0.0f, 0.0f, -1.0f); // 后
	default: return glm::vec3(0.0f);
	}
}

glm::vec3 Tools::getCubemapFaceUp(unsigned int face)
{
	switch (face)
	{
	case 0: return glm::vec3(0.0f, -1.0f, 0.0f); // 右
	case 1: return glm::vec3(0.0f, -1.0f, 0.0f); // 左
	case 2: return glm::vec3(0.0f, 0.0f, 1.0f); // 上
	case 3: return glm::vec3(0.0f, 0.0f, -1.0f); // 下
	case 4: return glm::vec3(0.0f, -1.0f, 0.0f); // 前
	case 5: return glm::vec3(0.0f, -1.0f, 0.0f); // 后
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

	//四元数 ---> 欧拉角
	glm::mat4 rotation = glm::toMat4(quaternion);
	glm::extractEulerAngleXYX(rotation, eulerAngle.x, eulerAngle.y, eulerAngle.z);

	//解构的角度是弧度制，我们需要将其转换为角度
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
