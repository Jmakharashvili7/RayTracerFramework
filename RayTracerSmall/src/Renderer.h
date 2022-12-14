#pragma once
#include "Config.h"

class Renderer
{
public:
	static void SmoothScaling(MemoryPool* memPool);
	static void render(Sphere* spheres[], uint iteration);
	static void render(Sphere* spheres, uint iteration, MemoryPool* memPool);
private:
	Renderer();
	~Renderer();

	static Vec3f trace(const Vec3f& rayorig, const Vec3f& raydir, Sphere* spheres[], const int& depth);
	static Vec3f trace(const Vec3f& rayorig, const Vec3f& raydir, Sphere* spheres, const int& depth);
	static void DoTracing(int batchSize, int index, int width, float invHeight, float invWidth, float angle, float aspectRatio, Vec3f* image, Sphere* spheres[]);
	static void RenderBatch(Sphere* spheres, const uint& sphereIndex, const int& r, const int& batchSize, MemoryPool* memPool);
	static float mix(const float& a, const float& b, const float& mix);
};

