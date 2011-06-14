#ifndef UBERS_SIMULATION_H
#define UBERS_SIMULATION_H

#include "PuzzleBoard.h"
#include "Terrain.h"
#include "Ogre/Ogre.h"
#include "Mapinfo.h"

#include <map>
#include <list>

#include "lua.hpp"
#include <luabind/luabind.hpp>

class Application;

// przedzia³ na œcie¿ce
struct Zone {
	double start;
	double end;
};

class Player;

class Unit {
public:
	Unit(Player* p, std::string className, int initialHP) {
		this->className = className;
		hp = initialHP;
		player = p;
		sceneNode = NULL;
		pos = vel = 0.0;
		direction = 1.0;
		currentQueuedAnim = NULL;
		active = true;
	}
	inline double const getPos() { return pos; }
	inline void setPos(double p) { pos = p; }

	inline double const getDirection() { return direction; }
	inline void setDirection(double d) { direction = d; }

	inline double getVel() { return vel; }
	inline void setVel(double v) { vel = v; }

	inline std::string getClassName() { return className; }
	inline bool isDead() { return (hp <= 0); }

	inline int getHP() { return hp; }
	inline void setHP(int newHP) { hp = newHP; }

	inline double getWidth() { return width; }
	inline void setWidth(double newWidth) { width = newWidth; }

	inline Player* getPlayer() { return player; }
	
	inline Ogre::SceneNode* getSceneNode() { return sceneNode; }
	inline void setSceneNode(Ogre::SceneNode* node) { sceneNode = node; }

	inline Ogre::Entity* getEntity() { return entity; }
	inline void setEntity(Ogre::Entity* e) { entity = e; }

	inline void setActive(bool act) { active = act; }
	inline bool isActive() { return active; }

	bool addAnimation(std::string name);
	void clearAnimations();
	void advanceAnimations(float timeElapsed) {
		if(anims.empty()) {
			// odtwórz kolejn¹ animacjê z kolejki			
			if(currentQueuedAnim && currentQueuedAnim->hasEnded()) {
				currentQueuedAnim->setTimePosition(0.0);
				currentQueuedAnim = NULL;
			}
			if(!currentQueuedAnim) {
				while(!animQueue.empty() && !currentQueuedAnim) {
					currentQueuedAnim = fetchAnimation(animQueue.front());
					std::cout << "popping queued anim " << animQueue.front() << std::endl;
					animQueue.pop_front();
				}
			}			
			if(currentQueuedAnim)
				currentQueuedAnim->addTime(timeElapsed);
		}
		else {
			std::map<std::string, Ogre::AnimationState*>::iterator it = anims.begin();
			while(it != anims.end()) {
				Ogre::AnimationState* anim = it->second;
				anim->addTime(timeElapsed);
				if(anim->hasEnded())  {
					anim->setTimePosition(0.0);
					anims.erase(it++);
				}
				else 
					++it;		
			}
		}
	}
	bool hasQueuedAnimations() { return (!animQueue.empty() || currentQueuedAnim); }	
	void enqueueAnimation(std::string name) { animQueue.push_back(name); }	
private:
	Ogre::AnimationState* fetchAnimation(std::string name);

	std::string className;
	int hp;
	double pos;
	double vel;
	double width;	// absolute (not relative to the path)
	double yaw;
	double direction;
	bool active;
	Player* player;

	Ogre::Entity* entity;
	Ogre::SceneNode* sceneNode;
	std::map<std::string, Ogre::AnimationState*> anims;
	Ogre::AnimationState* currentQueuedAnim;
	std::list<std::string> animQueue;
};

// owns Board
class Player {
public:
	Player(std::string name);
	~Player();
	PuzzleBoard* getBoard() { return board; }
	std::string getName() { return name; }
	Zone getSpawnZone() { return spawnZone; }
	void setSpawnZone(Zone z) { spawnZone = z; }
	void setYaw(double y) { yaw = y; }
	double getYaw() { return yaw; }
	double const getDirection() { return direction; }
	void setDirection(double d) { direction = d; }

	bool hasWaitingUnits() { return !unitQueue.empty(); }
	Unit* dequeueUnit() { 
		Unit* result = unitQueue.front();
		unitQueue.pop_front();
		return result;
	}
	void enqueueUnit(Unit* u) {
		unitQueue.push_back(u);
	}
	Unit* peekUnit() {
		return (unitQueue.empty() ? NULL : unitQueue.front());
	}
private:
	PuzzleBoard* board;
	std::string name;
	unsigned int techLevel;
	int castleHP;
	std::list<Unit*> unitQueue;
	Zone spawnZone;
	double yaw;	// k¹t obrotu jednostek gracza
	double direction; // kierunek marszu jednostek gracza

};

// owns Players, scene data (entities, animations, etc.), Camera
class Simulation {
public:
	Simulation(Application* app);
	~Simulation();
	void tick(float timeElapsed);
	Player* getPlayer(unsigned int num);
	bool addPlayer(Player* player);
	void addUnit(Player* player, std::string className);
	bool loadMap(MapInfo& info);
	Ogre::SceneNode* getCameraNode() { return cameraNode; }

	// funkcje udostêpniane skryptom
	void moveForwards(Unit* unit, double vel = 1.0);
	void moveBackwards(Unit* unit, double vel = 1.0);
	Unit* getNearestEnemy(Unit* unit);
	luabind::object getEnemiesLua(lua_State* lua, Unit* unit);
	void inflictDamage(Unit* unit, Unit* enemy, unsigned int damage);	
	bool requestAnimation(Unit* unit, std::string animName);
	void queueAnimation(Unit* unit, std::string animName);
	void stopAnimations(Unit* unit);
	double getDistance(Unit* u1, Unit* u2);

	std::list<Unit*> getEnemies(Unit* unit);

	Ogre::SceneNode* castleNode;
private:	
	bool addUnit(Unit* unit);
	void removeUnit(std::list<Unit*>::iterator);
	void createScene();
	bool isZoneEmpty(Zone z);
	bool collisions(Unit* unit);	// true, jeœli jednostka z czymœ koliduje
	bool inBounds(Unit* unit);		// true, jeœli jednostka mieœci siê na œcie¿ce
	bool isInSpawnZone(Unit* unit);
	bool collide(Unit* u1, Unit *u2);

	void callHandler(Unit* u, std::string eventName);

	static const double SPAWN_ZONE_WIDTH;

	Application* app;
	Ogre::Camera* camera;
	Ogre::SceneNode* cameraNode;
	Ogre::SceneManager* sceneManager;	
	TerrainLoader* loader;
	
	std::vector<Player*> players;
	std::list<Unit*> units;
	MapInfo info;

	Ogre::Vector3 path;
};

#endif