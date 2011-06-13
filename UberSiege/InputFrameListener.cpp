#include "InputFrameListener.h"
#include "Application.h"
#include "PuzzleBoard.h"
#include "Simulation.h"
#include <iostream>

InputFrameListener::InputFrameListener(Application* app) {
	this->app = app;
	this->window = app->getRenderWindow();
	isRunning = true;

	// TODO: imiona graczy
	simulation = new Simulation(app);
	MapInfo info;
	app->loadMap("map1", info);
	simulation->loadMap(info);
	simulation->setPlayersCount(2);
	player1 = new Player("p1");
	player2 = new Player("p2");
	simulation->addPlayer(player1);
	simulation->addPlayer(player2);	

	// odczytaj uchwyt okna i utwórz obiekt obs³uguj¹cy urz¹dzenia wejœciowe
	OIS::ParamList params;
	unsigned int wndHandle = 0;
	std::ostringstream wndHandleString;
	window->getCustomAttribute("WINDOW", &wndHandle);
	wndHandleString << wndHandle;
	params.insert(std::make_pair("WINDOW", wndHandleString.str()));
	inputManager = OIS::InputManager::createInputSystem(params);

	keyboard = (OIS::Keyboard*) inputManager->createInputObject(OIS::OISKeyboard, true);
	mouse = (OIS::Mouse*) inputManager->createInputObject(OIS::OISMouse, true);

	mouse->setEventCallback(this);
	keyboard->setEventCallback(this);

	initialize();
	scanForLayouts(player1, boardWidget1);
	scanForLayouts(player2, boardWidget2);
}

InputFrameListener::~InputFrameListener() {
	inputManager->destroyInputObject(mouse);
	inputManager->destroyInputObject(keyboard);
	OIS::InputManager::destroyInputSystem(inputManager);	

	delete player1;
	delete player2;
	delete boardWidget1;
	delete boardWidget2;
	delete simulation;
}

void InputFrameListener::bindKey(OIS::KeyCode code, Action a) {
	bindings[code] = a;
}

void InputFrameListener::unbindKey(OIS::KeyCode code) {
	bindings.erase(code);
}

void InputFrameListener::scanForLayouts(Player* p, PuzzleBoardWidget* pbw) {
	std::vector<std::string> layouts = app->getLayoutFinder()->scan(p->getBoard());
	if(layouts.size() > 0)
		pbw->refreshAll();
}

void InputFrameListener::processAction(Action a) {	
	switch(a) {
		case P1_MOVE_LEFT:
			boardWidget1->moveLeft();
			break;
		case P1_MOVE_RIGHT:	
			boardWidget1->moveRight();
			break;
		case P1_MOVE_UP:
			boardWidget1->moveUp();
			break;
		case P1_MOVE_DOWN:
			boardWidget1->moveDown();
			break;
		case P1_SELECT:
		{
			boardWidget1->selectCurrent();
			scanForLayouts(player1, boardWidget1);
		}
			break;
		case P1_CLEAR:
		{
			boardWidget1->clear();
			scanForLayouts(player1, boardWidget1);
		}
			break;
		case P1_CONFIRM:
		{
			boardWidget1->accept();
			scanForLayouts(player1, boardWidget1);
		}
			break;	
		case P2_MOVE_LEFT:
			boardWidget2->moveLeft();
			break;
		case P2_MOVE_RIGHT:
			boardWidget2->moveRight();
			break;
		case P2_MOVE_UP:
			boardWidget2->moveUp();
			break;
		case P2_MOVE_DOWN:
			boardWidget2->moveDown();
			break;
		case P2_SELECT:
		{
			boardWidget2->selectCurrent();
			scanForLayouts(player2, boardWidget2);
		}
			break;
		case P2_CLEAR:
		{
			boardWidget2->clear();
			scanForLayouts(player2, boardWidget2);
		}
			break;
		case P2_CONFIRM:
		{
			boardWidget2->accept();
			scanForLayouts(player2, boardWidget2);
		}
			break;
		default:
			break;
	}
}

