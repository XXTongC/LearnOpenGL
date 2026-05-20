#include "EnvironmentProfile.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "stb_image.h"
#include "tools/config/ProfileConfigParser.h"

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

	std::shared_ptr<Texture> createFloatEquirectangularTexture(
		const std::vector<float>& data,
		unsigned int width,
		unsigned int height,
		unsigned int unit
	)
	{
		unsigned int textureId{ 0 };
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB16F,
			static_cast<GLsizei>(width),
			static_cast<GLsizei>(height),
			0,
			GL_RGB,
			GL_FLOAT,
			data.data()
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto texture = std::make_shared<Texture>();
		texture->setTexture(textureId);
		texture->setWidth(static_cast<GLuint>(width));
		texture->setHeight(static_cast<GLuint>(height));
		texture->setUnit(unit);
		texture->setTextureTarget(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return texture;
	}
}

bool EnvironmentProfile::hasHdrSource() const
{
	return !hdrEquirectangularPath.empty();
}

bool EnvironmentProfile::hasEnvironmentSource() const
{
	return useProceduralEnvironment || hasHdrSource();
}

std::shared_ptr<Texture> EnvironmentTextureLoader::loadEquirectangular(const EnvironmentProfile& profile)
{
	if (profile.useProceduralEnvironment)
	{
		return createProceduralEquirectangular(profile);
	}

	return loadHdrEquirectangular(profile);
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

std::shared_ptr<Texture> EnvironmentTextureLoader::createProceduralEquirectangular(const EnvironmentProfile& profile)
{
	const unsigned int width = std::max(profile.proceduralWidth, 16u);
	const unsigned int height = std::max(profile.proceduralHeight, 8u);
	std::vector<float> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 3u, 0.0f);

	constexpr float pi = 3.14159265359f;
	const float sunDirX = 0.35f;
	const float sunDirY = 0.72f;
	const float sunDirZ = 0.45f;
	const float sunLength = std::sqrt(sunDirX * sunDirX + sunDirY * sunDirY + sunDirZ * sunDirZ);
	const float normalizedSunX = sunDirX / sunLength;
	const float normalizedSunY = sunDirY / sunLength;
	const float normalizedSunZ = sunDirZ / sunLength;

	for (unsigned int y = 0; y < height; ++y)
	{
		const float v = height > 1 ? static_cast<float>(y) / static_cast<float>(height - 1) : 0.0f;
		const float phi = (v - 0.5f) * pi;
		const float dirY = std::sin(phi);
		const float cosPhi = std::cos(phi);
		const float up = std::max(dirY, 0.0f);
		const float down = std::max(-dirY, 0.0f);

		for (unsigned int x = 0; x < width; ++x)
		{
			const float u = width > 1 ? static_cast<float>(x) / static_cast<float>(width - 1) : 0.0f;
			const float theta = (u - 0.5f) * 2.0f * pi;
			const float dirX = cosPhi * std::cos(theta);
			const float dirZ = cosPhi * std::sin(theta);

			const float skyBlend = std::pow(up, 0.55f);
			const float horizonBlend = 1.0f - std::min(std::abs(dirY) * 2.0f, 1.0f);
			const float sunDot = std::max(dirX * normalizedSunX + dirY * normalizedSunY + dirZ * normalizedSunZ, 0.0f);
			const float sunCore = std::pow(sunDot, 512.0f) * profile.proceduralSunIntensity;
			const float sunGlow = std::pow(sunDot, 16.0f) * profile.proceduralSunIntensity * 0.12f;

			float r = (0.08f + 0.34f * skyBlend + 0.38f * horizonBlend) * profile.proceduralSkyIntensity;
			float g = (0.14f + 0.46f * skyBlend + 0.26f * horizonBlend) * profile.proceduralSkyIntensity;
			float b = (0.22f + 0.78f * skyBlend + 0.08f * horizonBlend) * profile.proceduralSkyIntensity;

			r = r * (1.0f - down) + profile.proceduralGroundIntensity * 0.18f * down;
			g = g * (1.0f - down) + profile.proceduralGroundIntensity * 0.15f * down;
			b = b * (1.0f - down) + profile.proceduralGroundIntensity * 0.12f * down;

			const float sun = sunCore + sunGlow;
			r += sun * 1.0f;
			g += sun * 0.82f;
			b += sun * 0.52f;

			const size_t index = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 3u;
			pixels[index + 0] = r;
			pixels[index + 1] = g;
			pixels[index + 2] = b;
		}
	}

	return createFloatEquirectangularTexture(pixels, width, height, profile.hdrTextureUnit);
}

