#pragma once
#ifndef Pi
#define Pi 3.14159265359
#endif

#include "atlimage.h"
#include "Types.h"
#include <utility>
#include "math.h"
#include <tuple>
#include "glm/glm.hpp"

extern int ClosetNeighboor(float x);
extern double c_light;
extern double Gravity_const;

struct Image{
	CImage *im;
	unsigned char *pData;
	int pitch;
	std::tuple<unsigned char, 
		       unsigned char,
			   unsigned char,
			   unsigned char> getpix(int x, int y)
	{
	/*	if (x > im->GetHeight() || y > im->GetWidth()){
			return std::make_tuple(0,0,0,0);
		}	*/
		
		auto pCurrentLine = pData;
		pCurrentLine += pitch * x;
			unsigned char b = pCurrentLine[y * 4];
			unsigned char g = pCurrentLine[y * 4 + 1];
			unsigned char r = pCurrentLine[y * 4 + 2];
			unsigned char alpha = pCurrentLine[y * 4 + 3];
			return std::make_tuple(r, g, b, alpha);
	}
	Image (CImage *_im):im(_im) {
		pData = (unsigned char*)im->GetBits();
		pitch = im->GetPitch();
	}
};


//next function from http://ray-tracing.ru/articles245.html
 bool RaySphereIntersection(glm::vec3 ray_pos, glm::vec3 ray_dir, glm::vec3 spos, float r, float& tResult);

struct Object{
	virtual std::pair< bool, glm::vec3> Intersection(glm::vec3 &pos, glm::vec3 &dir, double len) = 0;
	virtual std::tuple<unsigned char, 
					   unsigned char,
					   unsigned char,
					   unsigned char> getpix(glm::vec3 pos) = 0;
};

struct Sphere : public Object
{
	double radius;
	glm::vec3 pos;
	Sphere (double rad, glm::vec3 v): radius(rad), pos(v){
	}
	std::pair<bool,glm::vec3> Intersection(glm::vec3 &raypos, glm::vec3 &dir, double len)
	{
		float t;
		if ( RaySphereIntersection(raypos, dir, pos, radius, t)){
			if (t <= len){
				return std::make_pair(true, raypos + dir * t);
			}
			else{
				return std::make_pair(false, glm::vec3(0,0,0));
			}
		}
		else{
			return std::make_pair(false, glm::vec3(0,0,0));
		}

	}
};

struct BlackHole :public Sphere
{
	double mass;
	BlackHole(double _mass) : mass(_mass), Sphere( 2 * Gravity_const * _mass / (c_light * c_light), glm::vec3(0,0,0)) {
	}
	std::tuple<unsigned char, 
					   unsigned char,
					   unsigned char,
					   unsigned char> getpix(glm::vec3 pos) {
		return std::make_tuple(0,0,0,255);
	}
};

struct AccDisk : public Object
{
	Image *pict;
	double radius;
	AccDisk( double r, CImage *p ): radius(r)
	{
		pict = new Image(p);
	}
	std::pair<bool, glm::vec3> Intersection(glm::vec3 &raypos, glm::vec3 &dir, double len)
	{
		if (fabs(dir.z) < 0.0000001){
			return std::make_pair(false, glm::vec3(0,0,0));
		}
		double t = -raypos.z / dir.z;
		if (t >= 0 && t <= len) {
			auto wher = raypos + dir * glm::vec3(t, t, t);
			if (glm::length(wher) <= radius)
				return std::make_pair(true, wher);
			else
				return std::make_pair(false, wher);
		} else {
			return std::make_pair(false, glm::vec3(0,0,0));
		}
	}
	std::tuple<unsigned char, 
			   unsigned char,
			   unsigned char,
			   unsigned char> getpix(glm::vec3 pos) {
		double x, y;
		x = (pos.x / (radius * 2) + 0.5) * pict->im->GetWidth() ;
		y = (pos.y / (radius * 2) + 0.5) * pict->im->GetHeight() ;
		// load pixel
		return pict->getpix(ClosetNeighboor(x), ClosetNeighboor(y));
	}
};

struct Planet : public Sphere
{
	Image *im;
	Planet(double r, glm::vec3 v, CImage *i) : Sphere(r, v){
		im = new Image(i);
	}
	virtual std::tuple<unsigned char, 
					   unsigned char,
					   unsigned char,
					   unsigned char> getpix(glm::vec3 raypos){
	   float x, y, z;
	   glm::vec3 dir =glm::normalize(Sphere::pos - raypos);
	   x = dir.x; y = dir.y; z = dir.z;
	   float fi = atan2(x, y);
	   float teta = asin(z);
	   fi = (fi + Pi) * im->im->GetWidth() / (2 * Pi);
	   teta = (teta + Pi / 2)  * im->im->GetHeight() / (Pi);
	   unsigned char r, g, b, a;
	   return im->getpix(ClosetNeighboor(teta), ClosetNeighboor(fi));
	}
};

class CScene
{
public: 
	Image *fone;
	BlackHole *bh;
	//AccDisk *disk;
	std::vector<Object *> objs;
  // Set of meshes
};