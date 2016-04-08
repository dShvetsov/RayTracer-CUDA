#include "Tracer.h"

#include "glm/gtx/perpendicular.hpp"

#include "math.h"
using namespace glm;
#ifndef Pi
#define Pi 3.14159265359
#endif
extern double Gravity_const;

SRay CTracer::MakeRay(glm::uvec2 pixelPos, float q, float w)
{
	int xRes = m_camera.m_resolution.x;
	int yRes = m_camera.m_resolution.y;
	glm::vec3 coef( xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ), 
					xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ), 
					xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ) );
	glm::vec3 ViewDir = m_camera.m_forward * coef;
	coef = glm::vec3(xRes, xRes, xRes);
	glm::vec3 Right = m_camera.m_right * coef;
	coef = glm::vec3(yRes, yRes, yRes);
	glm::vec3 Up = m_camera.m_up * coef;
	glm::vec3 one((pixelPos.x + 0.5 + q)/xRes - 0.5,
				  (pixelPos.x + 0.5 + q)/xRes - 0.5,
				  (pixelPos.x + 0.5 + q)/xRes - 0.5 );
	glm::vec3 two((pixelPos.y + 0.5 + w)/yRes - 0.5,
				  (pixelPos.y + 0.5 + w)/yRes - 0.5,
				  (pixelPos.y + 0.5 + w)/yRes - 0.5 );
	glm::vec3 dir = glm::normalize(ViewDir + Right * one + Up * two);
	glm::vec3 start = m_camera.m_pos;
	SRay ray;
	ray.m_dir = dir;
	ray.m_start = start;
  return ray;
}

SRay CTracer::MakeRay(glm::uvec2 pixelPos)
{
	int xRes = m_camera.m_resolution.x;
	int yRes = m_camera.m_resolution.y;
	glm::vec3 coef( xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ), 
					xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ), 
					xRes / (2 * tan(m_camera.m_viewAngle.x / 2) ) );
	glm::vec3 ViewDir = m_camera.m_forward * coef;
	coef = glm::vec3(xRes, xRes, xRes);
	glm::vec3 Right = m_camera.m_right * coef;
	coef = glm::vec3(yRes, yRes, yRes);
	glm::vec3 Up = m_camera.m_up * coef;
	glm::vec3 one((pixelPos.x + 0.5)/xRes - 0.5,
				  (pixelPos.x + 0.5)/xRes - 0.5,
				  (pixelPos.x + 0.5)/xRes - 0.5 );
	glm::vec3 two((pixelPos.y + 0.5)/yRes - 0.5,
				  (pixelPos.y + 0.5)/yRes - 0.5,
				  (pixelPos.y + 0.5)/yRes - 0.5 );
	glm::vec3 dir = glm::normalize(ViewDir + Right * one + Up * two);
	glm::vec3 start = m_camera.m_pos;
	SRay ray;
	ray.m_dir = dir;
	ray.m_start = start;
  return ray;
}



inline int ClosetNeighboor(float x){
	if (x - int(x) > 0.5) 
		return int(x) + 1;
	else 
		return int (x);
}

