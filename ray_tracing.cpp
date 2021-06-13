#include "rt.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <stdio.h>

color ray_color(const ray& r, const hittable& world, int depth, color prev_attenuation) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);
    if ((prev_attenuation.x() <= 0.01) &&
        (prev_attenuation.y() <= 0.01) &&
        (prev_attenuation.z() <= 0.01) )
        return color(0,0,0);

    if (world.hit(r, 0.001, infinity, rec)) {
        color tmp_color(0, 0, 0);
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->is_reflect && rec.mat_ptr->reflect_ray(r, rec, attenuation, scattered))
            tmp_color += attenuation * ray_color(scattered, world, depth-1, attenuation * prev_attenuation);
        if (rec.mat_ptr->is_refract && rec.mat_ptr->refract_ray(r, rec, attenuation, scattered))
            tmp_color += attenuation * ray_color(scattered, world, depth-1, attenuation * prev_attenuation);
        if (rec.mat_ptr->is_light)
            tmp_color += rec.mat_ptr->emitted();
        
        return tmp_color;
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    auto albedo = color::random(0.9, 1);
                    sphere_material = make_shared<dielectric>(1.5, albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5, color(1.0, 1.0, 1.0));
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

int main() {

    // Image

    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    // World

    auto world = random_scene();
    // hittable_list world;

    // auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    // auto material_center = make_shared<light>(color(1.0, 1.0, 1.0));
    // auto material_left   = make_shared<dielectric>(1.5, color(1.0, 1.0, 1.0));
    // auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

    // world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    // world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    // world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0), -0.45, material_left));
    // world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    // Setup kdtree
    fprintf(stderr, "%d\n", world.objects.size());
    world.build();

    // Camera

    point3 lookfrom(-13,2,10);
    point3 lookat(0,0,0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 16.5;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);


    // Render
    printf("P3\n%d %d\n255\n", image_width, image_height);

    for (int j = image_height-1; j >= 0; --j) {
        fprintf(stderr, "\rScanlines remaining: %d ", j);
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth, color(1.0, 1.0, 1.0));
            }
            write_color(pixel_color, samples_per_pixel);
        }
    }
    fprintf(stderr, "\nFinished!!!\n");
}