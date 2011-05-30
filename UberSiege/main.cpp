#include "Application.h"
#include <cstdlib>
#include <ctime>

int main() {	
	srand(time(NULL));
	Application app;
	if(app.startup())
		app.go();
	return 0;
}