glm::vec3 CTracer::TraceRay(SRay ray)
{
	float alpha = 0;
	glm::vec3 answerpix(0,0,0);
	float llastrad, rad = glm::length(ray.m_start);
	glm::vec3 lastpos = ray.m_start;
	glm::vec3 lastdir = ray.m_dir;
	glm::vec3 dt(0.00002, 0.00002, 0.00002);
	glm::vec3 C(c_light, c_light, c_light);
	glm::vec3 G(Gravity_const, Gravity_const, Gravity_const);
	glm::vec3 M(m_pScene->bh->mass, m_pScene->bh->mass, m_pScene->bh->mass); 
	
	for (int i = 0; i < 100000; i++){
		
		llastrad = rad;
		lastpos = ray.m_start;
		lastdir = ray.m_dir;

		// compute new rate
		glm::vec3 a = -glm::normalize(ray.m_start) * G * M / glm::dot(ray.m_start, ray.m_start);
		// we should use only normal component
		auto normal = glm::perp(a, ray.m_dir);
		//compute new dir
		ray.m_dir = glm::normalize(ray.m_dir * C + normal * dt) ;
		//compute peremeshenie with blackhole
		auto dist = C * lastdir * dt + normal * dt * dt / glm::vec3(2, 2, 2);
		//ray.m_start = ray.m_start + C * lastdir * dt + normal * dt * dt / glm::vec3(2, 2, 2);
		ray.m_start += dist;
		auto rad = glm::length(ray.m_start);
		// test if we in blackhole

		auto pr = m_pScene->bh->Intersection(lastpos, glm::normalize(dist), glm::length(dist));
		if (pr.first)
		{
			return answerpix + glm::vec3(0,0,0);
		}


		// test if we on disk
		/*
		pr = m_pScene->disk->Intersection(lastpos, lastdir, glm::length(ray.m_start - lastpos));
		if (pr.first){
			unsigned char r, g, b, a;
			std::tie(r, g, b, a) = 	m_pScene->disk->getpix(pr.second);
			if (a == 255)
			{
				return answerpix + glm::vec3(float(r) / 255, float(g) / 255, float(b) / 255);
			} else {
					alpha = float(a) / 255;
					answerpix += glm::vec3(alpha * float(r) / 255, alpha * float(g) / 255, alpha * float(b) / 255);
			}
		} */
		for (auto obj = m_pScene->objs.begin(); obj != m_pScene->objs.end(); obj++)
		{
			pr = (*obj)->Intersection(lastpos, glm::normalize(dist), glm::length(dist));
			if (pr.first){
				unsigned char r, g, b, a;
				std::tie(r, g, b, a) = (*obj)->getpix(pr.second);
				if (!alphamixed){
					a = 255;
				}
				if (a == 255)
				{
					return answerpix + glm::vec3(float(r) / 255, float(g) / 255, float(b) / 255);
				} else {
						alpha = float(a) / 255;
						answerpix += glm::vec3(alpha * float(r) / 255, alpha * float(g) / 255, alpha * float(b) / 255);
				}
			}
		}
		if (rad > llastrad && glm::length(ray.m_dir - lastdir) < 0.001 ){
			for (auto obj = m_pScene->objs.begin(); obj != m_pScene->objs.end(); obj++)
			{
				pr = (*obj)->Intersection(lastpos, lastdir, 10e40);
				if (pr.first){
					unsigned char r, g, b, a;
					std::tie(r, g, b, a) = (*obj)->getpix(pr.second);
					if (a == 255)
					{
						return answerpix + glm::vec3(float(r) / 255, float(g) / 255, float(b) / 255);
					} else {
							alpha = float(a) / 255;
							answerpix += glm::vec3(alpha * float(r) / 255, alpha * float(g) / 255, alpha * float(b) / 255);
					}
				}
			}
			break;
		}
		float timestep = c_light / (1000 * glm::length(a));
		dt = glm::vec3(timestep, timestep, timestep);
	}
	float x, y, z;
	x = ray.m_dir.x; y = ray.m_dir.y; z = ray.m_dir.z;
	float fi = atan2(x, y);
	float teta = asin(z);
	fi = (fi + Pi) *( m_pScene->fone->im->GetWidth() - 1) / (2 * Pi);
	teta = (teta + Pi / 2)  *( m_pScene->fone->im->GetHeight() - 1) / (Pi);
	unsigned char r, g, b, a;
	std::tie(r, g, b, a) =  m_pScene->fone->getpix(ClosetNeighboor(teta), ClosetNeighboor(fi));
	glm::vec3 color((1 - alpha) * float(r) / 255.0, (1 - alpha) * float(g) / 255.0, (1 - alpha) * float(b) / 255.0);
  return answerpix + color;
}


