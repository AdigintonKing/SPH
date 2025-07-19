# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch"
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch-build"
  "/home/adiginton/Documents/SPH_heat/build/extern/install/NeighborhoodSearch"
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/tmp"
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch-stamp"
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src"
  "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/adiginton/Documents/SPH_heat/build/extern/CompactNSearch/src/Ext_NeighborhoodSearch-stamp${cfgdir}") # cfgdir has leading slash
endif()
