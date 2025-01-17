#include "vec3.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "bvh.h"

#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<thread>
#include<vector>
#include<ctime>
#include "omp.h"
using namespace std;

//#include "rtweekend.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "pdf.h"
//#include "my_image.h"
#include "spectrum.h"
//#include "geometry.h"
#include "transform.h"
#include "primitive.h"
#include "triangle.h"
//#include "shape_list.h"
#include "matte.h"
#include "constant.h"

#include "reflection.h"
#include "glass.h"
#include "metal.h"

color ray_color(
    const ray& r, const color& background, const hittable& world,
    const shared_ptr<hittable>& lights, int depth
) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;

    //specular situation
    if (srec.is_specular) {
        return srec.attenuation
            * ray_color(srec.specular_ray, background, world, lights, depth - 1);
    }

    //Lambertian situation
    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    //sample pdf
    ray scattered = ray(rec.p, p.generate(), r.Time());
    auto pdf_val = p.value(scattered.direction());

    return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
        * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;

}


//color ray_color_new(
//    const ray& r, const color& background, const hittable& world,
//    const pbrt::PrimitiveLists& primitives,
//    const shared_ptr<hittable>& lights, int depth
//) {
//    hit_record rec;
//
//    // If we've exceeded the ray bounce limit, no more light is gathered.
//    if (depth <= 0)
//        return color(0, 0, 0);
//
//    //test hit the primitives
//    pbrt::SurfaceInteraction surface_rec;
//    bool hit_prim = primitives.Intersect(r, &surface_rec);
//
//    // If the ray hits nothing, return the background color.
//    //if (!world.hit(r, 0.001, infinity, rec))
//        //return background;
//
//    //test hit the world
//    if (!world.hit(r, 0.001, r.tMax, rec))
//    {   // If the ray hits nothing, return the background color.
//        if(!hit_prim)
//            return background;
//        //hit the prims
//        //scatter_record prim_srec;
//        //make rec from surface_rec
//        //hit_record p_rec;
//        
//        rec.p = surface_rec.p;
//        rec.normal = surface_rec.n;
//        rec.t = r.tMax;
//        rec.mat_ptr = surface_rec.mat_ptr;
//        rec.u = surface_rec.uv.x;
//        rec.v = surface_rec.uv.y;
//        rec.set_face_normal(r, rec.normal);
//        //return color(.3, .3, .3);
//                
//    }
//    //hit the world or prims
//    scatter_record srec;
//    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
//    if (!rec.mat_ptr->scatter(r, rec, srec))
//        return emitted;
//
//    if (srec.is_specular) {
//        return srec.attenuation
//            * ray_color_new(srec.specular_ray, background, world, primitives, lights, depth - 1);
//    }
//
//    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
//    mixture_pdf p(light_ptr, srec.pdf_ptr);
//
//    ray scattered = ray(rec.p, p.generate(), r.Time());
//    auto pdf_val = p.value(scattered.direction());
//
//    return emitted
//        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
//        * ray_color_new(scattered, background, world, primitives, lights, depth - 1) / pdf_val;
//
//}

inline pbrt::Spectrum color2spect(color c) {
    Float rgb[3] = { c.x,c.y,c.z };
    return pbrt::Spectrum::FromRGB(rgb);
}

