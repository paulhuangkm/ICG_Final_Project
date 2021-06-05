all: ray_tracing.cpp ray.h color.h vec3.h camera.h hittable_list.h hittable.h material.h rt.h sphere.h
	g++ -std=c++11 ray_tracing.cpp -o ray_tracing
	./ray_tracing > image.ppm