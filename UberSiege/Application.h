#ifndef UBERS_APPLICATION_H
#define UBERS_APPLICATION_H

#include "CEGUI.h"
#include "Ogre/Ogre.h"
#include "OGRE/Terrain/OgreTerrain.h"
#include "OGRE/Terrain/OgreTerrainGroup.h"
#include "RendererModules/Ogre/CEGUIOgreRenderer.h"
#include "OGRE\OgrePrerequisites.h"

#include "InputFrameListener.h"
#include "PuzzleBoard.h"

class Application {
public:
	Application();
	~Application();

	virtual void loadResources();
	virtual bool startup();
	virtual void createScene();
	virtual bool isRunning() { return running; }
	virtual void go();
	virtual bool initGUI();

	PuzzleBoardWidget* getBoardWidget() { return boardWidget; }
	PuzzleBoardWidget* getBoardWidget2() { return boardWidget2; }
	BoardLayoutFinder* getLayoutFinder() { return layoutFinder; }
	PuzzleBoard* getBoard() { return board; }
	PuzzleBoard* getBoard2() { return board2; }
	Ogre::SceneNode* getCameraNode() { return cameraNode; }
private:
	void renderFrame();	
	void defineTerrain(long x, long y);
	void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img);
	void initBlendMaps(Ogre::Terrain* terrain);
	void configureTerrainDefaults(Ogre::Light* light);

	Ogre::SceneManager* sceneManager;
	Ogre::Root* root;
	Ogre::Camera* camera;
	Ogre::SceneNode* cameraNode;
	InputFrameListener* listener;
	CEGUI::OgreRenderer* renderer;
	bool running;

	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;
	bool mTerrainsImported;	

	// TODO: przenieœæ to w inne miejsce
	PuzzleBoard* board;
	PuzzleBoard* board2;
	PuzzleBoardWidget* boardWidget;
	PuzzleBoardWidget* boardWidget2;
	BoardLayoutFinder* layoutFinder;
};

#endif