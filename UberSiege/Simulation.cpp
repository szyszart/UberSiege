#include "Simulation.h"
#include "Application.h"
using namespace std;

const double Simulation::SPAWN_ZONE_WIDTH = 0.1;

Player::Player(string name) {
	this->name = name;
	board = new PuzzleBoard();
}

Player::~Player() {
	delete board;
}

Ogre::AnimationState* Unit::fetchAnimation(string name) {
	map<string, Ogre::AnimationState*>::iterator it = anims.find(name);
	/*
	std::cout << "Animation list" << std::endl;
	Ogre::AnimationStateSet* set = entity->getAllAnimationStates();
	Ogre::AnimationStateIterator iter = set->getAnimationStateIterator();
	while(iter.hasMoreElements())
		std::cout << iter.getNext()->getAnimationName() << std::endl;
	*/
	
	Ogre::AnimationState* anim = NULL;
	if(it == anims.end()) {		
		anim = entity->getAnimationState(name);
		if(!anim)
			return NULL;
		anim->setEnabled(true);
		anim->setTimePosition(0.0);
		anim->setLoop(false);
	}
	return anim;
}

bool Unit::addAnimation(string name) {
	Ogre::AnimationState* anim = fetchAnimation(name);
	if(!anim)
		return false;
	anims[name] = anim;
	return true;
}

void Unit::clearAnimations() {
	anims.clear();
	animQueue.clear(); 
	currentQueuedAnim = NULL;
}

Simulation::Simulation(Application* app) {
	this->app = app;
	sceneManager = app->getSceneManager();
	camera = app->getCamera();
	loader = new TerrainLoader(sceneManager);
	createScene();
}

Simulation::~Simulation() {
	delete loader;
}

list<Unit*> Simulation::getEnemies(Unit* unit) {
	list<Unit*> result;
	for(list<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		Unit* u = *it;
		if(unit->getPlayer() != u->getPlayer())
			result.push_back(u);
	}
	return result;
}

void Simulation::moveForwards(Unit* unit, double vel) {
	unit->setVel(unit->getDirection() * abs(vel));	
}

void Simulation::moveBackwards(Unit* unit, double vel) {
	unit->setVel(-unit->getDirection() * abs(vel));
}

Unit* Simulation::getNearestEnemy(Unit* unit) {
	double maxDist = 1.0;
	Unit* found = NULL;
	for(list<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		Unit* u = *it;
		double dist = abs(unit->getPos() - u->getPos());
		if((unit->getPlayer() != u->getPlayer()) && (dist <= maxDist)) {
			found = u;
			maxDist = dist;
		}
	}	
	return found;
}

luabind::object Simulation::getEnemiesLua(lua_State* lua, Unit* unit) {
	luabind::object result = luabind::newtable(lua);  
    list<Unit*> enemies = getEnemies(unit);
    int index = 1;
	for(list<Unit*>::iterator it = enemies.begin(); it != enemies.end(); ++it) {
		result[index++] = *it;
	}	
    return result;
}

void Simulation::inflictDamage(Unit* unit, Unit* enemy, unsigned int damage) {
	enemy->setHP(enemy->getHP() - damage);
	cout << "hp left " << enemy->getHP() << endl;
}

double Simulation::getDistance(Unit* u1, Unit* u2) {
	double pos1 = (path * u1->getPos()).length();
	double pos2 = (path * u2->getPos()).length();
	return abs(pos1 - pos2);
}

bool Simulation::requestAnimation(Unit* unit, string animName) {
	return unit->addAnimation(animName);
}

void Simulation::queueAnimation(Unit* unit, std::string animName) {
	unit->enqueueAnimation(animName);
}

void Simulation::stopAnimations(Unit* unit) {
	unit->clearAnimations();
}

