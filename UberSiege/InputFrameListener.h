#ifndef UBERS_INPUTFRAMELISTENER_H
#define UBERS_INPUTFRAMELISTENER_H

#include "Simulation.h"
#include "Ogre/Ogre.h" 
#include "OIS/OIS.h"
#include "CEGUI.h"
#include <map>

enum Action {
	P1_MOVE_LEFT,
	P1_MOVE_RIGHT,	
	P1_MOVE_UP,
	P1_MOVE_DOWN,
	P1_SELECT,
	P1_CLEAR,
	P1_CONFIRM,
	
	P2_MOVE_LEFT,
	P2_MOVE_RIGHT,	
	P2_MOVE_UP,
	P2_MOVE_DOWN,
	P2_SELECT,
	P2_CLEAR,
	P2_CONFIRM
};

typedef std::map<OIS::KeyCode, Action> KeyBindings;

// owns GUI (PuzzleBoardWidgets and others)
class InputFrameListener: public Ogre::FrameListener, public OIS::MouseListener, public OIS::KeyListener {
public:
	InputFrameListener(Application* app);
	~InputFrameListener();

	void bindKey(OIS::KeyCode code, Action a);
	void unbindKey(OIS::KeyCode code);

	// Ogre::FrameListener
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
	void initialize();
	void processAction(Action a);
	bool scanForLayouts(Player* p, std::vector<std::string>& layouts);
	void spawnUnits(Player* p, std::vector<std::string>& layouts);
	CEGUI::MouseButton convertButton(OIS::MouseButtonID id);

	OIS::InputManager* inputManager;
	OIS::Keyboard* keyboard;
	OIS::Mouse* mouse;
	Ogre::RenderWindow* window;
	Application* app;
	Simulation* simulation;
	Player* player1;
	Player* player2;
	PuzzleBoardWidget* boardWidget1;
	PuzzleBoardWidget* boardWidget2;

	std::vector<std::string> p1Layouts;
	std::vector<std::string> p2Layouts;

	KeyBindings bindings;
	bool isRunning;
};

#endif