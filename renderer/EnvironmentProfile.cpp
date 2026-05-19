#include "EnvironmentProfile.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

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

	std::string trim(std::string value)
	{
		auto isSpace = [](unsigned char ch)
		{
			return std::isspace(ch) != 0;
		};

		value.erase(value.begin(), std::find_if(value.begin(), value.end(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}));
		value.erase(std::find_if(value.rbegin(), value.rend(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}).base(), value.end());
		return value;
	}

	bool parseUnsigned(const std::string& value, unsigned int& output)
	{
		try
		{
			size_t parsedCharacters{ 0 };
			const auto parsed = std::stoul(value, &parsedCharacters);
			if (parsedCharacters != value.size())
			{
				return false;
			}

			output = static_cast<unsigned int>(parsed);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool parseBool(std::string value, bool& output)
	{
		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
		{
			return static_cast<char>(std::tolower(ch));
		});

		if (value == "1" || value == "true" || value == "yes" || value == "on")
		{
			output = true;
			return true;
		}

		if (value == "0" || value == "false" || value == "no" || value == "off")
		{
			output = false;
			return true;
		}

		return false;
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

std::string EnvironmentProfileStorage::defaultPath()
{
	return "config/environment_profile.local.ini";
}

bool EnvironmentProfileStorage::loadFromFile(const std::string& path, EnvironmentProfile& profile)
{
	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	EnvironmentProfile loadedProfile = profile;
	std::string line{};
	while (std::getline(input, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
		{
			continue;
		}

		const auto separator = line.find('=');
		if (separator == std::string::npos)
		{
			continue;
		}

		const auto key = trim(line.substr(0, separator));
		const auto value = trim(line.substr(separator + 1));
		if (key == "hdrEquirectangularPath")
		{
			loadedProfile.hdrEquirectangularPath = value;
			continue;
		}

		if (key == "hdrTextureUnit")
		{
			unsigned int parsedUnit{ loadedProfile.hdrTextureUnit };
			if (parseUnsigned(value, parsedUnit))
			{
				loadedProfile.hdrTextureUnit = parsedUnit;
			}
			continue;
		}

		if (key == "precomputeOnPrepare")
		{
			bool parsedPrecompute{ loadedProfile.precomputeOnPrepare };
			if (parseBool(value, parsedPrecompute))
			{
				loadedProfile.precomputeOnPrepare = parsedPrecompute;
			}
		}
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
	return true;
}
