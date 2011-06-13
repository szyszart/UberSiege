#ifndef UBERS_SIMULATION_H
#define UBERS_SIMULATION_H

#include "PuzzleBoard.h"
#include "Terrain.h"
#include "Ogre/Ogre.h"
#include "Mapinfo.h"

class Application;

struct UnitData {
	int hp;
	std::string className;
};

class Unit {
private:
	Ogre::Entity* entity;
	Ogre::AnimationState *anim;
	int hp;
};

// owns Board
class Player {
public:
	Player(std::string name);
	~Player();
	PuzzleBoard* getBoard() { return board; }
	std::string getName() { return name; }
private:
	PuzzleBoard* board;
	std::string name;
	unsigned int techLevel;
	int castleHP;
};

typedef int UnitHandle;

// owns Players, scene data (entities, animations, etc.), Camera
class Simulation {
public:
	Simulation(Application* app);
	~Simulation();
	void tick(float timeElapsed);
	void setPlayersCount(unsigned int numPlayers);
	unsigned int getPlayersCount();
	Player* getPlayer(unsigned int num);
	bool addPlayer(Player* player);
	bool loadMap(MapInfo& info);
	Ogre::SceneNode* getCameraNode() { return cameraNode; }
	//bool addUnit(Player* player, std::string name);

	// funkcje udostêpniane skryptom
	void moveForwards(Unit* unit);
	void moveBackwards(Unit* unit);
	UnitHandle enemyCollides(Unit* unit);
	void inflictDamage(Unit* unit, UnitHandle enemy, unsigned int damage);
	UnitData getUnitData(UnitHandle u);
	void requestAnimation(Unit* unit, std::string animName);

	Ogre::SceneNode* castleNode;
private:
	void createScene();
	//Ogre::Vector3 parsePoint(luabind::object o);

	Application* app;
	Ogre::Camera* camera;
	Ogre::SceneNode* cameraNode;
	Ogre::SceneManager* sceneManager;	
	
	std::vector<Player*> players;
	unsigned int maxNumPlayers;
	MapInfo info;

	// DEBUG, usuñ to
	TerrainLoader* loader;
	Ogre::AnimationState* animState;
	Ogre::AnimationState* animState2;
};

#endif