#include "Simulation.h"
#include "Application.h"
using namespace std;

const double Simulation::SPAWN_ZONE_WIDTH = 0.1;

Player::Player(string name, int initialHP) {
	this->name = name;
	hp = initialHP;
	board = new PuzzleBoard();
}

Player::~Player() {
	delete board;
}

Ogre::AnimationState* Unit::fetchAnimation(string name) {
	map<string, Ogre::AnimationState*>::iterator it = anims.find(name);

	Ogre::AnimationState* anim = NULL;
	if(it == anims.end()) {		
		anim = entity->getAnimationState(name);
		if(!anim)
			return NULL;
	}
	return anim;
}

bool Unit::addAnimation(string name) {
	Ogre::AnimationState* anim = fetchAnimation(name);
	if(!anim)
		return false;
	anim->setEnabled(true);
	anim->setLoop(false);
	anims[name] = anim;
	return true;
}

void Unit::clearAnimations() {
	std::map<std::string, Ogre::AnimationState*>::iterator it = anims.begin();
	while(it != anims.end()) {
		it->second->setEnabled(false);
		++it;
	}
	anims.clear();
}

void Unit::clearQueuedAnimations() {
	animQueue.clear(); 
	if(currentQueuedAnim)
		currentQueuedAnim->setEnabled(false);
	currentQueuedAnim = NULL;
}

Simulation::Simulation(Application* app) {
	this->app = app;
	sceneManager = app->getSceneManager();
	camera = app->getCamera();
	loader = new TerrainLoader(sceneManager);
	loser = NULL;
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

void Simulation::throwProjectile(Unit* unit, double velX, double velY, int damage) {
	Projectile p;
	double dir = unit->getPlayer()->getDirection();
	p.vel = dir * Ogre::Vector2(velX, velY);
	p.pos = Ogre::Vector2(unit->getPos(), unit->getEntity()->getBoundingRadius() / 2);
	p.damage = damage;
	p.entity = sceneManager->createEntity("Arrow.mesh");
	Ogre::SceneNode* node = sceneManager->getRootSceneNode()->createChildSceneNode();
	node->attachObject(p.entity);
	p.node = node;
	p.unit = unit;
	projectiles.push_back(p);
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
	if(!enemy->isDead()) {
		enemy->setHP(enemy->getHP() - damage);
	}
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

void Simulation::stopQueuedAnimations(Unit* unit) {
	unit->clearQueuedAnimations();
}

bool Simulation::loadMap(MapInfo& info) {
	createScene();

	// TODO: przenie�� do pliku konfiguracyjnego
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
	Unit* unit = new Unit(player, className, 100);

	unit->setDirection(player->getDirection());
	Ogre::Entity* ent = sceneManager->createEntity(className + ".mesh");
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

bool Simulation::collide(Unit* u1, Unit* u2) {
	double pos1 = (path * u1->getPos()).length();
	double pos2 = (path * u2->getPos()).length();
	double w1 = u1->getWidth();
	double w2 = u2->getWidth();
	return (((pos2 - w2 >= pos1 - w1) && (pos2 - w2 <= pos1 + w1))
		|| ((pos2 + w2 >= pos1 - w1) && (pos2 + w2 <= pos1 + w1)));
}

bool Simulation::collide(Unit* u, Projectile p) {
	double posU = (path * u->getPos()).length();
	double wU = u->getWidth();

	double posP = (path * p.pos.x).length();
	double wP = 10.0;
	return (((posP - wP >= posU - wU) && (posP - wP <= posU + wU))
		|| ((posP + wP >= posU - wU) && (posP + wP <= posU + wU)));
}

bool Simulation::inBounds(Unit* unit) {
	double pos = unit->getPos();
	return (pos >= 0.0 && pos <= 1.0);
}

bool Simulation::isZoneEmpty(Zone z) {
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
				// pojazd wroga dotar� do zamku, uaktualnij HP
				Player* p = u->getPlayer();
				p->setHP(p->getHP() - u->getHP());
				u->setActive(false);
			}		
			else {
				// oblicz nowe po�o�enie jednostki
				double oldPos = u->getPos();
				double newPos = oldPos + u->getVel() * timeElapsed;			
				u->setPos(newPos);
				// sprawd�, czy wyst�pi kolizja
				if(!collisions(u) && inBounds(u)) {
					Ogre::SceneNode* node = u->getSceneNode();
					if(node) {				
						node->setPosition(info.pathStart + path * u->getPos());						
					}
				}
				else u->setPos(oldPos);
				u->advanceAnimations(timeElapsed);
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
			luabind::call_function<void>(callback, this, u, NULL);
		}		
		catch(std::exception e) {
			cerr << "Lua runtime exception in Simulation::callHandler " << eventName << endl;
			cerr << e.what() << endl;
		}
	}	
}

bool Simulation::hasEnded() {
	std::vector<Player*>::iterator it = players.begin();
	while(it != players.end()) {
		Player* p = *it;
		if(p->getHP() <= 0) {
			loser = p;
			return true;
		}
		++it;
	}
}