#define GLM_FORCE_CUDA
#include <cuda.h>
#include "ParallelTracer.cuh"
#include "glm/glm.hpp"
#include "Tracer.h"
#include <iostream>
#include "Scene.h"
#include "Tracer.h"
#include <cstdlib>
#include "glm/gtx/perpendicular.hpp"


struct Devim {
	glm::uvec3 *texture;
	int pitch;
	int width;
	int height;
}; 


//next function from http://ray-tracing.ru/articles245.html
__device__ bool RaySphereIntersection2(glm::vec3 &ray_pos, glm::vec3 &ray_dir, glm::vec3 &spos, float r, float& tResult)
{
  //a == 1; // because rdir must be normalized
  glm::vec3 k = ray_pos - spos;
  double b = glm::dot(k, ray_dir);
  double c = glm::dot (k, k) - r*r;
  double d = b*b - c;
 
  if(d >=0)
  {
    double sqrtfd = sqrtf(d);
    // t, a == 1
    double t1 = -b + sqrtfd;
    double t2 = -b - sqrtfd;
 
    double min_t  = glm::min(t1,t2);
    double max_t = glm::max(t1,t2);
 
    double t = (min_t >= 0) ? min_t : max_t;
    tResult = float(t);
    return (t > 0);
  }
  return false;
}

__device__ 	bool DiskIntersection(glm::vec3 &raypos, glm::vec3 &dir, float &t)
{
	bool ans = true;
	ans = abs(dir.z) >= 0.0000000001;
	if (ans){
		t = float(-raypos.z / dir.z);
		ans = ans & t >=0;
	}
	return ans;
}

__device__ SRay MakeRay(CTracer *t, glm::uvec2 pixelPos)
{
	int xRes = t->m_camera.m_resolution.x;
	int yRes = t->m_camera.m_resolution.y;
	float coef = float(xRes) / (2 * tan(t->m_camera.m_viewAngle.x / 2 ));
	glm::vec3 ViewDir = t->m_camera.m_forward *coef;
	coef = float(xRes) ;
	glm::vec3 Right = t->m_camera.m_right * coef;
	coef = float(yRes);
	glm::vec3 Up = t->m_camera.m_up * coef;
	float one = (pixelPos.x + 0.5)/xRes - 0.5;
	float two = (pixelPos.y + 0.5)/yRes - 0.5;

	glm::vec3 dir = glm::normalize(ViewDir + Right * one + Up * two);
	glm::vec3 start = t->m_camera.m_pos;
	SRay ray;
	ray.m_dir = dir;
	ray.m_start = start;
  return ray;
}

__device__ glm::uvec3 getpix(Devim pic, int x, int y)
{
	return pic.texture[x * pic.width + y] ;
} 


