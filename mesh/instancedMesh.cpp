#include "instancedMesh.h"

using namespace GLframework;

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
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * mInstanceCount, mInstanceMatrices);
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



