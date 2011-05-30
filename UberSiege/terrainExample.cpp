// na podstawie  http://www.ogre3d.org/tikiwiki/Basic+Tutorial+3


#include "Ogre\ExampleApplication.h"
#include "OGRE\Terrain\OgreTerrain.h"
#include "OGRE\Terrain\OgreTerrainGroup.h"
#include "OGRE\OgrePrerequisites.h"

class ExampleTerrain : public ExampleApplication
{
	private:
		Ogre::TerrainGlobalOptions* mTerrainGlobals;
		Ogre::TerrainGroup* mTerrainGroup;
		bool mTerrainsImported;	

		void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
		{
			img.load("terrain1_heightmap513.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			if (flipX)
				img.flipAroundY();
			if (flipY)
				img.flipAroundX();
 
		}
		void defineTerrain(long x, long y)
		{
			Ogre::String filename = mTerrainGroup->generateFilename(x, y);
			if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
			{
				mTerrainGroup->defineTerrain(x, y);
			}
			else
			{
				Ogre::Image img;
				getTerrainImage(x % 2 != 0, y % 2 != 0, img);
				mTerrainGroup->defineTerrain(x, y, &img);
				mTerrainsImported = true;
			}
		}

		void createScene(void)
		{
			Ogre::Camera* mCamera = sceneManager->createCamera("MainCam");
            mCamera->setPosition(Ogre::Vector3(1683, 500, 2116));
			mCamera->lookAt(Ogre::Vector3(1963, 500, 1660));
			mCamera->setNearClipDistance(0.1);
			mCamera->setFarClipDistance(50000);
 
			if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
			{
				mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
			}
 
			Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
			Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);
 
			Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
			lightdir.normalise();
 
			Ogre::Light* light = mSceneMgr->createLight("tstLight");
			light->setType(Ogre::Light::LT_DIRECTIONAL);
			light->setDirection(lightdir);
			light->setDiffuseColour(Ogre::ColourValue::White);
			light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
 
			mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
 
			mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();
 
			mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, 513, 10000.0f);
			mTerrainGroup->setFilenameConvention(Ogre::String("BasicTutorial3Terrain"), Ogre::String("dat"));
			mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
 
			configureTerrainDefaults(light);
 
			for (long x = 0; x <= 0; ++x)
				for (long y = 0; y <= 0; ++y)
					defineTerrain(x, y);
 
			// sync load since we want everything in place when we start
			mTerrainGroup->loadAllTerrains(true);
 
			if (mTerrainsImported)
			{
				Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
				while(ti.hasMoreElements())
				{
					Ogre::Terrain* t = ti.getNext()->instance;
					initBlendMaps(t);
				}
			}
		 
			mTerrainGroup->freeTemporaryResources();
			mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

			/*bool combine = false;
			if (combine)
			{
				Ogre::Image combined;
				std::string file_name("terrain1_tex");
	
				combined.loadTwoImagesAsRGBA((file_name+".png").c_str(), (file_name+"_SPEC.bmp").c_str(), 
				  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
				combined.save((file_name+"_diffusespecular.png").c_str());
 
			   combined.loadTwoImagesAsRGBA((file_name+"_NORM.tga").c_str(), (file_name+"_DISP.bmp").c_str(), 
				  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
			   combined.save( (file_name+"_normalheight.png").c_str() );
			}*/
		}

	void initBlendMaps(Ogre::Terrain* terrain)
	{
		Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
		Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
		Ogre::Real minHeight0 = 70;
		Ogre::Real fadeDist0 = 40;
		Ogre::Real minHeight1 = 70;
		Ogre::Real fadeDist1 = 15;
		float* pBlend1 = blendMap1->getBlendPointer();
		for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
		{
			for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
			{
				Ogre::Real tx, ty;
 
				blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
				Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
				Ogre::Real val = (height - minHeight0) / fadeDist0;
				val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
 
				val = (height - minHeight1) / fadeDist1;
				val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
				*pBlend1++ = val;
			}
		}
		blendMap0->dirty();
		blendMap1->dirty();
		blendMap0->update();
		blendMap1->update();
	}
	void configureTerrainDefaults(Ogre::Light* light)
	{
		// Configure global
		mTerrainGlobals->setMaxPixelError(8);
		// testing composite map
		mTerrainGlobals->setCompositeMapDistance(3000);
 
		// Important to set these so that the terrain knows what to use for derived (non-realtime) data
		mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
		mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
		mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
 
		// Configure default import settings for if we use imported image
		Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
		defaultimp.terrainSize = 513;
		defaultimp.worldSize = 10000.0f;
		defaultimp.inputScale = 300;
		defaultimp.minBatchSize = 33;
		defaultimp.maxBatchSize = 65;
		// textures
		defaultimp.layerList.resize(3);
		//defaultimp.layerList[0].worldSize = 200;
		//defaultimp.layerList[0].textureNames.push_back("terrain1_tex_diffusespecular.png");
		//defaultimp.layerList[0].textureNames.push_back("terrain1_tex_normalheight.png");
		defaultimp.layerList[0].worldSize = 100;
		defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
		defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
		defaultimp.layerList[1].worldSize = 30;
		defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
		defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
		defaultimp.layerList[2].worldSize = 200;
		defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
		defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
	}
};

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpszArgument, int nFunsterStil)
{
	
	ExampleTerrain app;
	app.go();	

	return 0;
}
