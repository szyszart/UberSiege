#ifndef UBERS_MAPINFO_H
#define UBERS_MAPINFO_H

struct MountPoint {
	Ogre::Vector3 pos;	
	Ogre::Vector3 scale;
	double rot;
};

struct MapInfo {
	std::string heightMap;
	Ogre::Vector3 pathStart;
	Ogre::Vector3 pathEnd;
	std::vector<MountPoint> mountPoints;
};

#endif