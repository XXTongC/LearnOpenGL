#include "renderer.h"

using namespace GLframework;

Renderer::Renderer()
{
	mShaderLibrary.initialize();
}

void Renderer::setClearColor(glm::vec3 color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);
}

std::shared_ptr<Shader> Renderer::getShader(MaterialType type)
{
	return mShaderLibrary.get(type);
}


void Renderer::render(
	std::shared_ptr<Scene> scene,
	Camera* camera,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	std::shared_ptr<AmbientLight> ambient,
	unsigned int fbo
)
{
	mFrameRenderState.begin(fbo);
	mRenderQueue.build(scene, camera);

	//	render shadowmap
	mShadowRenderer.render(camera, mRenderQueue.getOpacityObjects(), dirLight, pointLights, mShaderLibrary);

	mSceneRenderPass.render(
		mRenderQueue.getOpacityObjects(),
		mRenderQueue.getTransparentObjects(),
		camera,
		dirLight,
		spotLight,
		pointLights,
		ambient,
		mGlobalMaterial,
		mShaderLibrary
	);
}


void Renderer::msaaResolve(std::shared_ptr<Framebuffer> src, std::shared_ptr<Framebuffer> dst)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER,src->getFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->getFBO());
	glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, dst->getWidth(), dst->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
