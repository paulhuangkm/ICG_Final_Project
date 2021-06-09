#ifndef MATERIAL_H
#define MATERIAL_H

#include "rt.h"

struct hit_record;


class material {
    public:
        virtual bool reflect_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const = 0;
        virtual bool refract_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const = 0;
        virtual color emitted() const = 0;
    public:
        bool is_reflect;
        bool is_refract;
        bool is_light;
};


class lambertian : public material {
    public:
        lambertian(const color& a) : albedo(a) {
            is_reflect = true;
            is_refract = false;
            is_light = false;
        }

        virtual bool reflect_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            auto scatter_direction = rec.normal + random_unit_vector();

            // Catch degenerate scatter direction
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;

            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

        virtual bool refract_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }

        virtual color emitted() const override {
            return color(0,0,0);
        }

    public:
        color albedo;
};


class metal : public material {
    public:
        metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {
            is_reflect = true;
            is_refract = false;
            is_light = false;
        }

        virtual bool reflect_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }

        virtual bool refract_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }

        virtual color emitted() const override {
            return color(0,0,0);
        }

    public:
        color albedo;
        double fuzz;
};

class dielectric : public material {
    public:
        dielectric(double index_of_refraction, const color& a) : ir(index_of_refraction), albedo(a) {
            is_reflect = true;
            is_refract = true;
            is_light = false;
        }

        virtual bool reflect_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            double relative_ir = rec.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = relative_ir * sin_theta > 1.0;
            vec3 direction;
            if (cannot_refract)
                attenuation = color(1.0, 1.0, 1.0) * albedo;
            else{
                double reflect_ratio = reflectance(cos_theta, ir);
                attenuation = color(reflect_ratio, reflect_ratio, reflect_ratio) * albedo;
            }

            direction = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, direction);
            return true;
        }

        virtual bool refract_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            attenuation = color(0.5, 0.5, 0.5);
            double relative_ir = rec.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = relative_ir * sin_theta > 1.0;
            vec3 direction;
            if (cannot_refract)
                return false;
            else {
                double refract_ratio = 1.0 - reflectance(cos_theta, ir);
                attenuation = color(refract_ratio, refract_ratio, refract_ratio) * albedo;
            }
            
            direction = refract(unit_direction, rec.normal, relative_ir);
            scattered = ray(rec.p, direction);
            return true;
        }

        virtual color emitted() const override {
            return color(0,0,0);
        }

    public:
        double ir; // Index of Refraction
        color albedo;

    private:
        static double reflectance(double cosine, double ref_idx) {
            // Use Schlick's approximation for reflectance.
            auto r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*pow((1 - cosine),5);
        }
};

class light : public material  {
    public:
        light(color c) : emit(c) {
            is_reflect = false;
            is_refract = false;
            is_light = true;
        }

        virtual bool reflect_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }

        virtual bool refract_ray(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }

        virtual color emitted() const override {
            return emit;
        }

    public:
        color emit;
};
#endif