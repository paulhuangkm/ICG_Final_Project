#include "rt.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "rectangle.h"
#include "triangle.h"
#define NONE 0
// 0: random/triangle scene, 1: cornell box
#define world_type 0

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
    if(world_type==1)
        return color(0,0,0);
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

hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light_source = make_shared<light>(color(15.0, 15.0, 15.0));

    auto material1 = make_shared<dielectric>(1.5, color(1.0, 1.0, 1.0));
    objects.add(make_shared<sphere>(point3(280, 200, 280), 50.0, material1));

    objects.add(make_shared<rectangle>(NONE, NONE, 0, 555, 0, 555, 1, 555, green));
    objects.add(make_shared<rectangle>(NONE, NONE, 0, 555, 0, 555, 1, 0, red));
    objects.add(make_shared<rectangle>(213, 343, NONE, NONE, 227, 332, 2, 554, light_source));
    objects.add(make_shared<rectangle>(0, 555, NONE, NONE, 0, 555, 2, 0, white));
    objects.add(make_shared<rectangle>(0, 555, NONE, NONE, 0, 555, 2, 555, white));
    objects.add(make_shared<rectangle>(0, 555, 0, 555, NONE, NONE, 3, 555, white));

    return objects;
}

hittable_list triangle_scene() {
    hittable_list objects;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -41; a < 41; a+=5) {
        for (int b = -41; b < 41; b+=5) {
            auto choose_mat = random_double();
            point3 p1(a + 0.9*random_double(), 0.4 + 3.0*random_double(), b + 0.9*random_double());
            point3 p2(p1.x() + random_double(1.0, 4.0), p1.y(), p1.z() - random_double(1.0, 4.0));
            point3 p3(p1.x() + random_double(0, 4.0), p1.y() + random_double(1.0, 4.0), p1.z() + random_double(0, 4.0));
            shared_ptr<material> sphere_material;

            if (choose_mat < 0.8) {
                // diffuse
                auto albedo = color::random() * color::random();
                sphere_material = make_shared<lambertian>(albedo);
                objects.add(make_shared<triangle>(p1, p2, p3, sphere_material));
            } else {
                // metal
                auto albedo = color::random(0.5, 1);
                auto fuzz = random_double(0, 0.5);
                sphere_material = make_shared<metal>(albedo, fuzz);
                objects.add(make_shared<triangle>(p1, p2, p3, sphere_material));
            }
        }
    }

    return objects;
}

int main() {

    int max_depth = 50;

    // Random Scene
    auto aspect_ratio = 3.0 / 2.0;
    int image_width = 1200;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 10;
    point3 lookfrom(39, 6, 9);
    point3 lookat(0,0,0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 30.0;
    auto aperture = 0.1;
    double vfov = 20.0;

    // Cornell Box
    // auto aspect_ratio = 1.0;
    // int image_width = 600;
    // int image_height = static_cast<int>(image_width / aspect_ratio);
    // int samples_per_pixel = 200;
    // point3 lookfrom(278, 278, -800);
    // point3 lookat(278, 278, 0);
    // vec3 vup(0,1,0);
    // auto dist_to_focus = 10.0;
    // auto aperture = 0.1;
    // double vfov = 40.0;

    // World

    auto world = triangle_scene();

    // Setup kdtree
    fprintf(stderr, "%d\n", world.objects.size());
    world.build();

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);


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