pbrt::Spectrum ray_color_newmaterial(
    const ray& r, const pbrt::Spectrum& background, const hittable& world,
    const pbrt::PrimitiveLists& primitives,
    const shared_ptr<hittable>& lights, int depth
) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        //return color(0, 0, 0);
        return pbrt::Spectrum(0.0);

    //test hit the primitives
    pbrt::SurfaceInteraction surface_rec;
    bool hit_prim = primitives.Intersect(r, &surface_rec);

    // If the ray hits nothing, return the background color.
    //if (!world.hit(r, 0.001, infinity, rec))
        //return background;

    //test hit the world
    if (!world.hit(r, 0.001, r.tMax, rec))
    {   // If the ray hits nothing, return the background color.
        if (!hit_prim)
            return background;

        //hit the prims
            //make rec from surface_rec
        pbrt::MemoryArena arena;
        surface_rec.ComputeScatteringFunctions(r, arena, true);
        Vector3f wi;
        Point2f u(random_Float(), random_Float());
        Float pdf;
        //sample ; could add sample_type
        //-r.d?
        auto f = surface_rec.bsdf->Sample_f(-r.d, &wi, u, &pdf);
        ray scatterray = ray(surface_rec.p, wi, r.Time());
        auto absconsine = AbsDot(wi, surface_rec.n) / wi.Length() / surface_rec.n.Length();
        return f * ray_color_newmaterial(scatterray, background, world, primitives, lights, depth - 1)
            * absconsine / pdf;
        /*rec.p = surface_rec.p;
        rec.normal = surface_rec.n;
        rec.t = r.tMax;
        rec.mat_ptr = surface_rec.mat_ptr;
        rec.u = surface_rec.uv.x;
        rec.v = surface_rec.uv.y;
        rec.set_face_normal(r, rec.normal);*/
        //return color(.3, .3, .3);

    }
    //hit the world 
    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec))
        //return emitted;
        return color2spect(emitted);

    if (srec.is_specular) {
        return color2spect(srec.attenuation)
            * ray_color_newmaterial(srec.specular_ray, background, world, primitives, lights, depth - 1);
    }

    //importance sample
    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    ray scattered = ray(rec.p, p.generate(), r.Time());
    auto pdf_val = p.value(scattered.direction());

    return color2spect(emitted)
        + color2spect(srec.attenuation) * rec.mat_ptr->scattering_pdf(r, rec, scattered)
        * ray_color_newmaterial(scattered, background, world, primitives, lights, depth - 1) / pdf_val;

}

hittable_list random_scene() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(Point3f(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_Float();
            Point3f center(a + 0.9 * random_Float(), 0.2, b + 0.9 * random_Float());

            if ((center - Point3f(4, 0.2, 0)).Length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + Vector3f(0, random_Float(0, .5), 0);
                    world.add(make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_Float(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(Point3f(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(Point3f(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(Point3f(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list two_spheres(pbrt::PrimitiveLists& primitives) {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    objects.add(make_shared<sphere>(Point3f(0, -10, 0), 10, red));
    //objects.add(make_shared<sphere>(Point3f(0, 10, 0), 10, red));
    
    //auto sphere_ptr=make_shared<pbrt::Sphere>
    /**/
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(0, 10, 0)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto sphere_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 10, -10, 10, 360);
    //auto sphere_geo = make_shared< pbrt::GeometricPrimitive>(sphere_ptr, red, nullptr);
    auto sphere_geo = make_shared< pbrt::GeometricPrimitive>(sphere_ptr, nullptr, nullptr);
    
    primitives.add(sphere_geo);
    return objects;
}


shared_ptr< pbrt::GeometricPrimitive> test()
{
    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(0, 10, 0)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto sphere_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 10, -10, 10, 360);
    auto sphere_geo = make_shared< pbrt::GeometricPrimitive>(sphere_ptr, nullptr, nullptr);
    return sphere_geo;
}

hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(Point3f(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(Point3f(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(Point3f(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(Point3f(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(Point3f(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
    objects.add(make_shared<sphere>(Point3f(0, 7, 2), 2, difflight));

    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), aluminum);
    shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));
    objects.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, glass));
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, green));

    return objects;
}

hittable_list cornell_box_spectrum() {
    Float xyzred[3] = {};
    Float xyzwhite[3] = {};
    Float xyzgreen[3] = {};
    Float xyzlight[3] = {};
    
    Float rgbred[3] = { .65, .05, .05 };
    Float rgbwhite[3] = { .73, .73, .73 };
    Float rgbgreen[3] = { .12, .45, .15 };
    Float rgblight[3] = { 15, 15, 15 };

    pbrt::SampledSpectrum::Init();
    pbrt::SampledSpectrum Samp;
    Samp = pbrt::SampledSpectrum::FromRGB(rgbred, pbrt::SpectrumType::Reflectance);
    Samp.ToXYZ(xyzred);
    Samp = pbrt::SampledSpectrum::FromRGB(rgbwhite, pbrt::SpectrumType::Reflectance);
    Samp.ToXYZ(xyzwhite);
    Samp = pbrt::SampledSpectrum::FromRGB(rgbgreen, pbrt::SpectrumType::Reflectance);
    Samp.ToXYZ(xyzgreen);
    Samp = pbrt::SampledSpectrum::FromRGB(rgblight, pbrt::SpectrumType::Illuminant);
    Samp.ToXYZ(xyzlight);

    hittable_list objects;

    auto xyz2vec = [](Float xyz[3]) {
        return Vector3f(xyz[0], xyz[1], xyz[2]);
    };

    auto red = make_shared<lambertian>(xyz2vec(xyzred));
    auto white = make_shared<lambertian>(xyz2vec(xyzwhite));
    auto green = make_shared<lambertian>(xyz2vec(xyzgreen));
    auto light = make_shared<diffuse_light>(xyz2vec(xyzlight));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3ff(165, 330, 165), aluminum);
    shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));
    objects.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, glass));

    return objects;
}

hittable_list cornell_smoke() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    //objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));

    shared_ptr<hittable> box2 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, Vector3f(130, 0, 65));

    objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    return objects;
}

hittable_list cornell_box_primitive(pbrt::PrimitiveLists& primitives) {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), aluminum);
    shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));
    objects.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(190, 90, 190)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto Sph1_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 90, -90, 90, 360);
    auto Sph1_geo = make_shared< pbrt::GeometricPrimitive>(Sph1_ptr, nullptr, nullptr);
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, glass));
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, red));
    primitives.add(Sph1_geo);
    return objects;
}


