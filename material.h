#pragma once
#include "core.h"

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial,
		WhiteMaterial,
		DepthMaterial,
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

		

	private:
		MaterialType mType;
		//	深度检测相关
		bool mDepthTest{ true };
		GLenum mDepthFunc{ GL_LESS };
		bool mDepthWrite{ true };

		//	POLYGON_OFFSET relative
		bool mPolygonOffsetState{ false };
		unsigned int mPolygonOffsetType{ GL_POLYGON_OFFSET_FILL };
		float mFactor{ 0.0f };
		float mUnit{ 0.0f };

	};

}
