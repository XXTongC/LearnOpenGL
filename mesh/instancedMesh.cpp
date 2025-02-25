#include "instancedMesh.h"
#include <algorithm>
using namespace GLframework;

void InstancedMesh::sortMatrix(glm::mat4 viewMatrix)
{
	std::sort(mInstanceMatrices.begin( ), mInstanceMatrices.end( ), [ viewMatrix ] (const glm::mat4& A, const glm::mat4& B)
	{
	//	1. 计算a的相机系的Z

	auto modelMatrixA = A;
	auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	auto cameraPositionA = viewMatrix * worldPositionA;

	//2 计算b的相机系的Z
	auto modelMatrixB = B;
	auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	auto cameraPositionB = viewMatrix * worldPositionB;

	return cameraPositionA.z < cameraPositionB.z;
	});
}


int InstancedMesh::getMatricesUpdateState() const
{
	return mMatricesUpdateState;
}

void InstancedMesh::setMatricesUpdateState(int value)
{
	mMatricesUpdateState = value;
}

void InstancedMesh::updateMatrices()
{
	mMatricesUpdateState = 0;
	glBindBuffer(GL_ARRAY_BUFFER, mMatrixVbo);
	//如果使用glBufferData更新数据会重新开辟并分配空间，性能消耗大
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * mInstanceCount, mInstanceMatrices, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * mInstanceCount, mInstanceMatrices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


unsigned int InstancedMesh::getMatrixVbo() const
{
	return mMatrixVbo;
}

void InstancedMesh::setMatrixVbo(unsigned int value)
{
	mMatrixVbo = value;
}



unsigned int InstancedMesh::getInstanceCount() const
{
	return mInstanceCount;
}

void InstancedMesh::setInstanceCount(unsigned int value)
{
	mInstanceCount = value;
}



