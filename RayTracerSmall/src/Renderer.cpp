#include "Renderer.h"

void Renderer::SmoothScaling(MemoryPool* memPool)
{
#ifdef SS_USE_THREADS
	uint batchSize = FRAME_COUNT / SS_THREAD_COUNT; // set how many frames should one thread make

	Sphere* spheres = new Sphere[SPHERE_COUNT];
	for (uint i = 0; i < SPHERE_COUNT; ++i)
	{
		spheres[i] = JsonSphere::LoadSphereFromFile("Sphere" + std::to_string(i + 1) + ".json");
	}

	// start the threads
	for (float i = 0; i < SS_THREAD_COUNT; ++i)
	{
		ThreadPool::AddJob([spheres, i, batchSize, memPool] { RenderBatch(spheres, 1, i, batchSize, memPool); });
	}

	ThreadPool::WaitAllJobs();
#else
	Sphere* spheres[SPHERE_COUNT];
	for (uint i = 0; i < SPHERE_COUNT; ++i)
	{
		spheres[i] = (Sphere*)memPool->GetMemory(sizeof(Sphere*));
		*spheres[i] = (JsonSphere::LoadSphereFromFile("Sphere" + std::to_string(i + 1) + ".json"));
	}

	for (float i = 0; i <= FRAME_COUNT; i++)
	{
		float r = i / 100;
		spheres[1]->radius = r;// Radius++ change here
		spheres[1]->radius2 = r * r;
		render(spheres, i);
		std::cout << "Rendered and saved spheres" << i << ".ppm" << std::endl;
	}
#endif
}

void Renderer::render(Sphere* spheres, uint iteration, MemoryPool* memPool)
{
	Sphere* ptrSpheres[SPHERE_COUNT];
	for (uint i = 0; i < SPHERE_COUNT; ++i)
	{
		ptrSpheres[i] = (Sphere*)memPool->GetMemory(sizeof(Sphere*));
		*ptrSpheres[i] = spheres[i];
	}

	render(ptrSpheres, iteration);

	for (uint i = 0; i < SPHERE_COUNT; ++i)
	{
		memPool->FreeMemory(ptrSpheres[i]);
	}
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void Renderer::render(Sphere* spheres[], uint iteration)
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

	for (int i = 0; i < threadCount; i++) {
		ThreadPool::AddJob([batchSize, i, width, invHeight, invWidth, angle, aspectratio, image, spheres] {
			DoTracing(batchSize, i, width, invHeight, invWidth, angle, aspectratio, image, spheres);
			});
	}

	ThreadPool::WaitAllJobs();
#else
	uint imgIndex = 0;
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			int index = y * width + x;
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

			// calcualte image pixel
			image[index] = trace(Vec3f(0), raydir, spheres, 0);

			// write the result to image
			// if the result is less than one than set it to 1
			finalImage[imgIndex] = (unsigned char)((1.0f < image[index].x ? 1.0f : image[index].x) * 255);
			finalImage[imgIndex + 1] = (unsigned char)((1.0f < image[index].y ? 1.0f : image[index].y) * 255);
			finalImage[imgIndex + 2] = (unsigned char)((1.0f < image[index].z ? 1.0f : image[index].z) * 255);
			imgIndex += 3;
		}
	}
#endif
	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::string fileName = "RT_Output/spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(fileName, std::ios::out | std::ios::binary);

	std::string line = "P6\n" + std::to_string(width) + " " + std::to_string(height) + "\n255\n";
	ofs.write(line.c_str(), line.length());
	ofs.write(finalImage, imageSize);

	ofs.close();
	delete[] image;
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
Vec3f Renderer::trace(const Vec3f& rayorig, const Vec3f& raydir, Sphere* spheres[], const int& depth)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	// find intersection of this ray with the sphere in the scene
	for (uint i = 0; i < SPHERE_COUNT; ++i) {
		float t0 = INFINITY, t1 = INFINITY;
		//std::cout << rayorig.length() << std::endl;
		//std::cout << raydir.length() << std::endl;
		//std::cout << t0 << " " << t1 << std::endl;
		if (spheres[i]->intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = spheres[i];
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
		for (uint i = 0; i < SPHERE_COUNT; ++i) {
			if (spheres[i]->emissionColor.x > 0) {
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i]->center - phit;
				lightDirection.normalize();
				for (unsigned j = 0; j < SPHERE_COUNT; ++j) {
					if (i != j) {
						float t0, t1;
						if (spheres[j]->intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i]->emissionColor;
			}
		}
	}

	return surfaceColor + sphere->emissionColor;
}

void Renderer::RenderBatch(Sphere* spheres, const uint& sphereIndex, const int& r, const int& batchSize, MemoryPool* memPool)
{
	Sphere* spheresCopy = new Sphere[SPHERE_COUNT];
	for (uint i = 0; i < SPHERE_COUNT; ++i)
	{
		spheresCopy[i] = spheres[i];
	}

	for (float i = 0; i < batchSize; i++)
	{
		float index = ((r * batchSize) + i);
		float rad = index / 100;
		spheresCopy[sphereIndex].radius = rad;
		spheresCopy[sphereIndex].radius2 = rad * rad;

		render(spheresCopy, index, memPool);
		std::cout << "Rendered and saved spheres" << index+1 << ".ppm" << std::endl;

	}
		delete[] spheresCopy;
}

void Renderer::DoTracing(int batchSize, int index, int width, float invHeight, float invWidth, float angle, float aspectRatio, Vec3f* image, Sphere* spheres[])
{
	for (uint i = 0; i < batchSize; ++i) {
		for (uint x = 0; x < width; ++x) {
			int y = i + (batchSize * index);
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectRatio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

			image[y * width + x] = trace(Vec3f(0), raydir, spheres, 0);
		}
	}
}

float Renderer::mix(const float& a, const float& b, const float& mix)
{
	return b * mix + a * (1 - mix);
}