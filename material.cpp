#include "material.h"

using namespace GLframework;

unsigned Material::getCullFace() const
{
	return mCullFace;
}

bool Material::getFaceCullingState() const
{
	return mFaceCullingState;
}

unsigned Material::getFrontFace() const
{
	return mFrontFace;
}

void Material::setCullFace(unsigned value)
{
	mCullFace = value;
}

void Material::setFaceCullingState(bool value)
{
	mFaceCullingState = value;
}

void Material::setFrontFace(unsigned value)
{
	mFrontFace = value;
}

bool Material::getColorBlendState() const
{
	return mColorBlendState;
}

unsigned Material::getDFactor() const
{
	return mDFactor;
}

unsigned Material::getSFactor() const
{
	return mSFactor;
}

void Material::setColorBlendState(bool value)
{
	mColorBlendState = value;
}

void Material::setDFactor(unsigned value)
{
	mDFactor = value;
}

void Material::setSFactor(unsigned value)
{
	mSFactor = value;
}

void Material::setOpacity(float value)
{
	if (value < 0) mOpacity = 0.0f;
	else if (value > 1) mOpacity = 1.0f;
	else mOpacity = value;
}

float Material::getOpacity() const
{
	return mOpacity;
}



void Material::setPreStencilPreSettingType(PreStencilType preType)
{
	switch (preType)
	{
	case PreStencilType::Normal:
		//	ģ����Կ�ʼ���
		this->setStencilState(true);
	//	ģ����Խ��д�����
		this->setSFail(GL_KEEP);
		this->setZFail(GL_KEEP);
		this->setZPass(GL_REPLACE);
	//	��������д��
		this->setStencilMask(0xff);
	//	ģ����Թ���
		this->setStencilFunc(GL_ALWAYS);
		this->setStencilRef(1);
		this->setStencilFuncMask(0xff);
		break;

	case PreStencilType::Outlining:
		this->setDepthTest(false);
	//	ģ����Կ�ʼ���
		this->setStencilState(true);
	//	ģ����Խ��д�����
		this->setSFail(GL_KEEP);
		this->setZFail(GL_KEEP);
		this->setZPass(GL_KEEP);
	//	��������д��
		this->setStencilMask(0x00);
	//	ģ����Թ���
		this->setStencilFunc(GL_NOTEQUAL);
		this->setStencilRef(1);
		this->setStencilFuncMask(0xff);
		break;

	case PreStencilType::Custom:
		std::cout << "Material Stencil Settings as Custom" << std::endl;
		break;
	}
	
}

PreStencilType Material::getStencilPreSettingType() const
{
	return mStencilPreSettingType;
}


float Material::getFactor() const
{
	return mFactor;
}

float Material::getUnit() const
{
	return mUnit;
}

void Material::setFactor(float value)
{
	mFactor = value;
}

void Material::setUnit(float value)
{
	mUnit = value;
}

bool Material::getPolygonOffsetState() const
{
	return mPolygonOffsetState;
}

unsigned Material::getPolygonOffsetType() const
{
	return mPolygonOffsetType;
}

void Material::setPolygonOffsetState(bool value)
{
	mPolygonOffsetState = value;
}

void Material::setPolygonOffsetType(unsigned PolygonOffSetType)
{
	mPolygonOffsetType = PolygonOffSetType;
}


MaterialType Material::getMaterialType() const
{
	return mType;
}

GLenum Material::getDepthFunc() const
{
	return mDepthFunc;
}

bool Material::getDepthTest() const
{
	return mDepthTest;
}

bool Material::getDepthWrite() const
{
	return mDepthWrite;
}

void Material::setDepthFunc(GLenum value)
{
	mDepthFunc = value;
}

void Material::setDepthTest(bool value)
{
	mDepthTest = value;
}

void Material::setDepthWrite(bool value)
{
	mDepthWrite = value;
}

void Material::setMaterialType(MaterialType type)
{
	mType = type;
}

#pragma region plentyofStencil!
unsigned Material::getSFail() const
{
	return mSFail;
}

void Material::setSFail(unsigned value)
{
	mSFail = value;
}

unsigned Material::getStencilFunc() const
{
	return mStencilFunc;
}

unsigned Material::getStencilFuncMask() const
{
	return mStencilFuncMask;
}

unsigned Material::getStencilMask() const
{
	return mStencilMask;
}

unsigned Material::getStencilRef() const
{
	return mStencilRef;
}

bool Material::getStencilState() const
{
	return mStencilTestState;
}

unsigned Material::getZFail() const
{
	return mZFail;
}

unsigned Material::getZPass() const
{
	return mZPass;
}

void Material::setStencilFunc(unsigned value)
{
	mStencilFunc = value;
}

void Material::setStencilFuncMask(unsigned value)
{
	mStencilFuncMask = value;
}

void Material::setStencilMask(unsigned value)
{
	mStencilMask = value;
}

void Material::setStencilRef(unsigned value)
{
	mStencilRef = value;	
}

void Material::setStencilState(bool value)
{
	mStencilTestState = value;
}

void Material::setZFail(unsigned value)
{
	mZFail = value;
}

void Material::setZPass(unsigned value)
{
	mZPass = value;
}
#pragma endregion

