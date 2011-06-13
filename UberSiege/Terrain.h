#ifndef UBERS_TERRAIN_H
#define UBERS_TERRAIN_H

#include "Ogre/Ogre.h"
#include "OGRE/Terrain/OgreTerrain.h"
#include "OGRE/Terrain/OgreTerrainGroup.h"

class TerrainLoader {
public:
	TerrainLoader(Ogre::SceneManager* sm);
	~TerrainLoader();
	void loadTerrain(std::string filename);
private:
	Ogre::TerrainGlobalOptions* terrainGlobals;
	Ogre::TerrainGroup* terrainGroup;
	Ogre::SceneManager* sceneManager;

	void initBlendMaps(Ogre::Terrain* terrain);
};

#endif