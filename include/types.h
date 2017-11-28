#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <tuple>
#include <assert.h>
#include <iostream>
#include <limits>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "matrix.h"
#include "EasyBMP.h"

#define PI 3.1415926535897932384626433832795028841971
#define STEPS 255
typedef Matrix<std::tuple<uint, uint, uint>> Image;

typedef glm::vec3 v3;
typedef glm::vec2 v2;
typedef glm::uvec2 uv2;
typedef glm::uvec3 uv3;
typedef glm::mat4 m4;

struct SCamera
{
	v3 pos;  // Camera position and orientation
	v3 up;
	v3 right;
	v3 front;  // Orthonormal basis
	float fov;

	SCamera(v3 Pos 	  = v3(0.f, 0.f, 2.f),
		    v3 Up 	  = v3(0.f, 1.f, 0.f),
		   	v3 Right  = v3(1.f, 0.f, 0.f),
		    float FOV = PI/2
		   );

	m4 GetViewMatrix() const{
		return glm::lookAt(pos, pos+front, up);
	}

};

struct SRay
{
    v3 orig;
    v3 dir;
    float strength;

    SRay(const v3& origin, const v3& direction, float Strength = 0.f): orig(origin), dir(direction)
    {}
    SRay (): orig(v3()), dir(v3())
    {}
    void set_strength(float s);
    static SRay build_ray(float x, float y, float width, float height, const SCamera& camera);
};

struct SLight 
{
    v3 pos; // light position
    v3 intens; // light intensity
    SLight(const v3& p, const float i = 1): pos(p), intens(v3(i,i,i)) {}
};

struct SPhong
{
	static v3 phong_calc(const v3& diff_const, // diffuse reflection constant
				         const v3& spec_const, // specular reflection constant
				         float alpha, // shininess constant for this material
				         const v3& normal, // normal at hit point on the surface
				         const v3& hit,
				         const SCamera& cam,
				         SLight light
				         ){
            // direction from the hit point to toward light source
		v3 dir_light = glm::normalize(light.pos - hit);
            // direction pointing towards the viewer
		v3 dir_view = glm::normalize(cam.pos - hit);
            // direction that a perfectly reflected ray of light would take from this point on the surface
            // glm::reflect(a,b) : return a - 2*(a,b)*b
            // dir_reflect = 2*(dir_light,normal)*normal - dir_light
		v3 dir_reflect = glm::normalize(-glm::reflect(dir_light, normal));  		
        
        float dot_light_normal = glm::dot(dir_light, normal); 
		float dot_reflect_view = glm::dot(dir_reflect, dir_view); 

            // calculate light intesity in hit point
        float radius = glm::distance(light.pos, hit);
        float area = 4*PI*radius*radius;
	    v3 color = light.intens / area;
		color *= 1e+6f;
            // return black color
		if(dot_light_normal < 0.f) // no light
			return v3(0.f, 0.f, 0.f);

            // return diffuse 
		if(dot_reflect_view < 0.f) // no reflections 
			return color * diff_const * dot_light_normal;

		    // return ambient + diffuse + specular
		float specular = pow(dot_reflect_view, alpha);
		return color * ( diff_const*dot_light_normal + spec_const*specular );
	}
};

struct SVoxel
{
 	v3 pos;
 	float edge;
 	v3 color;
 	float value;

 	SVoxel();
 	SVoxel(v3 Position,
          float Edge,
          v3 Color = v3(1.f, 1.f, 1.f),
          float Value = 0.f);
 	
    float toSDF(v3 point);
 	float distance(SRay& ray, float start, float end);
};

struct SVoxelGrid
{
	std::vector<SVoxel> voxels;
	v3 topleft;
	v3 botright;
	float voxel_edge;
	uint num_x, num_y, num_z;

	SVoxelGrid(v3 Topleft, v3 BotRight);
	
    void initialize();
	int find(v3 point);
	void draw(v3 point);
	void print();
};