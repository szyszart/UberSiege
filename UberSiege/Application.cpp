#include "Application.h"
#include "PuzzleBoard.h"

#include <iostream>
#include <cstdlib>

class LogicProcessor {
public:
	static void clearHandlers() { handlers.clear(); }
    static void registerEvents(luabind::object const& table);
	static UnitHandlers getHandlers() { return handlers; }
private:
    static UnitHandlers handlers;
};
UnitHandlers LogicProcessor::handlers;

void LogicProcessor::registerEvents(luabind::object const& table) {
	if(luabind::type(table) == LUA_TTABLE) {
		for(luabind::iterator i(table), end; i != end; ++i) {
			try {
				std::string key = luabind::object_cast<std::string>(i.key());
				luabind::object callback = *i;
				if(luabind::type(callback) == LUA_TFUNCTION) {
					handlers[key] = callback;
				}
				else std::cerr << "registerEvents: expected a function!" << std::endl;
			}
			catch(luabind::cast_failed e) {
				std::cerr << "registerEvents: invalid key, expected a string!" << std::endl;				
			}
		}
	}
	else std::cerr << "registerEvents: invalid arguments, expected a table!" << std::endl;
}

Application::Application(): root(NULL), sceneManager(NULL), listener(NULL), camera(NULL), renderer(NULL), window(NULL) {
	running = false;
	layoutFinder = new BoardLayoutFinder();

	lua = luaL_newstate();
	luaL_openlibs(lua);
	luabind::open(lua);
}

Application::~Application() {
	if(renderer) 
		CEGUI::OgreRenderer::destroySystem();	

	delete layoutFinder;
	delete listener;
	delete root;

	unitTypes.clear();

	if(lua)
		lua_close(lua);
}

bool Application::getEventHandler(std::string className, std::string eventName, luabind::object& obj) {
	UnitTypes::iterator it = unitTypes.find(className);
	if(it != unitTypes.end()) {
		UnitHandlers& handlers = it->second;
		UnitHandlers::iterator it2 = handlers.find(eventName);
		if(it2 != handlers.end()) {
			obj = it2->second;
			return true;
		}
		else return false;
	}
	return false;
}

Ogre::Vector3 parsePoint(luabind::object o) {
	return Ogre::Vector3(luabind::object_cast<double>(o[1]),
		luabind::object_cast<double>(o[2]),
		luabind::object_cast<double>(o[3]));
}

bool Application::loadMap(std::string name, MapInfo& info) {
	std::string filename = name + ".lua";
	if(luaL_dofile(lua, filename.c_str()) != 0) {
		std::cerr << lua_tostring(lua, -1) << std::endl;
        lua_pop(lua, 1);
		return false;
    }

	try {
		info.heightMap = luabind::object_cast<std::string>(luabind::globals(lua)["heightMap"]);
	}
	catch(luabind::cast_failed e) {
		std::cerr << "loadMap: invalid heightmap name!" << std::endl;				
		return false;
	}

	luabind::object layouts = luabind::globals(lua)["mountPoints"];
	if(luabind::type(layouts) == LUA_TTABLE) {
		for(luabind::iterator i(layouts), end; i != end; ++i) {			
			luabind::object mountPoint = *i;
			if(luabind::type(mountPoint) == LUA_TTABLE) {
				try {
					MountPoint mp;
					mp.pos = parsePoint(mountPoint["pos"]);
					mp.scale = parsePoint(mountPoint["scale"]);
					mp.rot = luabind::object_cast<double>(mountPoint["rot"]);
					info.mountPoints.push_back(mp);
				}
				catch(luabind::cast_failed e) {
					std::cerr << "loadMap: invalid mount point attributes!" << std::endl;			
					return false;
				}
			}
			else {
				std::cerr << "loadMap: invalid mount point, expected a table!" << std::endl;			
				return false;
			}
		}
	}
	else {
		std::cerr << "loadMap: invalid mount points, expected a table!" << std::endl;				
		return false;
	}

	luabind::object path = luabind::globals(lua)["path"];
	if(luabind::type(path) == LUA_TTABLE) {
		try {
			info.pathStart = parsePoint(path["startPoint"]);
			info.pathEnd = parsePoint(path["endPoint"]);
		}
		catch(luabind::cast_failed e) {
			std::cerr << "loadMap: invalid path points!" << std::endl;				
			return false;
		}
	}
	else {
		std::cerr << "loadMap: invalid path points, expected a table!" << std::endl;				
		return false;
	}
	return true;
}

void Application::processConfig() {	
    if(luaL_dofile(lua, "config.lua") != 0) {
		std::cerr << lua_tostring(lua, -1) << std::endl;
        lua_pop(lua, 1);
		return;
    }

	readLayouts();
	readKeyBindings();
	processUnitScripts();
}