hittable_list cornell_box_trimesh(pbrt::PrimitiveLists& primitives) {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), aluminum);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    //box1 = make_shared<rotate_y>(box1, 15);
    //box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));
    //objects.add(box1);

    //auto glass = make_shared<dielectric>(1.5);
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, glass));
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, green));

    char* loadpath = "D:/Material/CodeField/Cpp_Code/PBRT/PBRT-Learning/data/David328.obj";
    auto ident = make_shared<pbrt::Transform>();
    //pbrt::Rotate(30, Point3f(0, 0, 200), Vector3f(1, 0, 0))*
    //cat_head
    auto obj2wor = make_shared<pbrt::Transform>(
        pbrt::Translate(Vector3f(220, 0, 0)) *
        pbrt::Rotate(-90, Point3f(400, 400, 0), Vector3f(0, 1, 0)) *
        pbrt::Translate(Vector3f(360, 360, 360)) *
        pbrt::Scale(20, 20, 20) *
        pbrt::RotateX(-60));
    //cone_tri
    /*auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(180,50,330))*
        pbrt::Rotate(25, Point3f(0, 0, 200), Vector3f(1, 0, 0))*
        pbrt::Rotate(180,Point3f(100,100,0),Vector3f(0,1,0)));*/
        //cout << "obj2wor: " << *obj2wor << endl;
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto trimesh = pbrt::CreateTriangleMesh(obj2wor, wor2obj, false, loadpath);
    //auto trimesh_shape = make_shared<pbrt::Shape_list>(trimesh, ident, ident);//(trimesh,nullptr,nullptr,false)
    //auto trimesh_geo = make_shared<pbrt::GeometricPrimitive>(trimesh_shape, green, nullptr);
    //primitives.push_back(trimesh_geo);
    //primitives.add(trimesh, green, nullptr);
    Float green_rgb[3] = { .32, .45, .55 };
    auto Kd = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(pbrt::Spectrum::FromRGB(green_rgb));
    auto sigma = make_shared<pbrt::ConstantTexture<Float>>(0.f);
    auto matte_green = make_shared<pbrt::MatteMaterial>(Kd, sigma, nullptr);
    primitives.add(trimesh, matte_green, nullptr);
    return objects;
}
const int CopperSamples = 56;
const Float CopperWavelengths[CopperSamples] = {
    298.7570554, 302.4004341, 306.1337728, 309.960445,  313.8839949,
    317.9081487, 322.036826,  326.2741526, 330.6244747, 335.092373,
    339.6826795, 344.4004944, 349.2512056, 354.2405086, 359.374429,
    364.6593471, 370.1020239, 375.7096303, 381.4897785, 387.4505563,
    393.6005651, 399.9489613, 406.5055016, 413.2805933, 420.2853492,
    427.5316483, 435.0322035, 442.8006357, 450.8515564, 459.2006593,
    467.8648226, 476.8622231, 486.2124627, 495.936712,  506.0578694,
    516.6007417, 527.5922468, 539.0616435, 551.0407911, 563.5644455,
    576.6705953, 590.4008476, 604.8008683, 619.92089,   635.8162974,
    652.5483053, 670.1847459, 688.8009889, 708.4810171, 729.3186941,
    751.4192606, 774.9011125, 799.8979226, 826.5611867, 855.0632966,
    885.6012714 };

