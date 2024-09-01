#pragma once
#include "core.h"
#include "shader.h"
#include <vector>
//extern GLframework::Shader shader;

namespace GLframework
{
	class Geometry
	{
	private:
		GLuint mVao{ 0 };
		GLuint mPosVbo{ 0 };
		GLuint mUvVbo{ 0 };
		GLuint mColorVbo{ 0 };
		GLuint mNormalVbo{ 0 };
		GLuint mEbo{ 0 };
		GLsizei mIndicesCount{ 0 };
		GLframework::Shader* mShader = nullptr;

	public:
		Geometry(GLframework::Shader* shader) { mShader = shader; }
		~Geometry();
		static Geometry* createBox(GLframework::Shader* shader, float length, float width, float height);
		//static Geometry* createBox(float length, float width, float height);
		static Geometry* createSphere(GLframework::Shader* shader, float radius, int mLatitude = 360, int mLong = 180);
		static Geometry* createPlane(GLframework::Shader* shader, float width, float height);
		void setShader(GLframework::Shader* shader);
		GLuint getVao() const { return mVao; }
		GLuint getIndicesCount() const { return mIndicesCount; }


	};
}