#include "tools.h"
#include "mesh.h"
using namespace GLframework;

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
