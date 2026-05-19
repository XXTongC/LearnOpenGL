#include "EnvironmentProfile.h"

#include <iostream>

#include "stb_image.h"

using namespace GLframework;

namespace
{
	unsigned int getFormatForChannelCount(int channels)
	{
		if (channels == 1)
		{
			return GL_RED;
		}

		if (channels == 4)
		{
			return GL_RGBA;
		}

		return GL_RGB;
	}

	unsigned int getInternalFormatForChannelCount(int channels)
	{
		if (channels == 1)
		{
			return GL_R16F;
		}

		if (channels == 4)
		{
			return GL_RGBA16F;
		}

		return GL_RGB16F;
	}
}

bool EnvironmentProfile::hasHdrSource() const
{
	return !hdrEquirectangularPath.empty();
}

std::shared_ptr<Texture> EnvironmentTextureLoader::loadHdrEquirectangular(const EnvironmentProfile& profile)
{
	if (!profile.hasHdrSource())
	{
		return nullptr;
	}

	int width{ 0 };
	int height{ 0 };
	int channels{ 0 };
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(profile.hdrEquirectangularPath.c_str(), &width, &height, &channels, 0);
	if (!data)
	{
		std::cerr << "Environment HDR Load Error: unable to load " << profile.hdrEquirectangularPath;
		if (stbi_failure_reason() != nullptr)
		{
			std::cerr << " (" << stbi_failure_reason() << ")";
		}
		std::cerr << std::endl;
		return nullptr;
	}

	unsigned int textureId{ 0 };
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		getInternalFormatForChannelCount(channels),
		width,
		height,
		0,
		getFormatForChannelCount(channels),
		GL_FLOAT,
		data
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);

	auto texture = std::make_shared<Texture>();
	texture->setTexture(textureId);
	texture->setWidth(static_cast<GLuint>(width));
	texture->setHeight(static_cast<GLuint>(height));
	texture->setUnit(profile.hdrTextureUnit);
	texture->setTextureTarget(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}
