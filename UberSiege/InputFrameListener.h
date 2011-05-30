#ifndef UBERS_INPUTFRAMELISTENER_H
#define UBERS_INPUTFRAMELISTENER_H

#include "Ogre/Ogre.h" 
#include "OIS/OIS.h"
#include "CEGUI.h"

class Application;

class InputFrameListener: public Ogre::FrameListener, public OIS::MouseListener, public OIS::KeyListener {
public:
	InputFrameListener(Application* app, Ogre::RenderWindow* window);
	~InputFrameListener();
	bool frameStarted(const Ogre::FrameEvent& evt);
	bool frameEnded(const Ogre::FrameEvent& evt);
	bool frameRenederingQueued(const Ogre::FrameEvent& evt);

	// OIS::KeyListener
    bool keyPressed(const OIS::KeyEvent& evt);
    bool keyReleased(const OIS::KeyEvent& evt);
    // OIS::MouseListener
    bool mouseMoved(const OIS::MouseEvent& evt);
    bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
    bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
private:
	CEGUI::MouseButton convertButton(OIS::MouseButtonID id);

	OIS::InputManager* inputManager;
	OIS::Keyboard* keyboard;
	OIS::Mouse* mouse;
	Ogre::RenderWindow* window;
	Application* app;
	bool isRunning;
};

#endif