void CTracer::RenderImage(int xRes, int yRes)
{
  // Reading input texture sample
  /*	   I dunno but, seems this part of code useless

  CImage* pImage = LoadImageFromFile("data/disk_32.png");
  if(pImage->GetBPP() == 32)
  {
	   I dunno but, seems this part of code useless
    auto pData = (unsigned char*)pImage->GetBits();
    auto pCurrentLine = pData;
    int pitch = pImage->GetPitch();

    for(int i = 0; i < pImage->GetHeight(); i++) // Image lines
    {
      for(int j = 0; j < pImage->GetWidth(); j++) // Pixels in line
      {
        unsigned char b = pCurrentLine[j * 4];
        unsigned char g = pCurrentLine[j * 4 + 1];
        unsigned char r = pCurrentLine[j * 4 + 2];
        unsigned char alpha = pCurrentLine[j * 4 + 3];
      }
	  pCurrentLine += pitch;
    }
  }
  */
  // Rendering
  m_camera.m_resolution = uvec2(xRes, yRes);
  m_camera.m_pixels.resize(xRes * yRes);
if (parallel){
	int i;
	#pragma omp parallel for
	  for(i = 0; i < yRes; i++)
	  {
		for(int j = 0; j < xRes; j++)
		{
			if (antislaysing)
			{
				SRay ray = MakeRay(uvec2(j, i), 0.25, 0.25);
				auto a = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), 0.25, -0.25);
				auto b = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), -0.25, 0.25);
				auto c = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), -0.25, -0.25);
				auto d = TraceRay(ray);
				m_camera.m_pixels[i * xRes + j] = ( a + b + c + d ) / glm::vec3(4, 4, 4);
			}
			else {
				 SRay ray = MakeRay(uvec2(j, i));
				 m_camera.m_pixels[i * xRes + j] = TraceRay(ray);
			}
		}
	  }
	} else {
	  for(int i = 0; i < yRes; i++)
	  {
		for(int j = 0; j < xRes; j++)
		{
			if (antislaysing)
			{
				SRay ray = MakeRay(uvec2(j, i), 0.25, 0.25);
				auto a = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), 0.25, -0.25);
				auto b = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), -0.25, 0.25);
				auto c = TraceRay(ray);
				ray = MakeRay(uvec2(j, i), -0.25, -0.25);
				auto d = TraceRay(ray);
				m_camera.m_pixels[i * xRes + j] = ( a + b + c + d ) / glm::vec3(4, 4, 4);
			}
			else {
				 SRay ray = MakeRay(uvec2(j, i));
				 m_camera.m_pixels[i * xRes + j] = TraceRay(ray);
			}
		}
	  }
	}
}


void CTracer::SaveImageToFile(std::string fileName)
{
  CImage image;

  int width = m_camera.m_resolution.x;
  int height = m_camera.m_resolution.y;

  image.Create(width, height, 24);
    
	int pitch = image.GetPitch();
	unsigned char* imageBuffer = (unsigned char*)image.GetBits();

	if (pitch < 0)
	{
		imageBuffer += pitch * (height - 1);
		pitch =- pitch;
	}

	int i, j;
	int imageDisplacement = 0;
	int textureDisplacement = 0;

	for (i = 0; i < height; i++)
	{
    for (j = 0; j < width; j++)
    {
      vec3 color = m_camera.m_pixels[textureDisplacement + j];

      imageBuffer[imageDisplacement + j * 3] = clamp(color.b, 0.0f, 1.0f) * 255.0f;
      imageBuffer[imageDisplacement + j * 3 + 1] = clamp(color.g, 0.0f, 1.0f) * 255.0f;
      imageBuffer[imageDisplacement + j * 3 + 2] = clamp(color.r, 0.0f, 1.0f) * 255.0f;
    }

		imageDisplacement += pitch;
		textureDisplacement += width;
	}

  image.Save(fileName.c_str());
	image.Destroy();
}

CImage* CTracer::LoadImageFromFile(std::string fileName)
{
  CImage* pImage = new CImage;

  if(SUCCEEDED(pImage->Load(fileName.c_str())))
    return pImage;
  else
  {
    delete pImage;
    return NULL;
  }
}