void Application::processUnitScripts() {
	luabind::module(lua) [
		luabind::class_<Simulation>("Simulation")
			.def("moveForwards",		&Simulation::moveForwards)
			.def("moveBackwards",		&Simulation::moveBackwards)
			.def("inflictDamage",		&Simulation::inflictDamage)
			.def("requestAnimation",	&Simulation::requestAnimation)
			.def("queueAnimation",		&Simulation::queueAnimation)
			.def("stopAnimations",		&Simulation::stopAnimations)
			.def("getNearestEnemy",		&Simulation::getNearestEnemy)
			.def("getDistance",			&Simulation::getDistance)
			.def("getEnemies",			&Simulation::getEnemiesLua, luabind::raw(_2)),
		luabind::class_<Unit>("Unit")
			.property("pos", &Unit::getPos, &Unit::setPos)
			.def("getPos", &Unit::getPos),
		luabind::def("registerEvents", &LogicProcessor::registerEvents)
	];

	std::vector<std::string> names;
	luabind::object layouts = luabind::globals(lua)["units"];
	if(luabind::type(layouts) == LUA_TTABLE) {
		for(luabind::iterator i(layouts), end; i != end; ++i) {
			try {
				std::string unitClass = luabind::object_cast<std::string>(i.key());
				std::string filename = luabind::object_cast<std::string>(*i);
				std::cout << "processUnitScripts: processing logic for " << unitClass << std::endl;										

				LogicProcessor::clearHandlers();

				if(luaL_dofile(lua, filename.c_str()) != 0) {
					std::cerr << lua_tostring(lua, -1) << std::endl;
					lua_pop(lua, 1);										
				}	
				else {
					unitTypes[unitClass] = LogicProcessor::getHandlers();
				}	
			}
			catch(luabind::cast_failed e) {
				std::cerr << "processUnitScripts: invalid unit data, expected a string!" << std::endl;				
			}
		}
		LogicProcessor::clearHandlers();
	}
	else std::cerr << "processUnitScripts: invalid unit data, expected a table!" << std::endl;
}

void Application::readKeyBindings() {
	luabind::object layouts = luabind::globals(lua)["bindings"];
	if(luabind::type(layouts) == LUA_TTABLE) {
		for(luabind::iterator i(layouts), end; i != end; ++i) {
			try {
				std::string action = luabind::object_cast<std::string>(i.key());
				std::string key = luabind::object_cast<std::string>(*i);
				std::map<std::string, Action>::iterator itA = actionNames.find(action);
				if(itA == actionNames.end()) {
					std::cerr << "readKeyBindings: invalid action name: " << action << std::endl;
					continue;
				}
				
				std::map<std::string, OIS::KeyCode>::iterator itK = keyNames.find(key);
				if(itK == keyNames.end()) {
					std::cerr << "readKeyBindings: invalid key name: " << key << std::endl;
					continue;
				}

				listener->bindKey(itK->second, itA->second);
				std::cout << "readKeyBindings: adding key binding " << action << " -> " << key << std::endl;
			}
			catch(luabind::cast_failed e) {
				std::cerr << "readKeyBindings: invalid key, expected a string!" << std::endl;				
			}
		}
	}
	else std::cerr << "readKeyBindings: invalid key bindings, expected a table!" << std::endl;				
}

void Application::readLayouts() {
	luabind::object layouts = luabind::globals(lua)["layouts"];
	if(luabind::type(layouts) == LUA_TTABLE) {
		for(luabind::iterator i(layouts), end; i != end; ++i) {
			try {
				std::string key = luabind::object_cast<std::string>(i.key());				
				luabind::object descr = *i;
				if(luabind::type(descr) == LUA_TTABLE) {
					parseOneLayout(key, descr);
				}
				else std::cerr << "readLayouts: invalid layout definition: " << key << std::endl;
			}
			catch(luabind::cast_failed e) {
				std::cerr << "readLayouts: invalid key, expected a string!" << std::endl;				
			}
		}
	}
	else std::cerr << "readLayouts: invalid layout descriptions, expected a table!" << std::endl;
}

void Application::parseOneLayout(std::string name, luabind::object& descr) {
	try {
		std::string unitName = luabind::object_cast<std::string>(descr["unit"]);
		luabind::object layout = descr["layout"];
		if(luabind::type(layout) == LUA_TTABLE) {
			try {
				std::string topRow = luabind::object_cast<std::string>(layout[1]);
				std::string bottomRow = luabind::object_cast<std::string>(layout[2]);
				std::cout << "Adding " << name << " as " << topRow << ", " << bottomRow << std::endl;
				layoutFinder->addLayout(unitName, std::pair<std::string, std::string>(topRow, bottomRow));
			}
			catch(luabind::cast_failed e) {
				std::cerr << "parseOneLayout: invalid key, expected a string!" << std::endl;				
			}
		}
		else std::cerr << "Error: invalid layout description for " << name << std::endl;				
	}
	catch(luabind::cast_failed e) {
		std::cerr << "Error: invalid layout description for " << name << std::endl;				
	}
}

void Application::loadResources() {
	Ogre::ConfigFile cf;
	
	#ifdef DEBUG
	cf.load("resources_d.cfg");
	#else
	cf.load("resources.cfg");
	#endif
	
	Ogre::ConfigFile::SectionIterator sIter = cf.getSectionIterator();
	Ogre::String sName, tName, dataName;
	while(sIter.hasMoreElements()) {
		sName = sIter.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap* settings = sIter.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for(i = settings->begin(); i != settings->end(); ++i) {
			tName = i->first;
			dataName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(dataName, tName, sName);
		}
	}

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

bool Application::startup() {
	if(!root)
		root = new Ogre::Root("plugins_d.cfg");
	if(!root->showConfigDialog())
		return false;
	window = root->initialise(true, "Uber Siege");
	sceneManager = root->createSceneManager(Ogre::ST_GENERIC);

	camera = sceneManager->createCamera("Camera");
	Ogre::Viewport* viewport = window->addViewport(camera);
	viewport->setBackgroundColour(Ogre::ColourValue(0.0, 0.0, 0.0));
	camera->setAspectRatio(Ogre::Real(viewport->getActualWidth() / viewport->getActualHeight()));

	loadResources();

	renderer = &CEGUI::OgreRenderer::bootstrapSystem();

	listener = new InputFrameListener(this);
	root->addFrameListener(listener);

	processConfig();

	return true;
}

void Application::renderFrame() {
	Ogre::WindowEventUtilities::messagePump();
	running = root->renderOneFrame();
}

void Application::go() {
	running = true;
	while(running) {
		renderFrame();
	}
}