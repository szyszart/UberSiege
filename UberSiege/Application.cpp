#include "Application.h"
#include "PuzzleBoard.h"

#include <iostream>
#include <cstdlib>

Application::Application(): root(NULL), sceneManager(NULL), listener(NULL), renderer(NULL) {
	running = false;
	board = new PuzzleBoard();
	board2 = new PuzzleBoard();
	boardWidget = boardWidget2 = NULL;
	layoutFinder = new BoardLayoutFinder();

	// TODO: odczytywanie z pliku konfiguracyjnego
	layoutFinder->addLayout("simple", std::pair<std::string, std::string>("ii", ""));
	layoutFinder->addLayout("dragon", std::pair<std::string, std::string>("fmf", "fff"));
}

Application::~Application() {
	if(renderer) 
		CEGUI::OgreRenderer::destroySystem();
	
	if(boardWidget)
		delete boardWidget;
	delete board;

	if(boardWidget2)
		delete boardWidget2;
	delete board2;

	delete layoutFinder;

	delete listener;
	
	delete mTerrainGlobals;
	delete mTerrainGroup;

	delete root;
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

bool Application::initGUI() {
	renderer = &CEGUI::OgreRenderer::bootstrapSystem();

	CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
	CEGUI::System::getSingleton().setDefaultMouseCursor("WindowsLook", "MouseArrow");
	CEGUI::Window* sheet = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
	CEGUI::System::getSingleton().setGUISheet(sheet);

	CEGUI::ImagesetManager::getSingleton().create("blocks.imageset");
	board->randomize();
	board2->randomize();

	boardWidget = new PuzzleBoardWidget(board);
	sheet->addChildWindow(boardWidget->getWindow());
	boardWidget->getWindow()->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0, 0), CEGUI::UDim(0.0, 0)));

	boardWidget2 = new PuzzleBoardWidget(board2);
	sheet->addChildWindow(boardWidget2->getWindow());
	CEGUI::UDim posX = CEGUI::UDim(1.0, 0) - boardWidget2->getWindow()->getWidth();
	CEGUI::UDim posY = CEGUI::UDim(1.0, 0) - boardWidget2->getWindow()->getHeight();
	boardWidget2->getWindow()->setPosition(CEGUI::UVector2(posX, posY));

	return true;
}

bool Application::startup() {
	if(!root)
		root = new Ogre::Root("plugins_d.cfg");
	if(!root->showConfigDialog())
		return false;
	Ogre::RenderWindow* window = root->initialise(true, "Uber Siege");
	sceneManager = root->createSceneManager(Ogre::ST_GENERIC);

	camera = sceneManager->createCamera("Camera");
	camera->setPosition(Ogre::Vector3(0, 0, 500));
	camera->lookAt(Ogre::Vector3(0, 0, 0));
	camera->setNearClipDistance(0.1);
	camera->setFarClipDistance(50000);

	Ogre::Viewport* viewport = window->addViewport(camera);
	viewport->setBackgroundColour(Ogre::ColourValue(0.0, 0.0, 0.0));
	camera->setAspectRatio(Ogre::Real(viewport->getActualWidth() / viewport->getActualHeight()));

	loadResources();
	initGUI();

	createScene();

	listener = new InputFrameListener(this, window);
	root->addFrameListener(listener);

	return true;
}

void Application::getTerrainImage(bool flipX, bool flipY, Ogre::Image& img) {
    img.load("terrain1_heightmap513.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if(flipX)
		img.flipAroundY();
    if(flipY)
        img.flipAroundX();
}

void Application::defineTerrain(long x, long y) {
	Ogre::String filename = mTerrainGroup->generateFilename(x, y);
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
		mTerrainGroup->defineTerrain(x, y);
	else {
		Ogre::Image img;
		getTerrainImage(x % 2 != 0, y % 2 != 0, img);
		mTerrainGroup->defineTerrain(x, y, &img);
		mTerrainsImported = true;
	}
}

void Application::configureTerrainDefaults(Ogre::Light* light) {
	// Configure global
	mTerrainGlobals->setMaxPixelError(8);
	// testing composite map
	mTerrainGlobals->setCompositeMapDistance(3000);
 
	// Important to set these so that the terrain knows what to use for derived (non-realtime) data
	mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(sceneManager->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
 
	// Configure default import settings for if we use imported image
	Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
	defaultimp.terrainSize = 513;
	defaultimp.worldSize = 10000.0f;
	defaultimp.inputScale = 300;
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

void Application::initBlendMaps(Ogre::Terrain* terrain) {
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

void Application::createScene() {
	cameraNode = sceneManager->getRootSceneNode()->createChildSceneNode();
	//cameraNode->setPosition(0, 1610, 7760);
	cameraNode->setPosition(160, 580, -440);
	cameraNode->yaw(Ogre::Radian(-0.36));
	cameraNode->attachObject(camera);

	Ogre::Entity* ent = sceneManager->createEntity("Icosphere.mesh");
	Ogre::SceneNode* unitNode = sceneManager->getRootSceneNode()->createChildSceneNode();
	unitNode->attachObject(ent);
	unitNode->setPosition(-5, 580, 0);	
	unitNode->yaw(Ogre::Radian(2.3));
	cameraNode = unitNode;

	mTerrainGlobals = new Ogre::TerrainGlobalOptions(); 
	mTerrainGroup = new Ogre::TerrainGroup(sceneManager, Ogre::Terrain::ALIGN_X_Z, 513, 10000.0f);
	mTerrainGroup->setFilenameConvention(Ogre::String("UberSiegeTerrain"), Ogre::String("dat"));
	mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

	Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
	lightdir.normalise();
 
	Ogre::Light* light = sceneManager->createLight("tstLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
 
	sceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	configureTerrainDefaults(light);

	for(long x = 0; x <= 0; ++x)
		for(long y = 0; y <= 0; ++y)
			defineTerrain(x, y);
 
	mTerrainGroup->loadAllTerrains(true);
	if(mTerrainsImported) {
		Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
		while(ti.hasMoreElements()) {
			Ogre::Terrain* t = ti.getNext()->instance;
			initBlendMaps(t);
		}
	}

	mTerrainGroup->freeTemporaryResources();
	sceneManager->setSkyDome(true, "CloudySky", 5, 8);
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