bool Simulation::loadMap(MapInfo& info) {
	createScene();

	// TODO: przenieœæ do pliku konfiguracyjnego
	Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
	lightdir.normalise();
 
	Ogre::Light* light = sceneManager->createLight("tstLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));

	loader->loadTerrain(info.heightMap);
	castleNode = NULL;
	for(vector<MountPoint>::iterator it = info.mountPoints.begin(); it != info.mountPoints.end(); ++it) {
		Ogre::Entity* ent = sceneManager->createEntity("ZamekMesh.mesh");		
		Ogre::SceneNode* node = sceneManager->getRootSceneNode()->createChildSceneNode();
		if(!castleNode)
			castleNode = node;
		node->attachObject(ent);
		node->setPosition(it->pos);
		node->setScale(it->scale);
		node->yaw(Ogre::Radian(it->rot));
	}
	
	path = info.pathEnd - info.pathStart;
	this->info = info;
	return true;
}

void Simulation::addUnit(Player* player, std::string className) {
	// TODO: dodaj odczyt pocz¹tkowych wartoœci HP
	Unit* unit = new Unit(player, className, 100);
	// TODO: nazwa pliku .mesh odczytywana z pliku konfiguracyjnego!
	unit->setDirection(player->getDirection());
	Ogre::Entity* ent = sceneManager->createEntity("infantry.mesh");
	unit->setWidth(ent->getBoundingRadius());
	unit->setEntity(ent);	
	unit->setPos(player->getSpawnZone().start);
	player->enqueueUnit(unit);
}

bool Simulation::addUnit(Unit* unit) {	
	if(!unit->getEntity())
		return false;

	Ogre::SceneNode* node = sceneManager->getRootSceneNode()->createChildSceneNode();
	node->attachObject(unit->getEntity());
	node->yaw(Ogre::Radian(unit->getPlayer()->getYaw()));
	unit->setSceneNode(node);	
	units.push_back(unit);
	return true;
}

void Simulation::removeUnit(list<Unit*>::iterator it) {
	Unit* u = *it;
	Ogre::SceneNode* node = u->getSceneNode();
	if(node) {
		node->removeAndDestroyAllChildren();		
		Ogre::SceneNode* parent = node->getParentSceneNode();
		parent->removeAndDestroyChild(node->getName());
	}
	units.erase(it);
	delete u;
}

void Simulation::createScene() {
	sceneManager->clearScene();

	cameraNode = sceneManager->getRootSceneNode()->createChildSceneNode();
	cameraNode->setPosition(0, 100, 50);
	cameraNode->attachObject(camera);

	camera->setPosition(Ogre::Vector3(0, 0, 0));
	camera->lookAt(Ogre::Vector3(0, 0, 0));
	camera->setNearClipDistance(0.1);
	camera->setFarClipDistance(50000);
	sceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	sceneManager->setSkyDome(true, "CloudySky", 5, 8);	
}

Player* Simulation::getPlayer(unsigned int num) {	
	return players[num];
}

bool Simulation::addPlayer(Player* player) {	
	if(players.size() >= 2)
		return false;
	
	Zone spawnZone;
	Ogre::Radian angle;
	switch(players.size()) {
		case 0:		
			spawnZone.start = 0.0;
			spawnZone.end = SPAWN_ZONE_WIDTH;
			angle = path.angleBetween(Ogre::Vector3::UNIT_Z);
			player->setDirection(1.0);
			break;
		case 1:
			spawnZone.start = 1.0 - SPAWN_ZONE_WIDTH;
			spawnZone.end = 1.0;
			angle = path.angleBetween(Ogre::Vector3::UNIT_Z) - Ogre::Degree(180.0);
			player->setDirection(-1.0);
			break;
	};
	player->setSpawnZone(spawnZone);
	player->setYaw(angle.valueRadians());
	players.push_back(player);
	return true;
}

bool Simulation::collisions(Unit* unit) {
	for(list<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		Unit* u = *it;
		if(u == unit)
			continue;
		if(collide(u, unit))
			return true;
	}
	return false;
}

bool Simulation::collide(Unit* u1, Unit *u2) {
	double pos1 = (path * u1->getPos()).length();
	double pos2 = (path * u2->getPos()).length();
	double w1 = u1->getWidth();
	double w2 = u2->getWidth();
	return (((pos2 - w2 >= pos1 - w1) && (pos2 - w2 <= pos1 + w1))
		|| ((pos2 + w2 >= pos1 - w1) && (pos2 + w2 <= pos1 + w1)));
}

bool Simulation::inBounds(Unit* unit) {
	double pos = unit->getPos();
	return (pos >= 0.0 && pos <= 1.0);
}

bool Simulation::isZoneEmpty(Zone z) {
	// TODO: wydajniej, na bie¿¹co uaktualniaj informacje o jednostkach wchodz¹cych do strefy
	for(list<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		Unit* u = *it;
		double w = u->getWidth();
		double pos = (path * u->getPos()).length();

		if( ((pos - w >= z.start) && (pos - w <= z.end)) ||		
			((pos + w >= z.start) && (pos + w <= z.end)) )
			return false;
	}
	return true;
}

bool Simulation::isInSpawnZone(Unit* u) {
	for(vector<Player*>::iterator it = players.begin(); it != players.end(); ++it) {
		Player* p = *it;
		if(p == u->getPlayer())
			continue;
		Zone z = p->getSpawnZone();
		if((u->getPos() >= z.start) && (u->getPos() <= z.end))
			return true;
	}
}

void Simulation::tick(float timeElapsed) {
	for(vector<Player*>::iterator it = players.begin(); it != players.end(); ++it) {
		Player* p = *it;
		if(p->hasWaitingUnits()) {
			Zone spawnZone = p->getSpawnZone();
			if(isZoneEmpty(spawnZone)) {
				Unit* u = p->peekUnit();
				if(!collisions(u)) {
					p->dequeueUnit();
					addUnit(u);
					callHandler(u, "Init");
				}
				else cout << "Nope, there's a collision" << endl;
			}
		}
	}
	
	list<Unit*>::iterator it = units.begin();
	while(it != units.end()) {
		Unit* u = *it;
		if(!u->isActive()) {
			if(!u->hasQueuedAnimations()) {
				removeUnit(it++);
				continue;
			}
			else u->advanceAnimations(timeElapsed);
		}
		else {
			callHandler(u, "Tick");
			if(u->isDead()) {			
				callHandler(u, "Die");
				u->setActive(false);
			}
			else if(isInSpawnZone(u)) {
				callHandler(u, "Leave");
				u->setActive(false);
			}		
			else {
				// oblicz nowe po³o¿enie jednostki
				double oldPos = u->getPos();
				double newPos = oldPos + u->getVel() * timeElapsed;			
				u->setPos(newPos);
				// sprawdŸ, czy wyst¹pi kolizja
				if(!collisions(u) && inBounds(u)) {
					Ogre::SceneNode* node = u->getSceneNode();
					if(node) {				
						node->setPosition(info.pathStart + path * u->getPos());
						u->advanceAnimations(timeElapsed);
					}
				}
				else u->setPos(oldPos);
			}	
		}
		++it;
	}
}

void Simulation::callHandler(Unit* u, std::string eventName) {
	luabind::object callback;
	if(!app->getEventHandler(u->getClassName(), eventName, callback))
		return;
	if(luabind::type(callback) == LUA_TFUNCTION) {
		try {
			luabind::call_function<void>(callback, this, u, NULL);	// TODO: przeka¿ informacje o zdarzeniu zamiast NULL
		}		
		catch(std::exception e) {
			cerr << "Lua runtime exception in Simulation::callHandler " << eventName << endl;
			cerr << e.what() << endl;
		}
	}	
}
