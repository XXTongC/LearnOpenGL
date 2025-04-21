#pragma once
#include "../shadow.h"


namespace GLframework
{
    class PointLightShadow : public Shadow
    {
    public:
        PointLightShadow(int,int);
        PointLightShadow();
        ~PointLightShadow() override;

        static void initializeSharedDepthTexture(int width, int height, int maxLights);
        static void setMAX_POINT_LIGHT(int value);
        static int getMAX_POINT_LIGHT();
    	static std::shared_ptr<Texture> getSharedDepthTexture();
      
        void setShadowMapIndex(int value);
        int getShadowMapIndex() const { return mShadowMapIndex; }
        void setRenderTargetSize(int width, int height) override;
        
    private:
        static std::shared_ptr<Texture> mSharedDepthTexture;
        static int MAX_POINT_LIGHTS;
    	int mShadowMapIndex;
        
    };
}
