#include "PBRDeferredLightBuffer.h"

#include <algorithm>
#include <array>

#include "core.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "renderer/MaterialBindingContext.h"

using namespace GLframework;

namespace
{
	struct PBRDeferredLightBufferData
	{
		glm::vec4 directionalColorIntensity{ 0.0f };
		glm::vec4 directionalDirectionEnabled{ 0.0f, -1.0f, 0.0f, 0.0f };
		glm::vec4 ambientColor{ 0.0f };
		glm::ivec4 pointLightMeta{ 0 };
		std::array<glm::vec4, PBRDeferredLightBuffer::maxPointLights()> pointLightColorIntensity{};
		std::array<glm::vec4, PBRDeferredLightBuffer::maxPointLights()> pointLightPositionEnabled{};
		std::array<glm::vec4, PBRDeferredLightBuffer::maxPointLights()> pointLightAttenuation{};
	};

	static_assert(sizeof(glm::vec4) == 16, "PBR deferred light SSBO expects vec4 to occupy 16 bytes.");
	static_assert(sizeof(glm::ivec4) == 16, "PBR deferred light SSBO expects ivec4 to occupy 16 bytes.");

	PBRDeferredLightBufferData buildLightBufferData(const MaterialBindingContext& context, int& packedPointLightCount)
	{
		PBRDeferredLightBufferData data{};

		if (context.dirLight)
		{
			data.directionalColorIntensity = glm::vec4(
				context.dirLight->getColor(),
				context.dirLight->getIntensity()
			);
			data.directionalDirectionEnabled = glm::vec4(context.dirLight->getDirection(), 1.0f);
		}

		if (context.ambient)
		{
			data.ambientColor = glm::vec4(context.ambient->getColor(), 1.0f);
		}

		packedPointLightCount = 0;
		for (const auto& pointLight : context.getPointLights())
		{
			if (!pointLight || packedPointLightCount >= PBRDeferredLightBuffer::maxPointLights())
			{
				continue;
			}

			const auto index = static_cast<std::size_t>(packedPointLightCount);
			data.pointLightColorIntensity[index] = glm::vec4(pointLight->getColor(), pointLight->getIntensity());
			data.pointLightPositionEnabled[index] = glm::vec4(pointLight->getPosition(), 1.0f);
			data.pointLightAttenuation[index] = glm::vec4(
				pointLight->getK2(),
				pointLight->getK1(),
				pointLight->getK0(),
				0.0f
			);
			++packedPointLightCount;
		}

		data.pointLightMeta = glm::ivec4(packedPointLightCount, 0, 0, 0);
		return data;
	}
}

PBRDeferredLightBuffer::~PBRDeferredLightBuffer()
{
	if (mBuffer != 0)
	{
		glDeleteBuffers(1, &mBuffer);
		mBuffer = 0;
	}
}

PBRDeferredLightBufferStats PBRDeferredLightBuffer::bind(const MaterialBindingContext& context)
{
	ensureBuffer();
	if (mBuffer == 0)
	{
		return {};
	}

	int packedPointLightCount = 0;
	const PBRDeferredLightBufferData data = buildLightBufferData(context, packedPointLightCount);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PBRDeferredLightBufferData), &data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint(), mBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return PBRDeferredLightBufferStats{
		true,
		packedPointLightCount,
		maxPointLights(),
		bindingPoint()
	};
}

void PBRDeferredLightBuffer::ensureBuffer()
{
	if (mBuffer != 0)
	{
		return;
	}

	glGenBuffers(1, &mBuffer);
}