__device__ float len (glm::vec3 v){
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

__device__ glm::vec3 TraceRay(CTracer *th, SRay ray, Devim backtext, Devim disktext)
{
	glm::vec3 answer(1,0,0);
	float c_light = 300000000;
	float Gravity_const = 6.67408e-11;
	float llastrad, rad = glm::length(ray.m_start);
	glm::vec3 lastpos = ray.m_start;
	glm::vec3 lastdir = ray.m_dir;
	float dt = 0.00002;
	//glm::vec3 C(c_light, c_light, c_light);
	//glm::vec3 G(Gravity_const, Gravity_const, Gravity_const);
	float G = Gravity_const;
	float C = c_light;
	float M = th->m_pScene->bh.mass; 
	for (int i = 0; i < 100000; i++){
		
		llastrad = rad;
		lastpos = ray.m_start;
		lastdir = ray.m_dir;

		// compute new rate
		glm::vec3 a = -glm::normalize(ray.m_start) * G * M / glm::dot(ray.m_start, ray.m_start);
		// we should use only normal component
		glm::vec3 normal = glm::perp(a, ray.m_dir);
		//compute new dir
		ray.m_dir = glm::normalize(ray.m_dir * C + normal * dt) ;
		//compute peremeshenie with blackhole
		glm::vec3 dist = C * lastdir * dt + normal * dt * dt / 2.0f;
		ray.m_start += dist;
		float rad = glm::length(ray.m_start);
		float t;

		if (DiskIntersection(lastpos, glm::normalize(dist), t)){
			glm::vec3 hlp = lastpos + glm::normalize(dist) * t;
			if (t < glm::length(dist) && glm::length(hlp) < th->m_pScene->ad.radius){ 
				float x = hlp.x, y = hlp.y;
				int m, n;
				m	= (x / (th->m_pScene->ad.radius * 2) + 0.5) * disktext.width;
				n = (y / ( th->m_pScene->ad.radius * 2) + 0.5) * disktext.height;
				glm::uvec3 asd = getpix(disktext, m, n);
				return glm::vec3(asd.x / 255.0f, asd.y / 255.0f, asd.z / 255.0f);
			}
		}  
		if (RaySphereIntersection2(lastpos, glm::normalize(dist), glm::vec3(0,0,0), th->m_pScene->bh.radius, t ))
		{
			if (t < glm::length(dist)) {
				return glm::vec3(0,0,0); 
			}
		} 
		if (rad > llastrad && glm::length(ray.m_dir - lastdir) < 0.001 ){
			break;
		}
		dt = c_light / (1000 * glm::length(a));
	}

	float mPi = 3.14159265359f;
	float x, y, z;
	x = ray.m_dir.x; y = ray.m_dir.y; z = ray.m_dir.z;
	float fi = atan2f(x, y);
	float teta = asinf(z);
	fi = (fi + mPi) * (float(backtext.width) - 1) / (2 * mPi);
	teta = (teta + mPi / 2)  *( float( backtext.height ) - 1) / (mPi);
	glm::uvec3 t = getpix(backtext, int(teta), int(fi));
	return glm::vec3(float(t.x) / 255.0, float(t.y) / 255.0, float(t.z) / 255.0); 
}

__global__ void iter(CTracer *th, glm::vec3 *cam, int xres, int yres,
					Devim b, Devim d)
{
	int k = blockIdx.x * blockDim.x + threadIdx.x;
	if (k < xres * yres)
	{
		 SRay ray = MakeRay(th, glm::uvec2(k % xres,  k / xres));
		 cam[k] = TraceRay(th, ray, b, d);
	}
}


void help(CTracer *th, int xRes, int yRes, glm::vec3 *pic) {
	
	CScene *sc;
	glm::vec3 *cam;
	cudaMalloc((void **)&cam, xRes * yRes * sizeof(glm::vec3));
	cudaMalloc((void **) &sc, sizeof(CScene));
	cudaMemcpy(sc, th->m_pScene, sizeof(CScene), cudaMemcpyHostToDevice);
	
	Devim d;
	Devim b;
	
	d.pitch = th->m_pScene->ad.texture->pitch;
	d.width = th->m_pScene->ad.texture->im->GetWidth();
	d.height = th->m_pScene->ad.texture->im->GetHeight();
	glm::uvec3 *tmp = (glm::uvec3 *) malloc(sizeof(glm::uvec3) * d.width * d.height);
	for (int i = 0; i < d.height; i++){
		for (int j = 0; j < d.width; j++){
			tmp[i * d.width + j] = th->m_pScene->ad.texture->getpix(i,j);
		}
	}
	glm::uvec3 *disktext;

	cudaMalloc((void **)&disktext, sizeof(glm::uvec3) * d.width * d.height);

	cudaMemcpy(disktext, tmp, sizeof(glm::uvec3) * d.width * d.height, cudaMemcpyHostToDevice);
	d.texture = disktext;
	free(tmp);


	b.pitch = th->m_pScene->fone->pitch;
	b.width = th->m_pScene->fone->im->GetWidth();
	b.height = th->m_pScene->fone->im->GetHeight();

	tmp = (glm::uvec3 *) malloc(sizeof(glm::uvec3) * b.width * b.height);
	for (int i = 0; i < b.height; i++){
		for (int j = 0; j < b.width; j++){
			tmp[i * b.width + j] = th->m_pScene->fone->getpix(i,j);
		}
	}
	
	glm::uvec3 *backtext;
	cudaMalloc((void **)&backtext, sizeof(glm::uvec3) * b.width * b.height);
	cudaMemcpy(backtext, tmp, b.width * b.height * sizeof(glm::uvec3), cudaMemcpyHostToDevice);
	free(tmp);
	b.texture = backtext;

	th->m_pScene = sc;
	CTracer *q;
	cudaMalloc((void **)&q, sizeof(CTracer));

	cudaMemcpy(cam, pic, sizeof(glm::vec3) * xRes * yRes, cudaMemcpyHostToDevice);

	cudaMemcpy(q, th, sizeof(CTracer), cudaMemcpyHostToDevice);
	//iter<<<gridDim, blockDim>>>(q, cam, xRes, yRes, alisa);
	iter<<<ceilf(xRes * yRes / 512.0f), 512>>>(q, cam, xRes, yRes, b, d);
	cudaMemcpy(pic, cam, sizeof(glm::vec3) * xRes * yRes, cudaMemcpyDeviceToHost);
}