const Float CopperN[CopperSamples] = {
    1.400313, 1.38,  1.358438, 1.34,  1.329063, 1.325, 1.3325,   1.34,
    1.334375, 1.325, 1.317812, 1.31,  1.300313, 1.29,  1.281563, 1.27,
    1.249062, 1.225, 1.2,      1.18,  1.174375, 1.175, 1.1775,   1.18,
    1.178125, 1.175, 1.172812, 1.17,  1.165312, 1.16,  1.155312, 1.15,
    1.142812, 1.135, 1.131562, 1.12,  1.092437, 1.04,  0.950375, 0.826,
    0.645875, 0.468, 0.35125,  0.272, 0.230813, 0.214, 0.20925,  0.213,
    0.21625,  0.223, 0.2365,   0.25,  0.254188, 0.26,  0.28,     0.3 };

const Float CopperK[CopperSamples] = {
    1.662125, 1.687, 1.703313, 1.72,  1.744563, 1.77,  1.791625, 1.81,
    1.822125, 1.834, 1.85175,  1.872, 1.89425,  1.916, 1.931688, 1.95,
    1.972438, 2.015, 2.121562, 2.21,  2.177188, 2.13,  2.160063, 2.21,
    2.249938, 2.289, 2.326,    2.362, 2.397625, 2.433, 2.469187, 2.504,
    2.535875, 2.564, 2.589625, 2.605, 2.595562, 2.583, 2.5765,   2.599,
    2.678062, 2.809, 3.01075,  3.24,  3.458187, 3.67,  3.863125, 4.05,
    4.239563, 4.43,  4.619563, 4.817, 5.034125, 5.26,  5.485625, 5.717 };

