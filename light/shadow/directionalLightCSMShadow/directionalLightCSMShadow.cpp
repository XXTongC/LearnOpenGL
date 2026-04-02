#include "directionalLightCSMShadow.h"

#include <deque>

#include "../../../camera/orthographiccamera.h"
#include "../../../camera/perspectivecamera.h"
#include "../../../tools/tools.h"

using namespace GLframework;

std::vector<glm::mat4> DirectionalLightCSMShadow::getLightMatrix(Camera* camera, glm::vec3 lightDir, const std::vector<float>& clips)
{
    std::vector<glm::mat4> matrices;
    matrices.reserve(clips.size() - 1);
    for(int i = 0;i<clips.size()-1;++i)
    {
        auto lightMatrix = getLightMatrix(camera, lightDir, clips[i], clips[i + 1]);
        matrices.push_back(lightMatrix);
    }

    return matrices;

}


glm::mat4 DirectionalLightCSMShadow::getLightMatrix(Camera* camera, glm::vec3 lightDir, float near, float far)
{
    //  1. Find the value of the world coordinate system of the eight corners of the current subviewcone
    auto perpCamera = (PerspectiveCamera*)camera;
    auto perpViewMatrix = perpCamera->getViewMatrix();
    auto perpProjectionMatrix = glm::perspective(glm::radians(perpCamera->getFovy()), perpCamera->getAspect(), near, far);
    auto corners = Tools::getFrustumCornersWorldSpace(perpProjectionMatrix * perpViewMatrix);

    //  2. The average of the positions of the eight corner points is used as the position of the light source to obtain the lightViewMatrix
    glm::vec3 center = glm::vec3(0.0f);
    for (int i = 0; i < corners.size(); ++i)
    {
        center += glm::vec3(corners[i]);
    }
    center /= corners.size();

    auto lightViewMatrix = glm::lookAt(center, center + lightDir, glm::vec3(0.0, 1.0, 0.0));

    //  3. The eight corner points are converted to the light source coordinate system, and the smallest AABB bounding box is found
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& t : corners)
    {
        //Change point v from the world coordinate system to the light source camera coordinate system
        const auto pt = lightViewMatrix * t;
        minX = std::min(pt.x, minX);
        maxX = std::max(pt.x, maxX);
        minY = std::min(pt.y, minY);
        maxY = std::max(pt.y, maxY);
        minZ = std::min(pt.z, minZ);
        maxZ = std::max(pt.z, maxZ);
    }
    /*
    //  4. Adjust (Make sure that the occlusion relation outside the bounding box is correct)
    const float zMult = 10.0f; // Z-direction multiplier, can be adjusted as needed
    const float xyMult = 1.2f; // XY-direction multiplier
    const float stabilityFactor = 1.0f; // Stability factor

    // Calculate the center and size of the bounding box
    glm::vec3 boxCenter = glm::vec3((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
    glm::vec3 boxSize = glm::vec3(maxX - minX, maxY - minY, maxZ - minZ);

    // Adjust the bounding box based on light direction
    glm::vec3 lightDirAbs = glm::abs(lightDir);
    boxSize.x *= (1.0f + lightDirAbs.x * xyMult);
    boxSize.y *= (1.0f + lightDirAbs.y * xyMult);
    boxSize.z *= zMult;

    // Apply stability factor
    static glm::vec3 lastBoxSize = boxSize;
    boxSize = glm::mix(lastBoxSize, boxSize, stabilityFactor);
    lastBoxSize = boxSize;

    // Update the bounding box
    minX = boxCenter.x - boxSize.x * 0.5f;
    maxX = boxCenter.x + boxSize.x * 0.5f;
    minY = boxCenter.y - boxSize.y * 0.5f;
    maxY = boxCenter.y + boxSize.y * 0.5f;
    minZ = boxCenter.z - boxSize.z * 0.5f;
    maxZ = boxCenter.z + boxSize.z * 0.5f;

    // �����Z�������ȷ����׽���������ӰͶ����
    float zRange = maxZ - minZ;
    minZ -= zRange * 0.1f;
    maxZ += zRange * 0.1f;

    // 5. Create orthographic projection matrix
    // Note: We swap the near and far planes because the camera is looking towards -z
    glm::mat4 lightProjectionMatrix = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

    // 6. Return the final light space transformation matrix
    return lightProjectionMatrix * lightViewMatrix;
    */

    //4 �Ż���Ӱ�߽�����
    const float zMult = 10.0f; // Z����Ļ�������
    const float xyMult = 1.2f; // XY����Ļ�������
    const float stabilityFactor = 0.99f; // �ȶ�������
    const int historyLength = 10; // ��ʷ��¼����
    const float lightAngleInfluence = 0.2f; // ��Դ�Ƕ�Ӱ������

    // ���㵱ǰ�߽������ĺͳߴ�
    glm::vec3 boxCenter = glm::vec3((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
    glm::vec3 boxSize = glm::vec3(maxX - minX, maxY - minY, maxZ - minZ);

    // ���ݹ�Դ����̬��������
    glm::vec3 lightDirAbs = glm::abs(lightDir);
    float dynamicXYMult = xyMult * (1.0f + glm::dot(lightDirAbs, glm::vec3(1.0f, 1.0f, 0.0f)) * lightAngleInfluence);
    float dynamicZMult = zMult * (1.0f + lightDirAbs.z * lightAngleInfluence);

    // Ӧ�ö�̬����
    boxSize.x *= dynamicXYMult;
    boxSize.y *= dynamicXYMult;
    boxSize.z *= dynamicZMult;

    // ʹ����ʷ��¼��ƽ���߽���С�ı仯
    static std::deque<glm::vec3> sizeHistory;
    sizeHistory.push_back(boxSize);
    if (sizeHistory.size() > historyLength) {
        sizeHistory.pop_front();
    }

    glm::vec3 averageSize(0.0f);
    for (const auto& size : sizeHistory) {
        averageSize += size;
    }
    averageSize /= sizeHistory.size();

    // Ӧ���ȶ�������
    boxSize = glm::mix(averageSize, boxSize, stabilityFactor);

    // ���±߽��
    minX = boxCenter.x - boxSize.x * 0.5f;
    maxX = boxCenter.x + boxSize.x * 0.5f;
    minY = boxCenter.y - boxSize.y * 0.5f;
    maxY = boxCenter.y + boxSize.y * 0.5f;
    minZ = boxCenter.z - boxSize.z * 0.5f;
    maxZ = boxCenter.z + boxSize.z * 0.5f;

    // ����ӦZ��Χ����
    float sceneScale = glm::length(boxSize); // ʹ�ñ߽���С��Ϊ�����߶ȵĹ���
    float zRangeExtension = sceneScale * 0.1f; // ���ݳ����߶ȶ�̬����Z��Χ��չ
    minZ -= zRangeExtension;
    maxZ += zRangeExtension;

    // Ӧ��"����������"�����Լ�����Ӱ����
    const float texelSize = (maxX - minX) / 1024.0f; // ������Ӱ��ͼ�ֱ���Ϊ1024x1024
    minX = std::floor(minX / texelSize) * texelSize;
    maxX = std::ceil(maxX / texelSize) * texelSize;
    minY = std::floor(minY / texelSize) * texelSize;
    maxY = std::ceil(maxY / texelSize) * texelSize;
    minZ = std::floor(minZ / texelSize) * texelSize;
    maxZ = std::ceil(maxZ / texelSize) * texelSize;

    auto lightProjectionMatrix = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

    return lightProjectionMatrix * lightViewMatrix;
    /*
	//4 ����(��Χ����������壬Ҳ�ܹ�Ӱ�쵽���ڲ��������Ӱ�ڵ�Ч����
    maxZ *= 10;
    minZ *= 10;

    //5 ���㵱ǰ��Դ��ͶӰ����
    auto lightProjectionMatrix = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

    return lightProjectionMatrix * lightViewMatrix;
    */
}

void DirectionalLightCSMShadow::setLayerCount(int value) {
	mLayerCount = value;
}

int DirectionalLightCSMShadow::getLayerCount() const
{
	return mLayerCount;
}

void DirectionalLightCSMShadow::generateCascadeLayers(std::vector<float>& layers, float near, float far)
{
	layers.clear();
	layers.resize(mLayerCount + 1);
	for (int i = 0; i <= mLayerCount; ++i)
	{
		float layer = near * glm::pow((far / near),static_cast<float>(i)/static_cast<float>(this->getLayerCount()));
		layers[i] = layer;
	}
}

DirectionalLightCSMShadow::~DirectionalLightCSMShadow()
{
    
}


DirectionalLightCSMShadow::DirectionalLightCSMShadow()
{
    mRenderTarget = Framebuffer::createCSMShadowFBO(1024, 1024, mLayerCount);

}


void DirectionalLightCSMShadow::setRenderTargetSize(int width, int height)
{
    if (mRenderTarget != nullptr) mRenderTarget.reset();
    mRenderTarget = Framebuffer::createCSMShadowFBO(1024, 1024, mLayerCount);

}

