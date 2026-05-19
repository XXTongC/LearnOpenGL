#include "renderer.h"

#include <algorithm>

using namespace GLframework;

Renderer::Renderer()
{
	mShaderLibrary.initialize();
}

void Renderer::projectObject(std::shared_ptr<Object> obj)
{
	if(obj->getType()==ObjectType::Mesh|| obj->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(obj);
		std::shared_ptr<Material> material = mesh->getMaterial();
		if(material->getColorBlendState())
		{
			mTransparentObjects.push_back(mesh);
		}else
		{
			mOpacityObjects.push_back(mesh);
		}
	}
	for(auto& t:obj->getChildren())
	{
		projectObject(t);
	}
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

	// 娓呯┖涓や釜闃熷垪
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	std::sort(mTransparentObjects.begin(), mTransparentObjects.end(), [camera](const std::shared_ptr<Mesh>& A,const std::shared_ptr<Mesh>& B)
		{
			//	1. 璁＄畻a鐨勭浉鏈虹郴鐨刏
			auto viewMatrix = camera->getViewMatrix();

			auto modelMatrixA = A->getModelMatrix();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionA = viewMatrix * worldPositionA;

			//2 璁＄畻b鐨勭浉鏈虹郴鐨刏
			auto modelMatrixB = B->getModelMatrix();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		});

	//	render shadowmap
	mShadowRenderer.render(camera, mOpacityObjects, dirLight, pointLights, mShaderLibrary);

	mSceneRenderPass.render(
		mOpacityObjects,
		mTransparentObjects,
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
