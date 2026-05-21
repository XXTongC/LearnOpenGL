#include "PBRDeferredTiledLightGrid.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "camera/camera.h"
#include "light/pointLight.h"
#include "renderer/PBRDeferredLightBuffer.h"

using namespace GLframework;

namespace
{
	struct ScreenBounds
	{
		int minX{ 0 };
		int minY{ 0 };
		int maxX{ 0 };
		int maxY{ 0 };
		float centerX{ 0.0f };
		float centerY{ 0.0f };
		float radiusPixels{ 0.0f };
		bool valid{ false };
		bool circularClip{ false };
	};

	float estimateLightRadius(const std::shared_ptr<PointLight>& light)
	{
		if (!light)
		{
			return 0.0f;
		}

		const glm::vec3 color = light->getColor();
		const float maxChannel = std::max({ color.r, color.g, color.b, 0.001f });
		const float intensity = std::max(light->getIntensity(), 0.001f) * maxChannel;
		constexpr float cutoff = 0.01f;
		const float targetAttenuationDenominator = intensity / cutoff;
		const float k2 = light->getK2();
		const float k1 = light->getK1();
		const float k0 = light->getK0();

		if (std::abs(k2) > 0.0001f)
		{
			const float c = k0 - targetAttenuationDenominator;
			const float discriminant = k1 * k1 - 4.0f * k2 * c;
			if (discriminant > 0.0f)
			{
				return std::max((-k1 + std::sqrt(discriminant)) / (2.0f * k2), 0.5f);
			}
		}

		if (std::abs(k1) > 0.0001f)
		{
			return std::max((targetAttenuationDenominator - k0) / k1, 0.5f);
		}

		return 64.0f;
	}

	glm::vec2 projectToScreen(const glm::vec3& worldPosition, const glm::mat4& viewProjection, unsigned int width, unsigned int height, bool& valid)
	{
		const glm::vec4 clip = viewProjection * glm::vec4(worldPosition, 1.0f);
		valid = clip.w > 0.0001f;
		if (!valid)
		{
			return {};
		}

		const glm::vec3 ndc = glm::vec3(clip) / clip.w;
		return {
			(ndc.x * 0.5f + 0.5f) * static_cast<float>(width),
			(ndc.y * 0.5f + 0.5f) * static_cast<float>(height)
		};
	}

	ScreenBounds calculateLightBounds(
		const std::shared_ptr<PointLight>& light,
		const Camera& camera,
		const glm::mat4& viewProjection,
		unsigned int width,
		unsigned int height
	)
	{
		ScreenBounds bounds{};
		if (!light || width == 0 || height == 0)
		{
			return bounds;
		}

		const glm::vec3 position = light->getPosition();
		bool centerValid = false;
		const glm::vec2 center = projectToScreen(position, viewProjection, width, height, centerValid);
		if (!centerValid)
		{
			bounds.minX = 0;
			bounds.minY = 0;
			bounds.maxX = static_cast<int>(width) - 1;
			bounds.maxY = static_cast<int>(height) - 1;
			bounds.valid = true;
			return bounds;
		}

		const float radius = estimateLightRadius(light);
		const glm::vec3 sampleOffsets[] = {
			camera.mRight * radius,
			-camera.mRight * radius,
			camera.mUp * radius,
			-camera.mUp * radius
		};
		float radiusPixels = 0.0f;
		for (const auto& offset : sampleOffsets)
		{
			bool sampleValid = false;
			const glm::vec2 sample = projectToScreen(position + offset, viewProjection, width, height, sampleValid);
			if (!sampleValid)
			{
				continue;
			}

			const glm::vec2 delta = sample - center;
			radiusPixels = std::max(radiusPixels, std::max(std::abs(delta.x), std::abs(delta.y)));
		}
		if (radiusPixels <= 0.0f)
		{
			radiusPixels = 64.0f;
		}
		else
		{
			radiusPixels = std::max(radiusPixels, 2.0f);
		}

		const float minX = center.x - radiusPixels;
		const float maxX = center.x + radiusPixels;
		const float minY = center.y - radiusPixels;
		const float maxY = center.y + radiusPixels;
		if (maxX < 0.0f || maxY < 0.0f || minX >= static_cast<float>(width) || minY >= static_cast<float>(height))
		{
			return bounds;
		}

		bounds.minX = std::clamp(static_cast<int>(std::floor(minX)), 0, static_cast<int>(width) - 1);
		bounds.minY = std::clamp(static_cast<int>(std::floor(minY)), 0, static_cast<int>(height) - 1);
		bounds.maxX = std::clamp(static_cast<int>(std::ceil(maxX)), 0, static_cast<int>(width) - 1);
		bounds.maxY = std::clamp(static_cast<int>(std::ceil(maxY)), 0, static_cast<int>(height) - 1);
		bounds.centerX = center.x;
		bounds.centerY = center.y;
		bounds.radiusPixels = radiusPixels;
		bounds.valid = bounds.maxX >= bounds.minX && bounds.maxY >= bounds.minY;
		bounds.circularClip = bounds.valid;
		return bounds;
	}