hittable_list cornell_box_newmaterial(pbrt::PrimitiveLists& primitives) {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    //shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    //shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), aluminum);
    /*shared_ptr<hittable> box1 = make_shared<box>(Point3f(0, 0, 0), Point3f(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vector3f(265, 0, 295));
    objects.add(box1);*/

    auto glass = make_shared<dielectric>(1.5);
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(250, 90, 250)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto Sph1_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 90, -90, 90, 360);

        //switch
    //new MatteMaterial
    /*Float green_rgb[3] = { .32, .45, .55 };
    auto Kd = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(pbrt::Spectrum::FromRGB(green_rgb));
    auto sigma= make_shared<pbrt::ConstantTexture<Float>>(30.f);
    auto matte_green = make_shared<pbrt::MatteMaterial>(Kd, sigma, nullptr);
    auto Sph1_geo = make_shared< pbrt::GeometricPrimitive>(Sph1_ptr, matte_green, nullptr);*/

    //new Glass Material
    //auto Kr = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(pbrt::Spectrum(1));
    //auto Kt = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(pbrt::Spectrum(1));
    //auto uroughness = make_shared<pbrt::ConstantTexture<Float>>(0);
    //auto vroughness = make_shared<pbrt::ConstantTexture<Float>>(0);
    //auto index = make_shared<pbrt::ConstantTexture<Float>>(1.5);
    //bool remapRoughness = true;
    //auto new_glass = make_shared<pbrt::GlassMaterial>(Kr, Kt, uroughness, vroughness, index, nullptr, remapRoughness);
    //auto glass_shpere = make_shared<pbrt::GeometricPrimitive>(Sph1_ptr, new_glass, nullptr);
     
    auto eta_spect = pbrt::Spectrum::FromSampled(CopperWavelengths, CopperN, CopperSamples);
    auto k_spect = pbrt:: Spectrum::FromSampled(CopperWavelengths, CopperK, CopperSamples);
    auto eta = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(eta_spect);
    auto k = make_shared<pbrt::ConstantTexture<pbrt::Spectrum>>(k_spect);
    auto rough = make_shared<pbrt::ConstantTexture<Float>>(.01f);
    auto uroughness = make_shared<pbrt::ConstantTexture<Float>>(0);
    auto vroughness = make_shared<pbrt::ConstantTexture<Float>>(0);
    bool remapRoughness = true;
    auto new_metal = make_shared<pbrt::MetalMaterial>(eta, k, rough, uroughness, vroughness, nullptr, remapRoughness);
    auto metal_shpere = make_shared<pbrt::GeometricPrimitive>(Sph1_ptr, new_metal, nullptr);
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, glass));
    //objects.add(make_shared<sphere>(Point3f(190, 90, 190), 90, red));
    primitives.add(metal_shpere);
    return objects;
}

