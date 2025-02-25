#pragma once
#include "mesh.h"
#include <string>
namespace GLframework
{
	class InstancedMesh : public Mesh
	{
	public:
		InstancedMesh(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material,unsigned int instanceCount,bool matricesUpdateState = true)
			:Mesh(geometry,material),mInstanceCount(instanceCount),mMatricesUpdateState(matricesUpdateState)
		{
			this->setType(GLframework::ObjectType::InstancedMesh);
			mInstanceMatrices.resize(instanceCount);

			glGenBuffers(1, &mMatrixVbo);
			glBindBuffer(GL_ARRAY_BUFFER, mMatrixVbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instanceCount, mInstanceMatrices.data(), GL_DYNAMIC_DRAW);

			glBindVertexArray(this->getGeometry()->getVao( ));
			glBindBuffer(GL_ARRAY_BUFFER, mMatrixVbo);
			const std::string matrixlocationName[ ]{"col0","col1","col2","col3"};
			for(int i = 0;i<4;i++ )
			{
				GLuint matrixLocation = glGetAttribLocation(this->getGeometry()->getShader()->getProgram( ), matrixlocationName[i].c_str());
				glEnableVertexAttribArray(matrixLocation);
				glVertexAttribPointer(matrixLocation,4, GL_FLOAT,GL_FALSE,sizeof(glm::mat4),(void*)(sizeof(float) * i * 4));
				glVertexAttribDivisor(matrixLocation, 1);	//逐一个实例更新
			}
			glBindVertexArray(0);
		}
		~InstancedMesh( )
		{
			
		}
		void updateMatrices( );
		void setMatrixVbo(unsigned int value );
		void setInstanceCount(unsigned int value);
		void setMatricesUpdateState(int value);
		int getMatricesUpdateState( ) const;
		unsigned int getMatrixVbo( ) const;
		unsigned int getInstanceCount( ) const;
		void sortMatrix(glm::mat4 viewMatrix);
		std::vector<glm::mat4> mInstanceMatrices;
	private:
		unsigned int	mInstanceCount{ 0 };
		unsigned int	mMatrixVbo{ 0 };
		//1 : uniformway, 0 : attribway
		int				mMatricesUpdateState = 1;
	};
}
