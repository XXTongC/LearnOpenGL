#include "EnvironmentProfile.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "framework/texture.h"
#include "stb_image.h"
#include "tools/config/ProfileConfigIO.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

namespace
{
	int toEditableInt(unsigned int value)
	{
		return static_cast<int>(value);
	}

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

void EnvironmentProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("Environment Source");
	builder.addConfigString("hdrEquirectangularPath", "HDR Path", &hdrEquirectangularPath);
	builder.addConfigInt(
		"hdrTextureUnit",
		"HDR Texture Unit",
		[this]() { return toEditableInt(hdrTextureUnit); },
		[this](int value) { hdrTextureUnit = value < 0 ? 0u : static_cast<unsigned int>(value); },
		0,
		31
	);
	builder.addConfigBool("useProceduralEnvironment", "Use Procedural Environment", &useProceduralEnvironment);
	builder.addConfigBool("precomputeOnPrepare", "Precompute On Prepare", &precomputeOnPrepare);

	builder.addSection("Procedural Environment");
	builder.addText("Mode", "Generated at precompute time when Use Procedural Environment is enabled.");
	builder.addConfigInt(
		"proceduralWidth",
		"Procedural Width",
		[this]() { return toEditableInt(proceduralWidth); },
		[this](int value) { proceduralWidth = value < 0 ? 0u : static_cast<unsigned int>(value); },
		64,
		2048
	);
	builder.addConfigInt(
		"proceduralHeight",
		"Procedural Height",
		[this]() { return toEditableInt(proceduralHeight); },
		[this](int value) { proceduralHeight = value < 0 ? 0u : static_cast<unsigned int>(value); },
		32,
		1024
	);
	builder.addConfigFloat("proceduralSkyIntensity", "Procedural Sky Intensity", &proceduralSkyIntensity, 0.0f, 10.0f);
	builder.addConfigFloat("proceduralGroundIntensity", "Procedural Ground Intensity", &proceduralGroundIntensity, 0.0f, 2.0f);
	builder.addConfigFloat("proceduralSunIntensity", "Procedural Sun Intensity", &proceduralSunIntensity, 0.0f, 20.0f);
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
	GL_EDITOR::PropertyBuilder builder{};
	loadedProfile.visitEditableProperties(builder);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	profile = loadedProfile;
	return true;
}

bool EnvironmentProfileStorage::saveToFile(const std::string& path, const EnvironmentProfile& profile)
{
	EnvironmentProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	snapshot.visitEditableProperties(builder);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local environment profile for PBR / IBL experiments",
		builder
	);
}