hittable_list final_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_Float(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(Point3f(x0, y0, z0), Point3f(x1, y1, z1), ground));
        }
    }

    hittable_list objects;

    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = Point3f(400, 400, 200);
    auto center2 = center1 + Vector3f(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(make_shared<sphere>(Point3f(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(
        Point3f(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
        ));

    auto boundary = make_shared<sphere>(Point3f(360, 150, 145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(Point3f(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
    objects.add(make_shared<sphere>(Point3f(400, 200, 400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.1);
    objects.add(make_shared<sphere>(Point3f(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(Point3f::random(0, 165), 10, white));
    }

    objects.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
        Vector3f(-100, 270, 395)
        )
    );

    return objects;
}


int main() {

    // Image
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    //int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 100;

    const int max_depth = 50;

    // World

    hittable_list world;

    Point3f lookfrom;
    Point3f lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    //color background(0, 0, 0);
    pbrt::Spectrum background(0.f);

    bool Isspectrum = false;

    pbrt::PrimitiveLists primitives;

    switch (12) {
    case 1:
        world = random_scene();
        background = color2spect(color(0.70, 0.80, 1.00));
        lookfrom = Point3f(13, 2, 3);
        lookat = Point3f(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;

    case 2:
        //world = two_spheres();
        world = two_spheres(primitives);
        //background = color(0.70, 0.80, 1.00);
        background = color2spect(color(0.70, 0.80, 1.00));
        lookfrom = Point3f(13, 2, 3);
        lookat = Point3f(0, 0, 0);
        vfov = 20.0;
        break;

    case 3:
        world = two_perlin_spheres();
        //background = color(0.70, 0.80, 1.00);
        background = color2spect(color(0.70, 0.80, 1.00));
        lookfrom = Point3f(13, 2, 3);
        lookat = Point3f(0, 0, 0);
        vfov = 20.0;
        break;

    case 4:
        world = earth();
        //background = color(0.70, 0.80, 1.00);
        background = color2spect(color(0.70, 0.80, 1.00));
        lookfrom = Point3f(13, 2, 3);
        lookat = Point3f(0, 0, 0);
        vfov = 20.0;
        break;

    case 5:
        world = simple_light();
        samples_per_pixel = 400;
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(26, 3, 6);
        lookat = Point3f(0, 2, 0);
        vfov = 20.0;
        break;

    
    case 6:
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 100;//200
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        break;

    case 7:
        world = cornell_smoke();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        break;

    case 8:
        world = final_scene();
        aspect_ratio = 1.0;
        image_width = 800;
        //samples_per_pixel = 10000;
        samples_per_pixel = 800;
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(478, 278, -600);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        break;

    default:
    case 9:
        world = cornell_box_spectrum();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 100;//200
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        Isspectrum = true;
        break;

    case 10:
        world = cornell_box_primitive(primitives);
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 100;//200
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        Isspectrum = true;
        break;

    case 11:
        world = cornell_box_trimesh(primitives);
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 100;//200
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);
        vfov = 40.0;
        Isspectrum = true;
        break;
        
    case 12:
        world = cornell_box_newmaterial(primitives);
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 100;//200
        //background = color(0, 0, 0);
        background = pbrt::Spectrum(0.f);
        lookfrom = Point3f(10, 500, 10);
        lookat = Point3f(250, 90, 250);
        /*lookfrom = Point3f(278, 278, -800);
        lookat = Point3f(278, 278, 0);*/
        vfov = 40.0;
        Isspectrum = true;
        break;
    }


    shared_ptr<hittable_list> lights = make_shared<hittable_list>();
    lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
    //lights->add(make_shared<sphere>(Point3f(190, 90, 190), 90, shared_ptr<material>()));
    

    // Camera

    Vector3f vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render
    //auto earth_texture = make_shared<my_image>("../PBRT-Learning/data/earthmap.jpg");
    //std::cout << earth_texture->get_width() << " " << earth_texture->get_height() << " " << earth_texture->get_bytes_per_scanline() << std::endl;
    
    const size_t core_num = std::thread::hardware_concurrency();
    size_t use_num = core_num;
    vector<thread> workers;
    workers.resize(use_num);
    
    thread ww;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    
    size_t part_samples = samples_per_pixel / use_num;

    /*
    auto work = [=](int i, int j, size_t ps, color& pixel_color) {

        for (size_t s = 0; s < ps; ++s) {
            auto u = (i + random_Float()) / (image_width - 1);
            auto v = (j + random_Float()) / (image_height - 1);
            ray r = cam.get_ray(u, v);
            pixel_color += ray_color(r, background, world, lights, max_depth);
        }
    };*/
    
    //core code
    
    clock_t start, end;
    start = clock();
    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
           
            //color pixel_color(0, 0, 0);
            //for (size_t idt = 0; idt < use_num; idt++)
             //   workers[idt] = thread(work, i, j, part_samples, std::ref(pixel_color));
            //for (auto& worker : workers)
             //   worker.join();

            //color pixel_color(0, 0, 0);
            pbrt::Spectrum pixel_color(0.f);
#pragma omp parallel for
            for (int s = 0; s < samples_per_pixel; ++s) {
                   auto u = (i + random_Float()) / (image_width - 1);
                auto v = (j + random_Float()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                //pixel_color += ray_color(r, background, world, lights, max_depth);
                //pixel_color += ray_color_newmaterial(r, background, world, primitives, lights, max_depth);
                pixel_color += ray_color_newmaterial(r, background, world, primitives, lights, max_depth);
            }
           
            if (Isspectrum)
            {
                Float xyz_pixel[3] = { pixel_color[0], pixel_color[1], pixel_color[2] };
                Float rgb_pixel[3] = {};
                pbrt::XYZToRGB(xyz_pixel, rgb_pixel);
                Vector3f rgb_color(rgb_pixel[0], rgb_pixel[1], rgb_pixel[2]);
                write_color(std::cout, rgb_color, samples_per_pixel);
            }
            else
            {
                Float pixel_arr[3];
                pixel_color.ToRGB(pixel_arr);
                color pixel_rgb(pixel_arr[0], pixel_arr[1], pixel_arr[2]);
                write_color(std::cout, pixel_rgb, samples_per_pixel);
            }

        }
    }
    end = clock();
    double run_time = (double)(end - start) / CLOCKS_PER_SEC;
    std::cerr << "\nrun_time: "<<run_time << " s.\nDone.\n";/**/
    /*
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(250, 90, 250)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto Sph1_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 90, -90, 90, 360);
    auto Sph1_geo = make_shared< pbrt::GeometricPrimitive>(Sph1_ptr, nullptr, nullptr);
    Ray r1(Point3f(250.f, 90.f, 400.f), Vector3f(0, 0, -10));
    pbrt::SurfaceInteraction suf_rec;
    Sph1_geo->Intersect(r1, &suf_rec);
    std::cout << suf_rec.p << "\n";
    std::cout<<"normal: \n" << suf_rec.n << "\n";
    
    ray r;
    pbrt::SurfaceInteraction surface_rec;
    bool hit_prim = primitives.Intersect(r, &surface_rec);
    pbrt::MemoryArena arena;
    surface_rec.ComputeScatteringFunctions(r, arena);
    pbrt::BSDF bsd(surface_rec);

    
    Point3f p(1, 0, 0);
    cout << "p: "<< p << endl;
    if (pbrt::RotateZ(90) == pbrt::Rotate(90, Vector3f(0, 0, 2.6)))
        cout << "equal" << endl;
    else
        cout << "not equal" << endl;
    
    cout << "pbrt::RotateZ(90)*p: " << pbrt::RotateZ(90)(p) << endl;
    cout << "pbrt::Rotate(90,Point3f(-1,0,0),Vector3f(0,0,1))*p: " << pbrt::Rotate(90,Point3f(0,1,0),Vector3f(0,0,1))(p) << endl;
    
    char* loadpath = "D:/Material/CodeField/Cpp_Code/PBRT/PBRT-Learning/data/tri.obj";
    //pbrt::TriangleMesh triMesh(loadpath);
    //cout << triMesh.nVertices<<" " << triMesh.nTriangles << " " << triMesh.vertexIndices.size() << endl;

    //auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(190, 90, 190)));
    auto ident= make_shared<pbrt::Transform>();
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(300, 100, 100)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto trimesh = pbrt::CreateTriangleMesh(obj2wor, wor2obj, false, loadpath);
    
    auto trimesh_shape = make_shared<pbrt::Shape_list>(ident, ident, false, trimesh);
    auto trimesh_geo = make_shared<pbrt::GeometricPrimitive>(trimesh_shape, nullptr, nullptr);
    cout << "obj2wor: " << obj2wor->GetMatrix() << endl;

    std::shared_ptr<pbrt::TriangleMesh> mesh = std::make_shared<pbrt::TriangleMesh>(*obj2wor, loadpath);
    std::vector<std::shared_ptr<pbrt::Triangle>> tris;
    int nTriangles = mesh->nTriangles;
    tris.reserve(nTriangles);
    for (int i = 0; i < nTriangles; ++i)
        tris.push_back(std::make_shared<pbrt::Triangle>(obj2wor, wor2obj,
            false, mesh, i));
    //tris[0]->mesh

    Ray r1(Point3f(100.f, 100.f, -50.f), Vector3f(0, 0, 10));
    Ray r2(Point3f(100.f, 150.f, -50.f), Vector3f(0, 0, 10));
    std::cout << "tMax: " << r1.tMax << endl;
    Float tHit;
    pbrt::SurfaceInteraction surface_rec;
    if (trimesh_shape->IntersectP(r1))
    {
        trimesh_geo->Intersect(r1,&surface_rec);
        cout << "triMesh hit in: " << r1.tMax << endl;
    }
    else
        cout << "triMesh Not hit" << endl;

    
    pbrt::SurfaceInteraction surface_rec1;
    cout << "test primitive" << endl;

    auto trip = primitives[0];
    if (trip->Intersect(r2, &surface_rec1))
    {
        
        cout << "trip hit in: " << r2.tMax << endl;
        cout << "point: " << surface_rec1.p << endl;
    }
    else
        cout << "trip  Not hit" << endl;

    
    
    auto te = test();
    std::cout << te->WorldBound().pMin << endl;
    std::cout << te->WorldBound().pMax << endl;
    //std::cout << te->shape->Area() << endl;

    Ray r1(Point3f(0.f, -10.f, 0.f), Vector3f(0, 10, 0));
    std::cout<<"tMax: " << r1.tMax << endl;
    bool hit_prim = false;
    pbrt::SurfaceInteraction surface_rec;
    
    for (const auto& prim : primitives) {
        if (prim->IntersectP(r1))
        {
            hit_prim = true;
            prim->Intersect(r1, &surface_rec);
        }
    }
    if (te->IntersectP(r1))
    {
        hit_prim = true;
        te->Intersect(r1, &surface_rec);
    }
    if (hit_prim)
        cout << "hit prim: "<<surface_rec.p << endl;
    else
        cout << "not" << endl;
    std::cout << "tMax: " << r1.tMax << endl;

    if(surface_rec.mat_ptr==nullptr)
        std::cout << "surface_rec.mat_ptr == nullptr" << endl;
    else
        std::cout << "surface_rec.mat_ptr != nullptr" << endl;


    
    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto obj2wor = make_shared<pbrt::Transform>(pbrt::Translate(Vector3f(0, 10, 0)));
    auto wor2obj = pbrt::Inverse(obj2wor);
    auto sphere_ptr = make_shared<pbrt::Sphere>(obj2wor, wor2obj, false, 10, 10, 10, 2 * pi);
    auto sphere_geo = make_shared< pbrt::GeometricPrimitive>(sphere_ptr, red, nullptr);
    vector<shared_ptr<pbrt::Primitive>> tprimitives;
    tprimitives.push_back(sphere_geo);
    std::cout << primitives.size() << endl;
    std::cout << primitives[0]->WorldBound().pMax << endl;
    
    
    Point3f p1(1, 1, 1);
    cout <<"p1: " << p1 << endl;
    Vector3f vt(2.0, 3.0, 1.0);
    pbrt::Transform T = pbrt::Translate(vt);
    auto Rz = pbrt::RotateZ(90);
    auto Tp = T(p1);
    auto Rp = Rz(p1);
    Vector3f ve(0.1, 0.1, 0.1);
    auto pve = make_shared<Vector3f>(0.1, 0.1, 0.1);
    //auto vv = *pve;
    auto Tep = T(p1, pve.get());
    auto Rep = Rz(p1, &ve);
    cout << "TP: " << Tp << endl;
    cout << "TeP: " << Tep << endl;
    cout << "RP: " << Rp << endl;
    cout << "ReP: " << Rep << endl;
    auto RzT = Rz * T;
    auto RzTp = RzT(p1);
    cout << "PzTp: " << RzTp << endl;
    
    pbrt::SampledSpectrum::Init();
    color rgb1(0.5, 0.5, 0.5);
    Float rgb[3] = { 0.75, 0.25, 0.8 };
    Float torgb[3] = {};
    Float xyz[3] = {};
    Float xyztorgb[3] = {};
    Float rgbtoxyz[3] = {};
    pbrt::SampledSpectrum sam = pbrt::SampledSpectrum::FromRGB(rgb, pbrt::SpectrumType::Reflectance);
   
    sam.ToXYZ(xyz);
    pbrt::RGBToXYZ(rgb, rgbtoxyz);

    sam.ToRGB(torgb);
    pbrt::XYZToRGB(rgbtoxyz, xyztorgb);

    cout << "original rgb \n";
    for (int i = 0; i < 3; i++) {
        cout << rgb[i] << " ";
    }
    cout << endl;

    cout << "sam from rgb to rgb \n";
    for (int i = 0; i < 3; i++) {
        cout << torgb[i] << " ";
    }
    cout << endl;

    cout << "rgb to xyz to rgb \n";
    for (int i = 0; i < 3; i++) {
        cout << xyztorgb[i] << " ";
    }
    cout << endl << "--------------" << endl;

    cout << "sam from rgb to xyz: " << endl;
    for (int i = 0; i < 3; i++) {
        cout << xyz[i] << " ";
    }

    cout << "\n rgbtoxyz: \n";
    for (int i = 0; i < 3; i++) {
        cout << rgbtoxyz[i] << " ";
    }
    cout << endl;
    */
}