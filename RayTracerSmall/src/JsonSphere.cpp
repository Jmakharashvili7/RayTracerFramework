#include "JsonSphere.h"

Sphere JsonSphere::LoadSphereFromFile(std::string fileName)
{
	// load and parse the json file (Spheres is the folder name where the spheres should be kept)
	std::ifstream file("Spheres/" + fileName);

	// check if the file was found
	if (!file)
	{
		std::cout << fileName << " does not exist!" << std::endl;
		return Sphere(Vec3f(0), 0.0f, Vec3f(0));
	}

	json data = json::parse(file);

	// load the json values	
	Vec3f center = Vec3f(data["centerX"].get<float>(), data["centerY"].get<float>(), data["centerZ"].get<float>());
	float radius = data["radius"].get<float>();
	float reflection = data["reflection"].get<float>();
	float transperency = data["transperency"].get<float>();
	Vec3f surfaceColor = Vec3f(data["surfaceColorX"].get<float>(), data["surfaceColorY"].get<float>(), data["surfaceColorZ"].get<float>());
	Vec3f emissionColor = Vec3f(data["emissionColorX"].get<float>(), data["emissionColorY"].get<float>(), data["emissionColorZ"].get<float>());

	// return the sphere
	return Sphere(center, radius, surfaceColor, reflection, transperency, emissionColor);
}

bool JsonSphere::WriteSphereToFile(std::string fileName, Sphere sphere)
{
	return false;
}
