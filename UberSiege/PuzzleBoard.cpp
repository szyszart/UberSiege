#include "PuzzleBoard.h"
#include <algorithm>
#include <iostream>

PuzzleBoard::PuzzleBoard(int width, int height) {
	this->width = width;
	this->height = height;
	grid = new PuzzleBlock[width * height];
	clear();
}

PuzzleBoard::~PuzzleBoard() {
	delete[] grid;
}

PuzzleBlock PuzzleBoard::getAt(int x, int y) {
	return grid[width * y + x];
}

void PuzzleBoard::setAt(PuzzleBlock b, int x, int y) {
	grid[width * y + x] = b;
}

void PuzzleBoard::swap(int x1, int y1, int x2, int y2) {
	PuzzleBlock t = getAt(x1, y1);
	setAt(getAt(x2, y2), x1, y1);
	setAt(t, x2, y2);
}

void PuzzleBoard::clear(PuzzleBlock b) {
	for(int i = 0; i < width * height; i++)
	  grid[i] = b;
}

void PuzzleBoard::randomize(int nRows) {	
	for(int y = 0; y < ((nRows < 0) ? height : nRows); y++) {
		for(int x = 0; x < width; x++) {
			int block = rand() % 4;
			switch(block) {
			case 0:
				setAt(BLOCK_FIRE, x, y);
				break;
			case 1:
				setAt(BLOCK_IRON, x, y);
				break;
			case 2:
				setAt(BLOCK_MAGIC, x, y);
				break;
			case 3:
				setAt(BLOCK_WOOD, x, y);
				break;
			default:
				setAt(BLOCK_EMPTY, x, y);
				break;
			};
		}
	}
}

void PuzzleBoard::shift(int nRows) {
	if(nRows <= 0)
		return;
	if(nRows >= height)
		randomize();
	else {
		int numSwapped = height - nRows;
		for(int y = 0; y < numSwapped; y++) {
			for(int x = 0; x < width; x++) {
				setAt(getAt(x, numSwapped - 1 - y), x, height - y - 1);
			}
		}
		randomize(nRows);
	}
}

PuzzleBoardWidget::PuzzleBoardWidget(PuzzleBoard* b) {
	board = b;
	posX = posY = selPosX = selPosY = 0;
	blockSelected = false;
	grid = NULL;
	imageBlocks = new WidgetBlock[board->getWidth() * board->getHeight()];
}

PuzzleBoardWidget::~PuzzleBoardWidget() {
	delete[] imageBlocks;
}

WidgetBlock PuzzleBoardWidget::getAt(int x, int y) {
	return imageBlocks[board->getWidth() * y + x];
}

void PuzzleBoardWidget::setAt(WidgetBlock& b, int x, int y) {
	imageBlocks[board->getWidth() * y + x] = b;
}

void PuzzleBoardWidget::createGUI() {
	grid = (CEGUI::GridLayoutContainer*) CEGUI::WindowManager::getSingleton().createWindow("GridLayoutContainer");
	int width = board->getWidth();
	int height = board->getHeight();

	grid->setGridDimensions(width, height);
	grid->setSize(CEGUI::UVector2(CEGUI::UDim(0.0, blockSize * width), CEGUI::UDim(0.0, blockSize * height)));
	grid->setAutoPositioning(CEGUI::GridLayoutContainer::AutoPositioning::AP_LeftToRight);

	CEGUI::Window* btnWindow;
	CEGUI::Window* staticWindow;
	for(int y = 0; y < board->getHeight(); y++) {
		for(int x = 0; x < board->getWidth(); x++) {
			staticWindow = CEGUI::WindowManager::getSingleton().createWindow("WindowsLook/Static");
			grid->addChildWindow(staticWindow);

			staticWindow->setSize(CEGUI::UVector2(CEGUI::UDim(0.0, blockSize), CEGUI::UDim(0.0, blockSize)));
			staticWindow->setProperty("FrameEnabled", "False");
			staticWindow->setProperty("BackgroundEnabled", "True");

			btnWindow = CEGUI::WindowManager::getSingleton().createWindow("WindowsLook/StaticImage");
			staticWindow->addChildWindow(btnWindow);
		
			btnWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0, borderSize), CEGUI::UDim(0.0, borderSize)));
			btnWindow->setSize(CEGUI::UVector2(CEGUI::UDim(0.0, blockImgSize),CEGUI::UDim(0.0, blockImgSize)));
		}
	}
	refreshAll();
	grid->layout();
}

