#ifndef NEIGHBORDATA_HH
#define NEIGHBORDATA_HH
#include <glm/glm.hpp>
#include "glm/vec3.hpp"

struct NeighborData {
  int idx;
  
  glm::vec3 d_normalized;
  float d;
  float d_squared;
};




#endif

