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
#include "Config.h"
#include "Renderer.h"

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv)
{
	auto start = std::chrono::steady_clock::now();
	MemoryPool* memPool = new MemoryPool();
	Sphere* spheres = new Sphere[SPHERE_COUNT];

	// This sample only allows one choice per program execution. Feel free to improve upon this
	srand(13);
	//BasicRender
	//render(spheres, 0);
	//SimpleShrinking();
	Renderer::SmoothScaling(memPool);

	memPool->PrintInfo();

	auto end = std::chrono::steady_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
	std::cout << "Elapsed time: " << elapsed_seconds << ". " << std::endl;

	//for (Sphere* sphere : spheres)
	//	memPool->FreeMemory(sphere);

	memPool->PrintInfo();

	delete[] spheres;
	delete memPool;

	//MemoryTracker::WalkTheHeap();
	return 0;
}