	bool tileIntersectsCircularBounds(
		const ScreenBounds& bounds,
		int tileX,
		int tileY,
		int tileSize,
		unsigned int width,
		unsigned int height
	)
	{
		if (!bounds.circularClip)
		{
			return true;
		}

		const float minX = static_cast<float>(tileX * tileSize);
		const float minY = static_cast<float>(tileY * tileSize);
		const float maxX = std::min(static_cast<float>((tileX + 1) * tileSize), static_cast<float>(width));
		const float maxY = std::min(static_cast<float>((tileY + 1) * tileSize), static_cast<float>(height));
		const float closestX = std::clamp(bounds.centerX, minX, maxX);
		const float closestY = std::clamp(bounds.centerY, minY, maxY);
		const float deltaX = closestX - bounds.centerX;
		const float deltaY = closestY - bounds.centerY;
		const float conservativeRadius = bounds.radiusPixels + 1.0f;
		return deltaX * deltaX + deltaY * deltaY <= conservativeRadius * conservativeRadius;
	}

	std::vector<std::shared_ptr<PointLight>> collectPackedPointLights(const MaterialBindingContext& context)
	{
		std::vector<std::shared_ptr<PointLight>> lights{};
		for (const auto& pointLight : context.getPointLights())
		{
			if (!pointLight || lights.size() >= static_cast<std::size_t>(PBRDeferredLightBuffer::maxPointLights()))
			{
				continue;
			}

			lights.push_back(pointLight);
		}
		return lights;
	}
}

PBRDeferredTiledLightGrid::~PBRDeferredTiledLightGrid()
{
	if (mTileBuffer != 0)
	{
		glDeleteBuffers(1, &mTileBuffer);
		mTileBuffer = 0;
	}
	if (mIndexBuffer != 0)
	{
		glDeleteBuffers(1, &mIndexBuffer);
		mIndexBuffer = 0;
	}
}

