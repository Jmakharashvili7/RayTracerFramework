#pragma once
#include "Config.h"
#include "json.h"

using json = nlohmann::json;

class JsonSphere
{
public:
	static Sphere LoadSphereFromFile(std::string fileName);
	static bool WriteSphereToFile(std::string fileName, Sphere sphere);
private:
	JsonSphere();
	~JsonSphere();
};

