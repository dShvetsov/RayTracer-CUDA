#include "Scene.h"

//next function from http://ray-tracing.ru/articles245.html
 bool RaySphereIntersection(glm::vec3 ray_pos, glm::vec3 ray_dir, glm::vec3 spos, float r, float& tResult)
{
  //a == 1; // because rdir must be normalized
  glm::vec3 k = ray_pos - spos;
  double b = glm::dot(k, ray_dir);
  double c = glm::dot(k, k) - r*r;
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
    tResult = t;
    return (t > 0);
  }
  return false;
}
