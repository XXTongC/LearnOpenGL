#pragma once
#include "../core.h"
#include "../object.h"

namespace GLframework
{
	class Tools
	{
	public:
		//Pass in the matrix, deconstruct the position, rotation information XYZ, scaling information
		static void decompose(glm::mat4 matrix,glm::vec3& position,glm::vec3& eulerAngle,glm::vec3& scale);
		//Used to give a read model a uniform Blend setting
		static void setModelBlend(std::shared_ptr<Object> obj, bool blend, float opacity);
		//Pass in a camera's Projection*View matrix product to get the eight corners of the corresponding cone
		static std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projView);

		
		static glm::vec3 getCubemapFaceDirection(unsigned int face);
		static glm::vec3 getCubemapFaceUp(unsigned int face);
		
	};
}
