#pragma once
#include <tuple>
#include "glm/glm.hpp"
#include "Types.h"
#include "Scene.h"

#include "string"
#include "atlimage.h"
extern bool RaySphereIntersection(glm::vec3 ray_pos, glm::vec3 ray_dir, glm::vec3 spos, float r, float& tResult);

extern bool antislaysing;
extern bool alphamixed;
extern bool parallel;

class CTracer
{
public:
  SRay MakeRay(glm::uvec2 pixelPos);  // Create ray for specified pixel
  SRay MakeRay(glm::uvec2 pixelPos, float q, float w); // Create ray with antislaysing
  glm::vec3 TraceRay(SRay ray); // Trace ray, compute its color
  void RenderImage(int xRes, int yRes);
  void SaveImageToFile(std::string fileName);
  CImage* LoadImageFromFile(std::string fileName);

public:
  SCamera m_camera;
  CScene* m_pScene;
};