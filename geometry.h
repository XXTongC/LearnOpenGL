#pragma once
#include "core.h"
#include "shader.h"
#include <vector>
// GLframework::Shader shader;

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
		std::shared_ptr<GLframework::Shader> mShader = nullptr;

	public:
		Geometry(std::shared_ptr<GLframework::Shader> shader) { mShader = shader; }
		Geometry(
			std::shared_ptr<GLframework::Shader> shader,
			const std::vector<float>& positions,
			const std::vector<float>& normals,
			const std::vector<float>& uvs,
			const std::vector<unsigned int>& indices
		);
		~Geometry();
		static std::shared_ptr<Geometry> createBox(std::shared_ptr<GLframework::Shader> shader, float length, float width, float height);
		//static Geometry* createBox(float length, float width, float height);
		static std::shared_ptr<Geometry> createSphere(std::shared_ptr< GLframework::Shader> shader, float radius, int mLatitude = 360, int mLong = 180);
		static std::shared_ptr<Geometry> createPlane(std::shared_ptr<GLframework::Shader> shader, float width, float height);
		static std::shared_ptr<Geometry> createScreenPlane(std::shared_ptr<GLframework::Shader> shader);
		void setShader(std::shared_ptr<GLframework::Shader> shader);
		GLuint getVao() const { return mVao; }
		GLuint getIndicesCount() const { return mIndicesCount; }


	};
}