#include "RuntimeVerificationCapture.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "../tools/Logger/LogManager.h"
#include "GL_ERROR_FIND.h"
#include "core.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	void writePpmRows(
		std::ofstream& output,
		const std::vector<unsigned char>& pixels,
		unsigned int width,
		unsigned int height
	)
	{
		const auto rowStride = static_cast<std::streamsize>(width * 3);
		for (int row = static_cast<int>(height) - 1; row >= 0; --row)
		{
			const auto rowOffset = static_cast<std::streamoff>(row) * rowStride;
			output.write(
				reinterpret_cast<const char*>(pixels.data() + rowOffset),
				rowStride
			);
		}
	}
}

namespace GL_RUNTIME
{
	bool RuntimeVerificationCapture::captureDefaultFramebuffer(
		const std::string& path,
		unsigned int width,
		unsigned int height
	)
	{
		if (width == 0 || height == 0 || path.empty())
		{
			return false;
		}

		std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
		GL_CALL(glReadBuffer(GL_BACK));
		GL_CALL(glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data()));

		const std::filesystem::path capturePath{ path };
		const auto parentPath = capturePath.parent_path();
		if (!parentPath.empty())
		{
			std::error_code error{};
			std::filesystem::create_directories(parentPath, error);
			if (error)
			{
				reportLine("Runtime verification capture failed: cannot create " + parentPath.string());
				return false;
			}
		}

		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			reportLine("Runtime verification capture failed: cannot write " + path);
			return false;
		}

		output << "P6\n" << width << ' ' << height << "\n255\n";
		writePpmRows(output, pixels, width, height);
		reportLine("Runtime verification capture written: " + path);
		return true;
	}
}
