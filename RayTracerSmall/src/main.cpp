// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <intrin.h>
#include "ThreadPool.h"
#include "Sphere.h"
#include "Vector3.h"
#include "MemoryPool.h"
#include "json.h"
#ifdef _WIN32
#include <algorithm>
#include <sstream>
#include <string.h>
#endif



using json = nlohmann::json;

#define uint unsigned int

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5
//[comment]
// Defined values to easily adjust threading for testing.
// SS_THREAD_COUNT = number of threads to use for smooth scaling function
// REND_THREAD_COUNT = number of threads to run trace function on in render
// FRAME_COUNT = number of frames to render
// SS_USE_THREAD = use threads for smooth scaling if defined
// REND_USE_THREADS = use threads for render if defined
//[/comment]
int threadMax = std::thread::hardware_concurrency();
#define SS_THREAD_COUNT threadMax
#define REND_THREAD_COUNT threadMax
#define FRAME_COUNT 120
#define SS_USE_THREADS
#define REND_USE_THREADSS


float mix(const float &a, const float &b, const float &mix)
{
	return b * mix + a * (1 - mix);
}

// Used for loading spheres in json format (path should not include the folder).
// Returns nullptr if the file was not found.
Sphere LoadSphereFromDisk(std::string fileName, MemoryPool* memPool = nullptr)
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

void WriteToDisk()
{

}

//[comment]
// This is the main trace function. It takes a ray as argument (defined by its origin
// and direction). We test if this ray intersects any of the geometry in the scene.
// If the ray intersects an object, we compute the intersection point, the normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective, diffuse).
// The function returns a color for the ray. If the ray intersects an object that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
//[/comment]
Vec3f trace(
	const Vec3f& rayorig,
	const Vec3f& raydir,
	const std::vector<Sphere> &spheres,
	const int& depth)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < spheres.size(); ++i) {
		float t0 = INFINITY, t1 = INFINITY;
		//std::cout << rayorig.length() << std::endl;
		//std::cout << raydir.length() << std::endl;
		//std::cout << t0 << " " << t1 << std::endl;
		if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}
	// if there's no intersection return black or background color
	if (!sphere) return Vec3f(2);

	Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - sphere->center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
					  // If the normal and the view direction are not opposite to each other
					  // reverse the normal direction. That also means we are inside the sphere so set
					  // the inside bool to true. Finally reverse the sign of IdotN which we want
					  // positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
	if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) {
		float facingratio = -raydir.dot(nhit);
		// change the mix value to tweak the effect
		float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
		refldir.normalize();
		Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->transparency) {
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
			refrdir.normalize();
			refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1); 
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (
			reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
	}
	else {
		// it's a diffuse object, no need to raytrace any further
		for (unsigned i = 0; i < spheres.size(); ++i) {
			if (spheres[i].emissionColor.x > 0) {
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i].center - phit;
				lightDirection.normalize();
				for (unsigned j = 0; j < spheres.size(); ++j) {
					if (i != j) {
						float t0, t1;
						if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
			}
		}
	}

	return surfaceColor + sphere->emissionColor;
}

