#include "Terrain.h"

TerrainLoader::TerrainLoader(Ogre::SceneManager* sm) {
	sceneManager = sm;
	terrainGlobals = new Ogre::TerrainGlobalOptions(); 
	terrainGlobals->setMaxPixelError(8);
	terrainGlobals->setCompositeMapDistance(3000);
	terrainGroup = new Ogre::TerrainGroup(sceneManager, Ogre::Terrain::ALIGN_X_Z, 513, 100.0f);

	Ogre::Terrain::ImportData& defaultimp = terrainGroup->getDefaultImportSettings();
	defaultimp.terrainSize = 513;
	defaultimp.worldSize = 100.0f;
	defaultimp.inputScale = 10;
	defaultimp.minBatchSize = 33;
	defaultimp.maxBatchSize = 65;
	// textures
	defaultimp.layerList.resize(3);
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

TerrainLoader::~TerrainLoader() {
	delete terrainGlobals;
}

void TerrainLoader::loadTerrain(std::string filename) {
	Ogre::Image img;
    img.load(filename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	terrainGroup->defineTerrain(0, 0, &img);
	terrainGroup->loadAllTerrains(true);
	Ogre::TerrainGroup::TerrainIterator ti = terrainGroup->getTerrainIterator();
	while(ti.hasMoreElements()) {
		Ogre::Terrain* t = ti.getNext()->instance;		
		initBlendMaps(t);
	}
	terrainGroup->freeTemporaryResources();
}

void TerrainLoader::initBlendMaps(Ogre::Terrain* terrain) {
	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
	Ogre::Real minHeight0 = 70;
	Ogre::Real fadeDist0 = 40;
	Ogre::Real minHeight1 = 70;
	Ogre::Real fadeDist1 = 15;
	float* pBlend1 = blendMap1->getBlendPointer();
	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y) {
		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x) {
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