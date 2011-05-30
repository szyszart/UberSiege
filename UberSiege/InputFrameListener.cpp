#include "InputFrameListener.h"
#include "Application.h"
#include "PuzzleBoard.h"
#include <iostream>

bool InputFrameListener::keyPressed(const OIS::KeyEvent& evt) {
	switch(evt.key) {
		// pierwszy gracz
		case OIS::KC_W:
			app->getBoardWidget()->moveUp();
			break;
		case OIS::KC_S:
			app->getBoardWidget()->moveDown();
			break;
		case OIS::KC_A:
			app->getBoardWidget()->moveLeft();
			break;
		case OIS::KC_D:
			app->getBoardWidget()->moveRight();
			break;
		case OIS::KC_Q:
		{
			app->getBoardWidget()->selectCurrent();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard());
			if(layouts.size() > 0)
				app->getBoardWidget()->refreshAll();
		}
			break;
		case OIS::KC_E:
		{
			app->getBoardWidget()->accept();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard());
			if(layouts.size() > 0)
				app->getBoardWidget()->refreshAll();
		}
			break;
		case OIS::KC_Z:
		{
			app->getBoardWidget()->clear();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard());
			if(layouts.size() > 0)
				app->getBoardWidget()->refreshAll();			
		}
			break;
		// drugi gracz
		case OIS::KC_I:
			app->getBoardWidget2()->moveUp();
			break;
		case OIS::KC_K:
			app->getBoardWidget2()->moveDown();
			break;
		case OIS::KC_J:
			app->getBoardWidget2()->moveLeft();
			break;
		case OIS::KC_L:
			app->getBoardWidget2()->moveRight();
			break;
		case OIS::KC_U:
		{
			app->getBoardWidget2()->selectCurrent();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard2());
			if(layouts.size() > 0)
				app->getBoardWidget2()->refreshAll();
		}
			break;
		case OIS::KC_O:
		{
			app->getBoardWidget2()->accept();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard2());
			if(layouts.size() > 0)
				app->getBoardWidget2()->refreshAll();
		}
			break;
		case OIS::KC_M:
		{
			app->getBoardWidget2()->clear();
			std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard2());
			if(layouts.size() > 0)
				app->getBoardWidget2()->refreshAll();
		}
			break;
		// pozosta³e
		case OIS::KC_ESCAPE:
			isRunning = false;
			break;
		default:
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

InputFrameListener::InputFrameListener(Application* a, Ogre::RenderWindow* window) {
	app = a;
	isRunning = true;

	this->window = window;
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

	// odœwie¿ pierwsz¹ tablicê klocków
	std::vector<std::string> layouts = app->getLayoutFinder()->scan(app->getBoard());
	if(layouts.size() > 0)
		app->getBoardWidget()->refreshAll();

	layouts = app->getLayoutFinder()->scan(app->getBoard2());
	if(layouts.size() > 0)
		app->getBoardWidget2()->refreshAll();
}

InputFrameListener::~InputFrameListener() {
	inputManager->destroyInputObject(mouse);
	inputManager->destroyInputObject(keyboard);
	OIS::InputManager::destroyInputSystem(inputManager);	
}

bool InputFrameListener::frameStarted(const Ogre::FrameEvent& evt) {
	keyboard->capture();	
	mouse->capture();		

	if(keyboard->isKeyDown(OIS::KC_C))
		app->getCameraNode()->translate(0, 0, -10);
	if(keyboard->isKeyDown(OIS::KC_V))
		app->getCameraNode()->translate(0, 0, 10);
	if(keyboard->isKeyDown(OIS::KC_B))
		app->getCameraNode()->translate(0, -10, 0);
	if(keyboard->isKeyDown(OIS::KC_N))
		app->getCameraNode()->translate(0, 10, 0);
	if(keyboard->isKeyDown(OIS::KC_1))
		app->getCameraNode()->translate(-10, 0, 0);
	if(keyboard->isKeyDown(OIS::KC_2))
		app->getCameraNode()->translate(10, 0, 0);
	if(keyboard->isKeyDown(OIS::KC_G))
		app->getCameraNode()->yaw(Ogre::Radian(-0.01));
	if(keyboard->isKeyDown(OIS::KC_H))
		app->getCameraNode()->yaw(Ogre::Radian(0.01));
	if(keyboard->isKeyDown(OIS::KC_3))
		std::cout << app->getCameraNode()->getPosition().x << ", " << app->getCameraNode()->getPosition().y << ", " <<  app->getCameraNode()->getPosition().z << "(" <<  app->getCameraNode()->getOrientation().getYaw() << ")" << std::endl;


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