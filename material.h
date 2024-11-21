#pragma once
#include "core.h"

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial,
		WhiteMaterial,
		DepthMaterial,
		OpacityMaskMaterial,
		ScreenMaterial,
		CubeMaterial,
	};

	enum class PreStencilType
	{
		Normal,
		Outlining,
		Custom,
	};

	class Material
	{
	public:
		Material(){}
		~Material(){}
		MaterialType getMaterialType() const;
		void setMaterialType(MaterialType type);
		void setDepthTest(bool value);
		void setDepthWrite(bool value);
		void setDepthFunc(GLenum value);
		bool getDepthTest() const;
		bool getDepthWrite() const;
		GLenum getDepthFunc() const;

		bool getPolygonOffsetState() const;
		void setPolygonOffsetState(bool value);

		unsigned int getPolygonOffsetType() const;
		void setPolygonOffsetType(unsigned int PolygonOffSetType);
		
		float getFactor() const;
		void setFactor(float value);

		float getUnit() const;
		void setUnit(float value);

		//	STENCIL relative set and get functions
		void setSFail(unsigned int value);
		void setZFail(unsigned int value);
		void setZPass(unsigned int value);
		void setStencilMask(unsigned int value);
		void setStencilFunc(unsigned int value);
		void setStencilRef(unsigned int value);
		void setStencilFuncMask(unsigned int value);
		void setStencilState(bool value);
		//	use system provided pre settings
		void setPreStencilPreSettingType(PreStencilType preType);

		unsigned int getSFail() const;
		unsigned int getZFail() const;
		unsigned int getZPass() const;
		unsigned int getStencilMask() const;
		unsigned int getStencilFunc() const;
		unsigned int getStencilRef() const;
		unsigned int getStencilFuncMask() const;
		PreStencilType getStencilPreSettingType() const;
		//	to decide Stencil open or not
		bool getStencilState() const;

		//	to decide ColorBlend open or not
		void setColorBlendState(bool value);
		void setSFactor(unsigned int value);
		void setDFactor(unsigned int value);
		void setOpacity(float value);

		bool getColorBlendState() const;
		float getOpacity() const;
		unsigned int getSFactor() const;
		unsigned int getDFactor() const;

		void setFaceCullingState(bool value);
		void setFrontFace(unsigned int value);
		void setCullFace(unsigned int value);

		bool getFaceCullingState() const;
		unsigned int getFrontFace() const;
		unsigned int getCullFace() const;



	private:
		MaterialType	mType;
		//	DEPTH TEST relative
		bool			mDepthTest{ true };
		GLenum			mDepthFunc{ GL_LESS };
		bool			mDepthWrite{ true };

		//	POLYGON_OFFSET relative
		bool			mPolygonOffsetState{ false };
		unsigned int	mPolygonOffsetType{ GL_POLYGON_OFFSET_FILL };
		float			mFactor{ 0.0f };
		float			mUnit{ 0.0f };

		//	STENCIL relative
		bool			mStencilTestState{ false };
		unsigned int	mSFail{ GL_KEEP };		//stencil test fail
		unsigned int	mZFail{ GL_KEEP };		//stencil test pass but depth test fail
		unsigned int	mZPass{ GL_KEEP };		//stencil test pass
		unsigned int	mStencilMask{ 0xff };	//stencil Bitmask
		unsigned int	mStencilFunc{ GL_ALWAYS };
		unsigned int	mStencilRef{ 0 };
		unsigned int	mStencilFuncMask{ 0xff };
		PreStencilType	mStencilPreSettingType{ PreStencilType::Custom };

		//	COLOR BLEND relative
		bool			mColorBlendState{ false };
		unsigned int	mSFactor{ GL_SRC_ALPHA };
		unsigned int	mDFactor{ GL_ONE_MINUS_SRC_ALPHA };
		float			mOpacity{ 1.0f };

		//	FACE CULLING relative
		bool			mFaceCullingState{ false };
		unsigned int	mFrontFace{ GL_CCW };
		unsigned int	mCullFace{ GL_BACK };
	};

}
