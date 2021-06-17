#include "rt.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "rectangle.h"
#include <pthread.h>
#include <stdio.h>

#define NONE 0
#define MAXN 2000
#define NUM_THREAD 2

color result[MAXN][MAXN];
double aspect_ratio;
int image_width, image_height, samples_per_pixel, max_depth;
hittable_list world;
camera cam;
pthread_t tid[NUM_THREAD];
int thrd_start_end[NUM_THREAD+1];

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

hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light_source = make_shared<light>(color(15.0, 15.0, 15.0));

    auto material1 = make_shared<dielectric>(1.5, color(1.0, 1.0, 1.0));
    objects.add(make_shared<sphere>(point3(280, 75, 280), 50.0, material1));

    objects.add(make_shared<rectangle>(NONE, NONE, 0, 555, 0, 555, 1, 555, green));
    objects.add(make_shared<rectangle>(NONE, NONE, 0, 555, 0, 555, 1, 0, red));
    objects.add(make_shared<rectangle>(213, 343, NONE, NONE, 227, 332, 2, 554, light_source));
    objects.add(make_shared<rectangle>(0, 555, NONE, NONE, 0, 555, 2, 0, white));
    objects.add(make_shared<rectangle>(0, 555, NONE, NONE, 0, 555, 2, 555, white));
    objects.add(make_shared<rectangle>(0, 555, 0, 555, NONE, NONE, 3, 555, white));

    return objects;
}

void calculate_image(int i, int j) {
    for (int s = 0; s < samples_per_pixel; ++s) {
        auto u = (i + random_double()) / (image_width-1);
        auto v = (j + random_double()) / (image_height-1);
        ray r = cam.get_ray(u, v);
        color tmp = ray_color(r, world, max_depth, color(1.0, 1.0, 1.0));
        result[j][i] += tmp;
    }
}

void *thrd_function(void *start_end) {
    int *cur = (int *) start_end;
    int start = cur[0], end = cur[1];
    for(int j=start ; j<end ; ++j)
        for(int i=0 ; i<image_width ; ++i)
            calculate_image(i, j);
    return NULL;
}

int main() {

    // World

    world = cornell_box();

    // Camera

    point3 lookfrom(13,2,3);
    point3 lookat(0,0,0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    aspect_ratio = 1.0;
    max_depth = 50;
    image_width = 600;
    image_height = static_cast<int>(image_width / aspect_ratio);
    samples_per_pixel = 10;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    double vfov = 40.0;

    cam = camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);


    // Render
    printf("P3\n%d %d\n255\n", image_width, image_height);
    for(int i = 0; i<NUM_THREAD ; ++i)
        thrd_start_end[i] = image_height / NUM_THREAD * i;
    thrd_start_end[NUM_THREAD] = image_height + 1;
    
    for(int thrd = 0; thrd<NUM_THREAD ; ++thrd)
        pthread_create(&tid[thrd], NULL, thrd_function, &thrd_start_end[thrd]);

    for(int thrd = 0; thrd<NUM_THREAD ; ++thrd)
        pthread_join(tid[thrd], NULL);
    for (int j = image_height-1; j >= 0; --j)
        for (int i = 0; i < image_width; ++i)
            write_color(result[j][i], samples_per_pixel);
    fprintf(stderr, "\nFinished!!!\n");
}