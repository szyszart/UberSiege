#ifndef UBERS_PUZZLEBOARD_H
#define UBERS_PUZZLEBOARD_H

#include "CEGUI.h"
#include <vector>
#include <string>

enum PuzzleBlock {
	BLOCK_EMPTY,
	BLOCK_WOOD,
	BLOCK_IRON,
	BLOCK_FIRE,
	BLOCK_MAGIC
};

class PuzzleBoard {
public:
	static const int DEFAULT_WIDTH	= 6;
	static const int DEFAULT_HEIGHT = 5;

	PuzzleBoard(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
	~PuzzleBoard();
	
	PuzzleBlock getAt(int x, int y);
	void setAt(PuzzleBlock b, int x, int y);
	void swap(int x1, int y1, int x2, int y2);
	void clear(PuzzleBlock b = BLOCK_EMPTY);
	void randomize(int nRows = -1);
	int getWidth() { return width; }
	int getHeight() { return height; }
	void shift(int nRows);
private:
	PuzzleBlock* grid;
	int width;
	int height;
};

struct WidgetBlock {
	std::string layout;
	int blockNumber;
};

class PuzzleBoardWidget {
public:
	PuzzleBoardWidget(PuzzleBoard* board);
	~PuzzleBoardWidget();

	CEGUI::Window* getWindow();
	PuzzleBoard* getBoard();
	void setBoard(PuzzleBoard* b);
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	void selectCurrent();
	void clear();	
	void accept();
	void refreshAll();

	WidgetBlock getAt(int x, int y);
	void setAt(WidgetBlock& b, int x, int y);
private:
	void createGUI();
	void refresh(int x, int y);
	void setNormalBorder(int x, int y);
	void refreshSelectionBorder();

	static const int blockImgSize = 64;
	static const int borderSize = 2;
	static const int blockSize = blockImgSize + 2 * borderSize;

	CEGUI::GridLayoutContainer* grid;
	PuzzleBoard* board;
	WidgetBlock* imageBlocks;

	int posX, posY;
	bool blockSelected;
	int selPosX, selPosY;
};

typedef std::pair<std::string, std::string> LayoutDescription;

class BoardLayoutFinder {
public:
	BoardLayoutFinder();
	bool addLayout(std::string name, LayoutDescription desc);
	bool removeLayout(std::string name);
	std::vector<std::string> scan(PuzzleBoardWidget* pbw);
private:
	bool matchLayout(PuzzleBoard* b, LayoutDescription desc, int startX, int startY);
	void clearLayout(PuzzleBoardWidget* widget, LayoutDescription desc, int startX, int startY, const std::string& name);
	PuzzleBlock charToBlock(char c);
	std::map<std::string, LayoutDescription> layouts;
};

#endif