PBRDeferredTiledLightGridStats PBRDeferredTiledLightGrid::bind(
	const MaterialBindingContext& context,
	unsigned int targetWidth,
	unsigned int targetHeight,
	int tileSize
)
{
	PBRDeferredTiledLightGridStats stats{};
	stats.enabled = true;
	stats.tileSize = std::max(tileSize, 1);
	if (!context.camera || targetWidth == 0 || targetHeight == 0)
	{
		return stats;
	}

	ensureBuffers();
	if (mTileBuffer == 0 || mIndexBuffer == 0)
	{
		return stats;
	}

	const int columns = static_cast<int>((targetWidth + static_cast<unsigned int>(stats.tileSize) - 1) / static_cast<unsigned int>(stats.tileSize));
	const int rows = static_cast<int>((targetHeight + static_cast<unsigned int>(stats.tileSize) - 1) / static_cast<unsigned int>(stats.tileSize));
	const int tileCount = std::max(columns * rows, 0);
	mTileLightCounts.assign(static_cast<std::size_t>(tileCount), 0);
	mTileLightEntries.clear();

	const glm::mat4 viewProjection = context.camera->getProjectionMatrix() * context.camera->getViewMatrix();
	const auto lights = collectPackedPointLights(context);
	stats.pointLightCount = static_cast<int>(lights.size());
	stats.fullLightIndexCount = tileCount * stats.pointLightCount;
	mTileLightEntries.reserve(static_cast<std::size_t>(tileCount) * lights.size());
	for (int lightIndex = 0; lightIndex < static_cast<int>(lights.size()); ++lightIndex)
	{
		const ScreenBounds bounds = calculateLightBounds(lights[lightIndex], *context.camera, viewProjection, targetWidth, targetHeight);
		if (!bounds.valid)
		{
			continue;
		}

		const int minTileX = std::clamp(bounds.minX / stats.tileSize, 0, columns - 1);
		const int maxTileX = std::clamp(bounds.maxX / stats.tileSize, 0, columns - 1);
		const int minTileY = std::clamp(bounds.minY / stats.tileSize, 0, rows - 1);
		const int maxTileY = std::clamp(bounds.maxY / stats.tileSize, 0, rows - 1);
		for (int y = minTileY; y <= maxTileY; ++y)
		{
			for (int x = minTileX; x <= maxTileX; ++x)
			{
				if (!tileIntersectsCircularBounds(bounds, x, y, stats.tileSize, targetWidth, targetHeight))
				{
					continue;
				}

				const int tileIndex = y * columns + x;
				++mTileLightCounts[static_cast<std::size_t>(tileIndex)];
				mTileLightEntries.push_back(TileLightEntry{ tileIndex, lightIndex });
			}
		}
	}

	mTileOffsetCount.assign(static_cast<std::size_t>(tileCount), glm::ivec4{ 0 });
	int lightIndexCount = 0;
	for (int tileIndex = 0; tileIndex < tileCount; ++tileIndex)
	{
		const int tileLightCount = mTileLightCounts[static_cast<std::size_t>(tileIndex)];
		mTileOffsetCount[static_cast<std::size_t>(tileIndex)] = glm::ivec4(
			lightIndexCount,
			tileLightCount,
			0,
			0
		);
		lightIndexCount += tileLightCount;
		stats.maxTileLightCount = std::max(stats.maxTileLightCount, tileLightCount);
		if (tileLightCount == 0)
		{
			++stats.emptyTileCount;
		}
		else
		{
			++stats.occupiedTileCount;
		}
	}

	mLightIndices.assign(static_cast<std::size_t>(lightIndexCount), 0);
	mTileWriteOffsets.assign(static_cast<std::size_t>(tileCount), 0);
	for (int tileIndex = 0; tileIndex < tileCount; ++tileIndex)
	{
		mTileWriteOffsets[static_cast<std::size_t>(tileIndex)] = mTileOffsetCount[static_cast<std::size_t>(tileIndex)].x;
	}
	for (const auto& entry : mTileLightEntries)
	{
		const auto tileIndex = static_cast<std::size_t>(entry.tileIndex);
		const int writeIndex = mTileWriteOffsets[tileIndex]++;
		mLightIndices[static_cast<std::size_t>(writeIndex)] = entry.lightIndex;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTileBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mTileOffsetCount.size() * sizeof(glm::ivec4)), mTileOffsetCount.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, tileBufferBindingPoint(), mTileBuffer);

	const int fallbackIndex = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mIndexBuffer);
	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		static_cast<GLsizeiptr>(std::max<std::size_t>(mLightIndices.size(), 1) * sizeof(int)),
		mLightIndices.empty() ? &fallbackIndex : mLightIndices.data(),
		GL_DYNAMIC_DRAW
	);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, indexBufferBindingPoint(), mIndexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	stats.bound = true;
	stats.tileColumns = columns;
	stats.tileRows = rows;
	stats.tileCount = tileCount;
	stats.lightIndexCount = static_cast<int>(mLightIndices.size());
	stats.culledLightIndexCount = std::max(stats.fullLightIndexCount - stats.lightIndexCount, 0);
	stats.tileBufferBindingPoint = tileBufferBindingPoint();
	stats.indexBufferBindingPoint = indexBufferBindingPoint();
	return stats;
}

void PBRDeferredTiledLightGrid::ensureBuffers()
{
	if (mTileBuffer == 0)
	{
		glGenBuffers(1, &mTileBuffer);
	}
	if (mIndexBuffer == 0)
	{
		glGenBuffers(1, &mIndexBuffer);
	}
}
