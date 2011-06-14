#ifndef UBERS_APPLICATION_H
#define UBERS_APPLICATION_H

#include "CEGUI.h"
#include "Ogre/Ogre.h"
#include "OGRE/Terrain/OgreTerrain.h"
#include "OGRE/Terrain/OgreTerrainGroup.h"
#include "RendererModules/Ogre/CEGUIOgreRenderer.h"
#include "OGRE/OgrePrerequisites.h"

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/raw_policy.hpp>

#include <map>

#include "InputFrameListener.h"
#include "PuzzleBoard.h"
#include "Mapinfo.h"

typedef std::map<std::string, luabind::object> UnitHandlers;
typedef std::map<std::string, UnitHandlers> UnitTypes;

class Application {
public:
	Application();
	~Application();

	virtual void loadResources();
	virtual bool startup();
	virtual bool isRunning() { return running; }
	virtual void go();

	BoardLayoutFinder* getLayoutFinder() { return layoutFinder; }
	Ogre::Camera* getCamera() { return camera; }
	Ogre::RenderWindow* getRenderWindow() { return window; }
	CEGUI::OgreRenderer* getGUIRenderer() { return renderer; }
	Ogre::SceneManager* getSceneManager() { return sceneManager; }
	bool loadMap(std::string name, MapInfo& info);

	bool getEventHandler(std::string className, std::string eventName, luabind::object& obj);

	static std::map<std::string, OIS::KeyCode> keyNames;
	static std::map<std::string, Action> actionNames;
private:
	void renderFrame();
	void processConfig();
	void processUnitScripts();

	void readKeyBindings();
	void readLayouts();
	void parseOneLayout(std::string name, luabind::object&);	

	Ogre::RenderWindow* window;
	Ogre::SceneManager* sceneManager;
	Ogre::Root* root;
	Ogre::Camera* camera;
	CEGUI::OgreRenderer* renderer;
	InputFrameListener* listener;
	lua_State* lua;
	bool running;	

	BoardLayoutFinder* layoutFinder;
	UnitTypes unitTypes;
};

#endif