CEGUI::Window* PuzzleBoardWidget::getWindow() {
	if(!grid)
		createGUI();
	return grid;
}

PuzzleBoard* PuzzleBoardWidget::getBoard() {
	return board;
}

void PuzzleBoardWidget::setNormalBorder(int x, int y) {
	if(blockSelected && (selPosX == x) && (selPosY == y))
		grid->getChildWindowAtPosition(x, y)->setProperty("BackgroundColours", "tl:FF0000FF tr:FF0000AA bl:FF0000FF br:FF0000CC");
	else
		grid->getChildWindowAtPosition(x, y)->setProperty("BackgroundColours", "tl:00000000 tr:00000000 bl:00000000 br:00000000");
}

void PuzzleBoardWidget::refreshSelectionBorder() {
	grid->getChildWindowAtPosition(posX, posY)->setProperty("BackgroundColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
}

void PuzzleBoardWidget::setBoard(PuzzleBoard* b) {
	board = b;
	refreshAll();
}

void PuzzleBoardWidget::moveLeft() {
	if(posX > 0) {
		setNormalBorder(posX, posY);
		posX--;
		refreshSelectionBorder();
	}
}

void PuzzleBoardWidget::moveRight() {
	if(posX < board->getWidth() - 1) {
		setNormalBorder(posX, posY);
		posX++;
		refreshSelectionBorder();
	}
}

void PuzzleBoardWidget::moveUp() {
	if(posY > 0) {
		setNormalBorder(posX, posY);
		posY--;
		refreshSelectionBorder();
	}
}

void PuzzleBoardWidget::moveDown() {
	if(posY < board->getHeight() - 1) {
		setNormalBorder(posX, posY);
		posY++;
		refreshSelectionBorder();
	}
}

void PuzzleBoardWidget::selectCurrent() {
	if(board->getAt(posX, posY) == BLOCK_EMPTY)
		return;

	if(blockSelected) {
		board->swap(posX, posY, selPosX, selPosY);
		blockSelected = false;
		refresh(posX, posY);
		refresh(selPosX, selPosY);
	}
	else {
		blockSelected = true;
		selPosX = posX;
		selPosY = posY;
		setNormalBorder(posX, posY);
	}
}

void PuzzleBoardWidget::accept() {
	if(!blockSelected) {
		board->shift(2);
		refreshAll();
	}
}

void PuzzleBoardWidget::clear() {
	blockSelected = false;
	board->randomize();
	refreshAll();
}

void PuzzleBoardWidget::refresh(int x, int y) {
	CEGUI::Window* staticWindow = grid->getChildWindowAtPosition(x, y);		
	setNormalBorder(x, y);
	refreshSelectionBorder();

	if(staticWindow->getChildCount() == 0)
		return;
	CEGUI::Window* btnWindow = staticWindow->getChildAtIdx(0);	
	switch(board->getAt(x, y)) {
		case BLOCK_WOOD:
			btnWindow->setProperty("Image", "set:Blocks image:wood");
			break;
		case BLOCK_IRON:
			btnWindow->setProperty("Image", "set:Blocks image:iron");
			break;
		case BLOCK_FIRE:
			btnWindow->setProperty("Image", "set:Blocks image:fire");
			break;
		case BLOCK_MAGIC:
			btnWindow->setProperty("Image", "set:Blocks image:magic");
			break;
		default:
		{
			WidgetBlock b = getAt(x, y);
			if(b.layout.empty())
				btnWindow->setProperty("Image", "set:Blocks image:empty");
			else {
				std::stringstream imageDesc;
				imageDesc << "set: u_icons image: " << b.layout << b.blockNumber;
				btnWindow->setProperty("Image", imageDesc.str());
			}
		}			
			break;
	}
}

void PuzzleBoardWidget::refreshAll() {	
	for(int y = 0; y < board->getHeight(); y++) {
		for(int x = 0; x < board->getWidth(); x++) {
			refresh(x, y);
		}
	}
}

BoardLayoutFinder::BoardLayoutFinder() {
}

bool BoardLayoutFinder::addLayout(std::string name, LayoutDescription desc) {
	if(desc.first.empty())	// odrzucamy puste uk³ady klocków
		return false;
	// dopuszczamy wszelkie niepuste uk³ady prostok¹tne
	if(desc.second.empty() || (desc.first.length() == desc.second.length())) {	
		layouts[name] = desc;
		return true;
	}
	return false;
}

bool BoardLayoutFinder::removeLayout(std::string name) {	
	return (layouts.erase(name) == 1);
}

std::vector<std::string> BoardLayoutFinder::scan(PuzzleBoardWidget* widget) {
	using namespace std;
	vector<string> result;
	PuzzleBoard* board = widget->getBoard();
	if(board->getHeight() < 1) 
		return result;

	map<string, LayoutDescription>::iterator iter;	
	int numFreeBlocks = 2 * board->getWidth();	// liczba pól, na których mog¹ byæ kolejne uk³ady
	for(iter = layouts.begin(); iter != layouts.end(); iter++) {
		LayoutDescription cur = iter->second;
		int layoutWidth = cur.first.length();	// zak³adamy, ¿e wszystkie uk³ady s¹ prostok¹tne; dba o to metoda addLayout		
		int layoutHeight = (cur.second.empty() ? 1 : 2);

		if(layoutWidth > board->getWidth())
			continue;

		for(int y = ((layoutHeight == 1) ? 0 : 1); y >= layoutHeight - 1; y--) {
			for(int x = 0; x <= board->getWidth() - layoutWidth; x += 2) {
				if(matchLayout(board, cur, x, board->getHeight() - 1 - y)) {	
					result.push_back(iter->first);
					clearLayout(widget, cur, x, board->getHeight() - 1 - y, iter->first);
					x += layoutWidth;
					numFreeBlocks -= layoutWidth * layoutHeight;
					if(numFreeBlocks <= 0)
						return result;
				}
			}
		}
	}
	return result;
}

bool BoardLayoutFinder::matchLayout(PuzzleBoard* board, LayoutDescription desc, int startX, int startY) {
	int layoutWidth = desc.first.length();	// zak³adamy, ¿e wszystkie uk³ady s¹ prostok¹tne; dba o to metoda addLayout
	int layoutHeight = (desc.second.empty() ? 1 : 2);
	std::string pattern = desc.first + desc.second;

	int i = 0;
	for(int y = startY; y < startY + layoutHeight; y++)
		for(int x = startX; x < startX + layoutWidth; x++) {
			if(board->getAt(x, y) != charToBlock(pattern[i++]))
				return false;
		}
	std::cout << "Got a match at: " << startX << ", " << startY << std::endl;
	return true;
}

void BoardLayoutFinder::clearLayout(PuzzleBoardWidget* widget, LayoutDescription desc, int startX, int startY, const std::string& name) {
	PuzzleBoard* board = widget->getBoard();
	int layoutWidth = desc.first.length();	// zak³adamy, ¿e wszystkie uk³ady s¹ prostok¹tne; dba o to metoda addLayout
	int layoutHeight = (desc.second.empty() ? 1 : 2);

	int i = 1;
	for(int y = startY; y < startY + layoutHeight; y++)
		for(int x = startX; x < startX + layoutWidth; x++) {
			board->setAt(BLOCK_EMPTY, x, y);
			WidgetBlock w;
			w.blockNumber = i++;
			w.layout = name;
			widget->setAt(w, x, y);
		}
}

PuzzleBlock BoardLayoutFinder::charToBlock(char c) {
	switch(c) {
		case 'W':
		case 'w':
			return BLOCK_WOOD;
		case 'I':
		case 'i':
			return BLOCK_IRON;
		case 'M':
		case 'm':
			return BLOCK_MAGIC;
		case 'F':
		case 'f':
			return BLOCK_FIRE;
		default:
			return BLOCK_EMPTY;
	}
}