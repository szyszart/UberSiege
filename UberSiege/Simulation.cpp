#include "Simulation.h"
#include "Application.h"

Player::Player(std::string name) {
	this->name = name;
	board = new PuzzleBoard();
}

Player::~Player() {
	delete board;
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

void Simulation::moveForwards(Unit* unit) {
}

void Simulation::moveBackwards(Unit* unit) {
}

UnitHandle Simulation::enemyCollides(Unit* unit) {
	return -1;
}

void Simulation::inflictDamage(Unit* unit, UnitHandle enemy, unsigned int damage) {
}

UnitData Simulation::getUnitData(UnitHandle u) {
	UnitData r;
	return r;
}

void Simulation::requestAnimation(Unit* unit, std::string animName) {
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
	for(std::vector<MountPoint>::iterator it = info.mountPoints.begin(); it != info.mountPoints.end(); ++it) {
		Ogre::Entity* ent = sceneManager->createEntity("ZamekMesh.mesh");		
		Ogre::SceneNode* node = sceneManager->getRootSceneNode()->createChildSceneNode();
		if(!castleNode)
			castleNode = node;
		node->attachObject(ent);
		node->setPosition(it->pos);
		node->setScale(it->scale);
		node->yaw(Ogre::Radian(it->rot));
	}

	Ogre::Vector3 path = info.pathEnd - info.pathStart;
	// DEBUG: dodaj ¿o³nierzyka po obu stronach

	Ogre::Vector3 pathNorm = path;
	pathNorm.normalise();
	Ogre::Radian angle = path.angleBetween(Ogre::Vector3::UNIT_Z);

	Ogre::Entity* ent = sceneManager->createEntity("archer.mesh");
	animState = ent->getAnimationState("Act: Walk_lower");
	animState->setEnabled(true);
	animState->setLoop(true);

	animState2 = ent->getAnimationState("Act: Walk_upper");
	animState2->setEnabled(true);
	animState2->setLoop(true);	
	Ogre::SceneNode* node = sceneManager->getRootSceneNode()->createChildSceneNode();
	node->attachObject(ent);
	node->setPosition(info.pathStart);	
	node->yaw(angle);
	castleNode = node;

	ent = sceneManager->createEntity("archer.mesh");
	std::cout << "Animation list" << std::endl;
	Ogre::AnimationStateSet* set = ent->getAllAnimationStates();
	Ogre::AnimationStateIterator iter = set->getAnimationStateIterator();
	while(iter.hasMoreElements())
		std::cout << iter.getNext()->getAnimationName() << std::endl;

	node = sceneManager->getRootSceneNode()->createChildSceneNode();
	node->attachObject(ent);
	node->setPosition(info.pathStart + path);	
	node->yaw(angle - Ogre::Degree(180.0));

	this->info = info;

	return true;
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

void Simulation::setPlayersCount(unsigned int n) {
	maxNumPlayers = n;
}

unsigned int Simulation::getPlayersCount() {
	return maxNumPlayers;
}

Player* Simulation::getPlayer(unsigned int num) {	
	return (num < maxNumPlayers) ? players[num] : NULL;
}

bool Simulation::addPlayer(Player* player) {	
	if(players.size() >= maxNumPlayers)
		return false;
	players.push_back(player);
	return true;
}

void Simulation::tick(float timeElapsed) {
	/*
		for each active unit u
			unitTick(u)

		for each player p
			if(p.unitQueue not empty && p.spawnArea is empty)
				u = p.unitQueue.dequeue()
				add u to active units
		

	*/

	// DIRTY: proof of concept
	static double position = 0.0;
	Ogre::Vector3 path = info.pathEnd - info.pathStart;	
	position += timeElapsed * 0.1;
	if(position > 1.0)
		position = 0.0;
	castleNode->setPosition(info.pathStart + position * path);
	animState->addTime(timeElapsed);
	animState2->addTime(timeElapsed);
}
