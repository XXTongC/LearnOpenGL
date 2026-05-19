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
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 1. 璁剧疆褰撳墠甯х粯鍒剁殑鏃跺€欙紝opengl鐨勫繀瑕佺姸鎬佹満鍙傛暟
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 寮€鍚祴璇曘€佽缃熀鏈啓鍏ョ姸鎬侊紝鎵撳紑妯℃澘娴嬭瘯鍐欏叆
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff); //淇濊瘉浜嗘ā鏉跨紦鍐插彲浠ヨ娓呯悊

	// 榛樿棰滆壊娣峰悎
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. 娓呯悊鐢诲竷 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