void DoTracing(int batchSize, int index, int width, float invHeight, float invWidth, float angle, float aspectRatio, Vec3f* image, std::vector<Sphere>& spheres)
{
	for (int i = 0; i < batchSize; ++i) {
		for (int x = 0; x < width; ++x) {
			int y = i + (batchSize * index);
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectRatio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

			image[y * width + x] = trace(Vec3f(0), raydir, spheres, 0);
		}
	}
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const std::vector<Sphere>& spheres, unsigned int iteration)
{
	// Testing Resolution
	unsigned width = 640, height = 480;

	// Production Resolution
	// unsigned width = 1920, height = 1080;
	Vec3f* image = new Vec3f[width * height];
	unsigned int imageSize = (width * height) * 3;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 30, aspectratio = width / float(height);
	float angle = tan(M_PI * 0.5 * fov / 180.);
	char* finalImage = new char[imageSize];

#ifdef REND_USE_THREADS
	// Trace rays
	int threadCount = REND_THREAD_COUNT;
	int batchSize = height / threadCount;

	std::vector<std::thread> threads;
	for (int i = 0; i < threadCount; i++) {
		threads.push_back(std::thread(DoTracing, batchSize, i, width, invHeight, invWidth, angle, aspectratio, image, spheres));
	}
	for (auto& th : threads)
	{ 
		th.join();
	}
#else
	uint imgIndex = 0;
	for (uint y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x) {
			int index = y * width + x;
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

			// calcualte image pixel
			image[index] = trace(Vec3f(0), raydir, spheres, 0);

			// write the result to image
			// if the result is less than one than set it to 1
			finalImage[imgIndex]     = (unsigned char)((1.0f < image[index].x ? 1.0f : image[index].x) * 255);
			finalImage[imgIndex + 1] = (unsigned char)((1.0f < image[index].y ? 1.0f : image[index].y) * 255);
			finalImage[imgIndex + 2] = (unsigned char)((1.0f < image[index].z ? 1.0f : image[index].z) * 255);
			imgIndex += 3;
		}	
	}
#endif
	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::string fileName ="RT_Output/spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(fileName, std::ios::out | std::ios::binary);

	std::string line = "P6\n" + std::to_string(width) + " " + std::to_string(height) + "\n255\n";
	ofs.write(line.c_str(), line.length());
	ofs.write(finalImage, imageSize);

	ofs.close();
	delete[] image;
}

void RenderBatch(std::vector<Sphere> spheres, const uint& sphereIndex, const int& r, const int& batchSize)
{
	for (float i = 0; i <= batchSize; i++)
	{
		std::vector<Sphere> sphereCopy = spheres;

		float index = ((r * batchSize) + i);
		float rad = index / 100;
		sphereCopy[sphereIndex].radius = rad;
		sphereCopy[sphereIndex].radius2 = rad * rad;

		render(sphereCopy, index);
		std::cout << "Rendered and saved spheres" << index << ".ppm" << std::endl;

		sphereCopy.clear();
	}
}

void SmoothScaling(std::vector<Sphere> spheres, const uint& sphereIndex)
{
#ifdef SS_USE_THREADS
	std::vector<std::thread> threads;
	int batchSize = FRAME_COUNT/SS_THREAD_COUNT; // set how many frames should one thread make

	// start the threads
	for (float i = 0; i < SS_THREAD_COUNT; i++)
	{
		ThreadPool::AddTask([spheres, sphereIndex, i, batchSize] { RenderBatch(spheres, sphereIndex, i, batchSize); });
	}

	ThreadPool::WaitAllThreads();
#else
	for (float i = 0; i <= FRAME_COUNT; i++)
	{
		float r = i / 100;
		spheres[1].radius = r;// Radius++ change here
		spheres[1].radius2 = r * r;
		render(spheres, i);
		std::cout << "Rendered and saved spheres" << i << ".ppm" << std::endl;
	}
#endif
}



//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv)
{
	auto start = std::chrono::steady_clock::now();
	MemoryPool* memPool = new MemoryPool();
	std::vector<Sphere> spheres;

	uint sphereCount = 4;
	for (uint i = 1; i <= sphereCount; i++)
	{
		spheres.push_back(LoadSphereFromDisk("Sphere" + std::to_string(i) + ".json"));
	}

	// This sample only allows one choice per program execution. Feel free to improve upon this
	srand(13);
	//BasicRender
	//render(spheres, 0);
	//SimpleShrinking();
	SmoothScaling(spheres, 1);

	memPool->PrintInfo();

	auto end = std::chrono::steady_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
	std::cout << "Elapsed time: " << elapsed_seconds << ". " << std::endl;

	//for (Sphere* sphere : spheres)
	//	memPool->FreeMemory(sphere);

	memPool->PrintInfo();

	spheres.clear();
	delete memPool;
	return 0;
}