bool InputFrameListener::keyPressed(const OIS::KeyEvent& evt) {
	KeyBindings::iterator it = bindings.find(evt.key);

	// klawisze przypisane na sta³e
	switch(evt.key) {
		case OIS::KC_ESCAPE:
			isRunning = false;
			break;
		default:
			// czy wciœniêto klawisz przypisany jakiejœ akcji
			if(it != bindings.end())
				processAction(it->second);		
			break;
	}

	CEGUI::System& sys = CEGUI::System::getSingleton();
	sys.injectKeyDown(evt.key);
	sys.injectChar(evt.text);	
	return true;
}

bool InputFrameListener::keyReleased(const OIS::KeyEvent& evt) { 
	CEGUI::System::getSingleton().injectKeyUp(evt.key);
	return true;
}

bool InputFrameListener::mouseMoved(const OIS::MouseEvent& evt) { 
	CEGUI::System& sys = CEGUI::System::getSingleton();
	sys.injectMouseMove(evt.state.X.rel, evt.state.Y.rel);
	if(evt.state.Z.rel)
		sys.injectMouseWheelChange(evt.state.Z.rel / 120.0f);
	return true;
}

CEGUI::MouseButton InputFrameListener::convertButton(OIS::MouseButtonID id) {
	CEGUI::MouseButton button;
	switch(id) {
		case OIS::MB_Left:
			button = CEGUI::LeftButton;
			break;
		case OIS::MB_Right:
			button = CEGUI::RightButton;
			break;
		case OIS::MB_Middle:
			button = CEGUI::MiddleButton;
			break; 
		default:
			button = CEGUI::LeftButton;
    }
	return button;
}

bool InputFrameListener::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) {
	CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
	return true; 
}

bool InputFrameListener::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id) { 
	CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));
	return true; 
}

void InputFrameListener::initialize() {
	CEGUI::OgreRenderer* renderer = app->getGUIRenderer();

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

	player1->getBoard()->randomize();
	player2->getBoard()->randomize();

	boardWidget1 = new PuzzleBoardWidget(player1->getBoard());
	sheet->addChildWindow(boardWidget1->getWindow());
	boardWidget1->getWindow()->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0, 0), CEGUI::UDim(0.0, 0)));

	boardWidget2 = new PuzzleBoardWidget(player2->getBoard());
	sheet->addChildWindow(boardWidget2->getWindow());
	CEGUI::UDim posX = CEGUI::UDim(1.0, 0) - boardWidget2->getWindow()->getWidth();
	CEGUI::UDim posY = CEGUI::UDim(1.0, 0) - boardWidget2->getWindow()->getHeight();
	boardWidget2->getWindow()->setPosition(CEGUI::UVector2(posX, posY));
}

bool InputFrameListener::frameStarted(const Ogre::FrameEvent& evt) {
	keyboard->capture();	
	mouse->capture();	

	simulation->tick(evt.timeSinceLastFrame);

	Ogre::SceneNode* cameraNode = simulation->getCameraNode();
	if(keyboard->isKeyDown(OIS::KC_5)) {
		cameraNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(-0.1),Ogre::Node::TS_WORLD);
	}
	if(keyboard->isKeyDown(OIS::KC_6)) {
		cameraNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(0.1),Ogre::Node::TS_WORLD);
	}
	if(keyboard->isKeyDown(OIS::KC_1)) {
		cameraNode->translate(0, -0.1, 0);		
	}
	if(keyboard->isKeyDown(OIS::KC_2)) {
		cameraNode->translate(0, 0.1, 0);
	}
	if(keyboard->isKeyDown(OIS::KC_3)) {
		cameraNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(-0.01),Ogre::Node::TS_WORLD);
	}
	if(keyboard->isKeyDown(OIS::KC_4)) {
		cameraNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(0.01),Ogre::Node::TS_WORLD);
	}

	if(window->isClosed() || !isRunning)
		return false;
	return true;
}

bool InputFrameListener::frameEnded(const Ogre::FrameEvent& evt) {
	return true;
}

bool InputFrameListener::frameRenederingQueued(const Ogre::FrameEvent& evt) {
	return true;
}