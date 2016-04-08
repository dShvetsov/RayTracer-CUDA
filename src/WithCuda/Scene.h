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
	int getsize() {
		return im->GetHeight() * im->GetWidth() * 4;
	}
	glm::uvec3 getpix(int x, int y)
	{
	/*	if (x > im->GetHeight() || y > im->GetWidth()){
			return std::make_tuple(0,0,0,0);
		}	*/
		
		auto pCurrentLine = pData;
		pCurrentLine += pitch * x;
			return glm::uvec3( pCurrentLine[y * 4],
							   pCurrentLine[y * 4 + 1],
							   pCurrentLine[y * 4 + 2]);
	}
	Image (CImage *_im):im(_im) {
		pData = (unsigned char*)im->GetBits();
		pitch = im->GetPitch();
	}
};


//next function from http://ray-tracing.ru/articles245.html
class CScene
{
public: 
	Image *fone;
	struct BlackHole {
		float radius;
		float mass;
	};
	BlackHole bh;
	//AccDisk *disk;
	struct AccDisk{
		float radius;
		Image *texture;
	};
	AccDisk ad;
  // Set of meshes
};