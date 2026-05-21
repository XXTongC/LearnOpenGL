#include "PBRDeferredClusteredLightGrid.h"

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

	float estimateLightRadius(const std::shared_ptr<PointLight>& light, float cutoff)
	{
		if (!light)
		{
			return 0.0f;
		}

		const glm::vec3 color = light->getColor();
		const float maxChannel = std::max({ color.r, color.g, color.b, 0.001f });
		const float intensity = std::max(light->getIntensity(), 0.001f) * maxChannel;
		const float safeCutoff = std::max(cutoff, 0.001f);
		const float targetAttenuationDenominator = intensity / safeCutoff;
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

	glm::vec2 projectToScreen(
		const glm::vec3& worldPosition,
		const glm::mat4& viewProjection,
		unsigned int width,
		unsigned int height,
		bool& valid
	)
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
		unsigned int height,
		float lightCutoff
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

		const float radius = estimateLightRadius(light, lightCutoff);
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
		radiusPixels = radiusPixels <= 0.0f ? 64.0f : std::max(radiusPixels, 2.0f);

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

	int depthToSlice(float depth, float nearPlane, float farPlane, int depthSlices)
	{
		const float safeNear = std::max(nearPlane, 0.001f);
		const float safeFar = std::max(farPlane, safeNear + 0.001f);
		const float normalized = (depth - safeNear) / (safeFar - safeNear);
		return std::clamp(static_cast<int>(std::floor(normalized * static_cast<float>(depthSlices))), 0, depthSlices - 1);
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

PBRDeferredClusteredLightGrid::~PBRDeferredClusteredLightGrid()
{
	if (mClusterBuffer != 0)
	{
		glDeleteBuffers(1, &mClusterBuffer);
		mClusterBuffer = 0;
	}
	if (mIndexBuffer != 0)
	{
		glDeleteBuffers(1, &mIndexBuffer);
		mIndexBuffer = 0;
	}
}

PBRDeferredClusteredLightGridStats PBRDeferredClusteredLightGrid::bind(
	const MaterialBindingContext& context,
	unsigned int targetWidth,
	unsigned int targetHeight,
	const PBRDeferredLightCullingConfig& config
)
{
	PBRDeferredClusteredLightGridStats stats{};
	stats.enabled = usesPbrDeferredGpuClusteredLightGrid(config);
	stats.layout = makePbrDeferredClusteredLightGridLayout(targetWidth, targetHeight, config);
	if (!stats.enabled || !context.camera || targetWidth == 0 || targetHeight == 0)
	{
		return stats;
	}

	ensureBuffers();
	if (mClusterBuffer == 0 || mIndexBuffer == 0)
	{
		return stats;
	}

	const auto lights = collectPackedPointLights(context);
	stats.pointLightCount = static_cast<int>(lights.size());
	const int clusterCount = stats.layout.clusterCount;
	mClusterLightCounts.assign(static_cast<std::size_t>(clusterCount), 0);
	mClusterLightEntries.clear();
	mClusterLightEntries.reserve(static_cast<std::size_t>(clusterCount) * lights.size());

	const glm::mat4 viewMatrix = context.camera->getViewMatrix();
	const glm::mat4 viewProjection = context.camera->getProjectionMatrix() * viewMatrix;
	const float nearPlane = std::max(context.camera->mNear, 0.001f);
	const float farPlane = std::max(context.camera->mFar, nearPlane + 0.001f);
	for (int lightIndex = 0; lightIndex < static_cast<int>(lights.size()); ++lightIndex)
	{
		const auto& light = lights[static_cast<std::size_t>(lightIndex)];
		const float radius = estimateLightRadius(light, config.lightCutoff);
		const float viewDepth = -(viewMatrix * glm::vec4(light->getPosition(), 1.0f)).z;
		const float minDepth = std::max(viewDepth - radius, nearPlane);
		const float maxDepth = std::min(viewDepth + radius, farPlane);
		if (maxDepth < nearPlane || minDepth > farPlane || maxDepth < minDepth)
		{
			continue;
		}

		const ScreenBounds bounds = calculateLightBounds(light, *context.camera, viewProjection, targetWidth, targetHeight, config.lightCutoff);
		if (!bounds.valid)
		{
			continue;
		}

		const int minTileX = std::clamp(bounds.minX / stats.layout.tileSize, 0, stats.layout.clusterColumns - 1);
		const int maxTileX = std::clamp(bounds.maxX / stats.layout.tileSize, 0, stats.layout.clusterColumns - 1);
		const int minTileY = std::clamp(bounds.minY / stats.layout.tileSize, 0, stats.layout.clusterRows - 1);
		const int maxTileY = std::clamp(bounds.maxY / stats.layout.tileSize, 0, stats.layout.clusterRows - 1);
		const int minSlice = depthToSlice(minDepth, nearPlane, farPlane, stats.layout.clusterDepthSlices);
		const int maxSlice = depthToSlice(maxDepth, nearPlane, farPlane, stats.layout.clusterDepthSlices);
		for (int slice = minSlice; slice <= maxSlice; ++slice)
		{
			for (int y = minTileY; y <= maxTileY; ++y)
			{
				for (int x = minTileX; x <= maxTileX; ++x)
				{
					if (!tileIntersectsCircularBounds(bounds, x, y, stats.layout.tileSize, targetWidth, targetHeight))
					{
						continue;
					}

					const int clusterIndex = (slice * stats.layout.clusterRows + y) * stats.layout.clusterColumns + x;
					auto& clusterLightCount = mClusterLightCounts[static_cast<std::size_t>(clusterIndex)];
					if (clusterLightCount >= stats.layout.maxLightsPerCluster)
					{
						continue;
					}

					++clusterLightCount;
					mClusterLightEntries.push_back(ClusterLightEntry{ clusterIndex, lightIndex });
				}
			}
		}
	}

	mClusterOffsetCount.assign(static_cast<std::size_t>(clusterCount), glm::ivec4{ 0 });
	int lightIndexCount = 0;
	for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
	{
		const int clusterLightCount = mClusterLightCounts[static_cast<std::size_t>(clusterIndex)];
		mClusterOffsetCount[static_cast<std::size_t>(clusterIndex)] = glm::ivec4(
			lightIndexCount,
			clusterLightCount,
			0,
			0
		);
		lightIndexCount += clusterLightCount;
	}

	mLightIndices.assign(static_cast<std::size_t>(lightIndexCount), 0);
	mClusterWriteOffsets.assign(static_cast<std::size_t>(clusterCount), 0);
	for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
	{
		mClusterWriteOffsets[static_cast<std::size_t>(clusterIndex)] = mClusterOffsetCount[static_cast<std::size_t>(clusterIndex)].x;
	}
	for (const auto& entry : mClusterLightEntries)
	{
		const auto clusterIndex = static_cast<std::size_t>(entry.clusterIndex);
		const int writeIndex = mClusterWriteOffsets[clusterIndex]++;
		mLightIndices[static_cast<std::size_t>(writeIndex)] = entry.lightIndex;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mClusterBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mClusterOffsetCount.size() * sizeof(glm::ivec4)), mClusterOffsetCount.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, clusterBufferBindingPoint(), mClusterBuffer);

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
	stats.lightIndexCount = static_cast<int>(mLightIndices.size());
	stats.culledLightIndexCount = std::max(clusterCount * stats.pointLightCount - stats.lightIndexCount, 0);
	return stats;
}

void PBRDeferredClusteredLightGrid::ensureBuffers()
{
	if (mClusterBuffer == 0)
	{
		glGenBuffers(1, &mClusterBuffer);
	}
	if (mIndexBuffer == 0)
	{
		glGenBuffers(1, &mIndexBuffer);
	}
}