std::string EnvironmentProfileStorage::defaultPath()
{
	return "config/environment_profile.local.ini";
}

bool EnvironmentProfileStorage::loadFromFile(const std::string& path, EnvironmentProfile& profile)
{
	EnvironmentProfile loadedProfile = profile;
	const bool loaded = GL_CONFIG::readKeyValueFile(path, [&loadedProfile](const std::string& key, const std::string& value)
	{
		if (key == "hdrEquirectangularPath")
		{
			loadedProfile.hdrEquirectangularPath = value;
			return;
		}

		if (key == "hdrTextureUnit")
		{
			unsigned int parsedUnit{ loadedProfile.hdrTextureUnit };
			if (GL_CONFIG::parseUnsigned(value, parsedUnit))
			{
				loadedProfile.hdrTextureUnit = parsedUnit;
			}
			return;
		}

		if (key == "precomputeOnPrepare")
		{
			bool parsedPrecompute{ loadedProfile.precomputeOnPrepare };
			if (GL_CONFIG::parseBool(value, parsedPrecompute))
			{
				loadedProfile.precomputeOnPrepare = parsedPrecompute;
			}
			return;
		}

		if (key == "useProceduralEnvironment")
		{
			bool parsedUseProceduralEnvironment{ loadedProfile.useProceduralEnvironment };
			if (GL_CONFIG::parseBool(value, parsedUseProceduralEnvironment))
			{
				loadedProfile.useProceduralEnvironment = parsedUseProceduralEnvironment;
			}
			return;
		}

		if (key == "proceduralWidth")
		{
			unsigned int parsedWidth{ loadedProfile.proceduralWidth };
			if (GL_CONFIG::parseUnsigned(value, parsedWidth))
			{
				loadedProfile.proceduralWidth = parsedWidth;
			}
			return;
		}

		if (key == "proceduralHeight")
		{
			unsigned int parsedHeight{ loadedProfile.proceduralHeight };
			if (GL_CONFIG::parseUnsigned(value, parsedHeight))
			{
				loadedProfile.proceduralHeight = parsedHeight;
			}
			return;
		}

		if (key == "proceduralSkyIntensity")
		{
			float parsedSkyIntensity{ loadedProfile.proceduralSkyIntensity };
			if (GL_CONFIG::parseFloat(value, parsedSkyIntensity))
			{
				loadedProfile.proceduralSkyIntensity = parsedSkyIntensity;
			}
			return;
		}

		if (key == "proceduralGroundIntensity")
		{
			float parsedGroundIntensity{ loadedProfile.proceduralGroundIntensity };
			if (GL_CONFIG::parseFloat(value, parsedGroundIntensity))
			{
				loadedProfile.proceduralGroundIntensity = parsedGroundIntensity;
			}
			return;
		}

		if (key == "proceduralSunIntensity")
		{
			float parsedSunIntensity{ loadedProfile.proceduralSunIntensity };
			if (GL_CONFIG::parseFloat(value, parsedSunIntensity))
			{
				loadedProfile.proceduralSunIntensity = parsedSunIntensity;
			}
		}
	});
	if (!loaded)
	{
		return false;
	}

	profile = loadedProfile;
	return true;
}

bool EnvironmentProfileStorage::saveToFile(const std::string& path, const EnvironmentProfile& profile)
{
	const std::filesystem::path filePath{ path };
	const auto parentPath = filePath.parent_path();
	if (!parentPath.empty())
	{
		std::error_code error{};
		std::filesystem::create_directories(parentPath, error);
		if (error)
		{
			return false;
		}
	}

	std::ofstream output(path, std::ios::trunc);
	if (!output)
	{
		return false;
	}

	output << "# Local environment profile for PBR / IBL experiments\n";
	output << "hdrEquirectangularPath=" << profile.hdrEquirectangularPath << '\n';
	output << "hdrTextureUnit=" << profile.hdrTextureUnit << '\n';
	output << "precomputeOnPrepare=" << (profile.precomputeOnPrepare ? 1 : 0) << '\n';
	output << "useProceduralEnvironment=" << (profile.useProceduralEnvironment ? 1 : 0) << '\n';
	output << "proceduralWidth=" << profile.proceduralWidth << '\n';
	output << "proceduralHeight=" << profile.proceduralHeight << '\n';
	output << "proceduralSkyIntensity=" << profile.proceduralSkyIntensity << '\n';
	output << "proceduralGroundIntensity=" << profile.proceduralGroundIntensity << '\n';
	output << "proceduralSunIntensity=" << profile.proceduralSunIntensity << '\n';
	return true;
}
