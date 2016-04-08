#include "Tracer.h"
#include <cstdio>
#include <cstring>
#include "ParallelTracer.cuh"
#include <iostream>
#include "Timer.h"
double c_light = 300000000;
double Gravity_const = 6.67408e-11;

void main(int argc, char** argv)
{
  CTracer tracer;
  CScene scene;
  int xRes = 1024;  // Default resolution
  int yRes = 768;
  FILE *file;
  if(argc == 2) // There is input file in parameters
  {
    file = fopen(argv[1], "r");
  }
  else
  {
	file = fopen("config2.txt", "r");
  }
    if(file)
    {
      int xResFromFile = 0;
      int yResFromFile = 0;
      if(fscanf(file, "%d %d", &xResFromFile, &yResFromFile) == 2)
      {
        xRes = xResFromFile;
        yRes = yResFromFile;
      }
	  float x, y, z;
	  char config[256];
	  while (fscanf(file, "%s", config) > 0){
		  if (strcmp(config, "camera") == 0){
			  // read camera position
			  if (fscanf(file, "%f%f%f", &x, &y, &z) == 3)
			  {
				tracer.m_camera.m_pos = glm::vec3(x, y, z);
			  }
			  // read camera forward
			  if (fscanf(file, "%f%f%f", &x, &y, &z) == 3)
			  {
				  tracer.m_camera.m_forward = glm::normalize(glm::vec3(x, y, z));
			  }
			  // read camera up
			  if (fscanf(file, "%f%f%f", &x, &y, &z) == 3)
			  {
				  tracer.m_camera.m_up = glm::normalize(glm::vec3(x, y, z));
			  }
			  // read camera right
			  if (fscanf(file, "%f%f%f", &x, &y, &z) == 3)
			  {
				  tracer.m_camera.m_right = glm::normalize(glm::vec3(x, y, z));
			  }
			  //read camera view angles
			  if (fscanf(file, "%f%f", &x, &y) == 2)
			  {
				tracer.m_camera.m_viewAngle = glm::vec2(x, y);
			  }
		  } else if (strcmp(config, "background") == 0){
			  char name[1000];
			  if (fscanf(file, "%s", name) == 1)
			  {
				 scene.fone = new Image(tracer.LoadImageFromFile(std::string(name)));
			  }
		  } else if (strcmp(config, "blackhole") == 0){
			  double m;
			  if (fscanf(file, "%lf", &m) == 1)
			  {
				  scene.bh.mass = m;
				  scene.bh.radius = 2 * Gravity_const * m / (c_light * c_light);
			  }
		  } else if (strcmp(config, "disk") == 0) {
			  double m;
			  char name[1000];
			  if (fscanf(file, "%lf%s", &m, name) == 2)
			  {
				  scene.ad.radius = m * scene.bh.radius;
				  scene.ad.texture = new Image(tracer.LoadImageFromFile(std::string(name)));
			  }
		  }
	  }
      fclose(file);
	}
    else{
		printf("Invalid config path!\r\n");
		return;
	}
  tracer.m_pScene = &scene;
  Timer t;
  t.start();
  tracer.RenderImage(xRes, yRes);
  t.check("Time with CUDA");
  tracer.SaveImageToFile("Result.png");

}