################################################################################
# general CMake and project setup
################################################################################
cmake_minimum_required(VERSION 3.1)

project      (SPH)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})


set(ExternalInstallDir "${CMAKE_BINARY_DIR}/extern/install" CACHE INTERNAL "")
set(EXT_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE INTERNAL "")
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(EXT_CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")
endif()

################################################################################
# own external libraries
################################################################################
include(SetUpExternalProjects)


find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#########################################################
# FIND GLUT
#########################################################
#find_package(GLUT REQUIRED)
#include_directories(${GLUT_INCLUDE_DIRS})
#link_directories(${GLUT_LIBRARY_DIRS})
#add_definitions(${GLUT_DEFINITIONS})
#if(NOT GLUT_FOUND)
#    message(ERROR " GLUT not found!")
#endif(NOT GLUT_FOUND)
#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
#########################################################





FIND_PACKAGE(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIRS} ${GSLCBLAS_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")



link_directories(${CMAKE_SOURCE_DIR}/lib)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

if(WIN32)
  set(LIBS glfw3 opengl32 assimp)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  find_package(X11 REQUIRED)
  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread)
  set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
  set(LIBS ${LIBS} ${APPLE_LIBS})
else()
  set(LIBS )
endif(WIN32)


find_package(CGAL QUIET COMPONENTS Core )

if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )
include_directories (BEFORE "../../include")

    include_directories (BEFORE "include")
else()

    message(STATUS "This program requires the CGAL library, and will not be compiled.")

endif()

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 14)
  endif ()
endmacro(use_cxx11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")


#Path of CImg.h file relative to this file path
set(CIMG_H_PATH ${PROJECT_SOURCE_DIR}/..)
include_directories( ${PROJECT_SOURCE_DIR} )
include_directories( ${CIMG_H_PATH} )




# ### CIMG related stuff
# Flags to enable fast image display, using the XSHM library.
SET(CIMG_XSHM_CCFLAGS  -Dcimg_use_xshm)

# Flags to enable screen mode switching, using the XRandr library.
SET(CIMG_XRANDR_CCFLAGS  -Dcimg_use_xrandr)

# Flags to enable native support for JPEG image files, using the JPEG library.
# ( http://www.ijg.org/ )
SET(CIMG_JPEG_CCFLAGS  -Dcimg_use_jpeg)

# Flags to enable native support for TIFF image files, using the TIFF library.
# ( http://www.libtiff.org/ )
SET(CIMG_TIFF_CCFLAGS  -Dcimg_use_tiff)

# Flags to enable native support for PNG image files, using the PNG library.
# ( http://www.libpng.org/ )
SET(CIMG_PNG_CCFLAGS  -Dcimg_use_png)

#Flags to enable OPENCV support (Camera)
# ( http://www.opencv.org/ )
SET(CIMG_OPENCV_CCFLAGS-Dcimg_use_opencv)

# Flags to enable native support for EXR image files, using the OpenEXR library.
# ( http://www.openexr.com/ )
SET(CIMG_OPENEXR_CCFLAGS  -Dcimg_use_openexr)

# Flags to enable native support for various video files, using the FFMPEG library.
# ( http://www.ffmpeg.org/ )
SET(CIMG_FFMPEG_CCFLAGS  -Dcimg_use_ffmpeg)

# Flags to enable native support of most classical image file formats, using the Magick++ library.
# ( http://www.imagemagick.org/Magick++/ )
SET(CIMG_MAGICK_CCFLAGS -Dcimg_use_magick)

# Flags to enable faster Discrete Fourier Transform computation, using the FFTW3 library
# ( http://www.fftw.org/ )
SET(CIMG_FFTW3_CCFLAGS  -Dcimg_use_fftw3)




# ### Search Additional Libraries ##########
FIND_PACKAGE(OpenCV)
#find_package(PCL)
FIND_PACKAGE(JPEG)
FIND_PACKAGE(TIFF)
FIND_PACKAGE(PNG)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)
FIND_PACKAGE(PCL 1.3 REQUIRED)

PKG_CHECK_MODULES(FFTW3 fftw3)
PKG_CHECK_MODULES(OPENEXR OpenEXR)
PKG_CHECK_MODULES(MAGICK Magick++)

# PKG_CHECK_MODULES(LIBAVCODEC libavcodec)
# PKG_CHECK_MODULES(LIBAVFORMAT libavformat)
# PKG_CHECK_MODULES(LIBSWSCALE libswscale)
# PKG_CHECK_MODULES(LIBAVUTIL libavutil)

if(NOT WIN32)
  FIND_PACKAGE(X11)
  FIND_PACKAGE(Threads REQUIRED)
endif()

# #### End of additional libraries search ##########


### Configure Paths according to detected packages
if(TIFF_FOUND)
  get_filename_component(TIFF_LIB_DIRS ${TIFF_LIBRARIES} PATH)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_TIFF_CCFLAGS}")
  link_directories(${TIFF_LIB_DIRS})
  include_directories(${TIFF_INCLUDE_DIR})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${TIFF_LIBRARIES})
endif()



if(JPEG_FOUND)
  get_filename_component(JPEG_LIB_DIRS ${JPEG_LIBRARIES} PATH)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_JPEG_CCFLAGS}")
  link_directories(${JPEG_LIB_DIRS})
  include_directories(${JPEG_INCLUDE_DIR})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${JPEG_LIBRARIES})
endif()




if (ZLIB_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_ZLIB_CCFLAGS}")
  link_directories(${ZLIB_LIB_DIRS})
  include_directories(${ZLIB_INCLUDE_DIR})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${ZLIB_LIBRARIES})

  # PNG requires ZLIB
  if(PNG_FOUND)
    SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_PNG_CCFLAGS}")
    link_directories(${PNG_LIB_DIRS})
    include_directories(${PNG_INCLUDE_DIR} )
    SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${PNG_LIBRARIES} )
  endif()
endif()




if(FFTW3_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_FFTW3_CCFLAGS}")
  link_directories( ${FFTW3_LIBRARY_DIRS} )
  include_directories( ${FFTW3_INCLUDE_DIRS} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${FFTW3_LIBRARIES} )
endif()




if(OPENEXR_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_OPENEXR_CCFLAGS}")
  link_directories( ${OPENEXR_LIBRARY_DIRS} )
  include_directories( ${OPENEXR_INCLUDE_DIRS} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${OPENEXR_LIBRARIES} )
endif()


if(MAGICK_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_MAGICK_CCFLAGS}")
  link_directories( ${MAGICK_LIBRARY_DIRS} )
  include_directories( ${MAGICK_INCLUDE_DIRS} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${MAGICK_LIBRARIES} )
endif()


include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})


if( LIBAVCODEC_FOUND  AND LIBAVFORMAT_FOUND AND LIBSWSCALE_FOUND AND LIBAVUTIL_FOUND )
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_FFMPEG_CCFLAGS}")
  link_directories( ${LIBAVFORMAT_LIBRARY_DIRS} )
  link_directories( ${LIBAVCODEC_LIBRARY_DIRS} )
  link_directories( ${LIBSWSCALE_LIBRARY_DIRS} )
  link_directories( ${LIBAVUTIL_LIBRARY_DIRS} )
  include_directories( ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS}/libavformat)
  include_directories( ${LIBAVCODEC_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS}/libavcodec )
  include_directories( ${LIBSWSCALE_INCLUDE_DIRS} ${LIBSWSCALE_INCLUDE_DIRS}/libswscale)
  include_directories( ${LIBAVUTIL_INCLUDE_DIRS} ${LIBAVUTIL_INCLUDE_DIRS}/libavutil )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${LIBAVFORMAT_LIBRARIES} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${LIBAVCODEC_LIBRARIES} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${LIBSWSCALE_LIBRARIES} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${LIBAVUTIL_LIBRARIES} )
endif()


if(NOT APPLE)
  if(NOT WIN32)
    if(X11_FOUND)
      SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_XSHM_CCFLAGS} ${CIMG_XRANDR_CCFLAGS}")
      SET(SYSTEM_LIBS ${SYSTEM_LIBS} Xext Xrandr)
    endif()
  endif(NOT WIN32)
endif(NOT APPLE)

if(X11_FOUND)
  link_directories(${X11_LIB_DIRS})
  include_directories(${X11_INCLUDE_DIR})
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${X11_LIBRARIES} )
endif()

if (NOT WIN32)
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
endif()

if( WIN32)
  SET( SYSTEM_LIBS  ${SYSTEM_LIBS}  gdi32 )
endif()

if (OpenCV_FOUND)
  message("OpenCV Found")
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_OPENCV_CCFLAGS}")
  include_directories(${OpenCV_INCLUDE_DIRS})
  link_directories(${OpenCV_LIB_DIRS})
  SET( SYSTEM_LIBS  ${SYSTEM_LIBS}  ${OpenCV_LIBS} )
endif()


if(LAPACK_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_LAPACK_CCFLAGS}")
  link_directories( ${LAPACK_LIBRARY_DIRS} )
  include_directories( ${LAPACK_INCLUDE_DIRS} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${LAPACK_LIBRARIES} )
endif()

if(BLAS_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_BLAS_CCFLAGS}")
  link_directories( ${BLAS_LIBRARY_DIRS} )
  include_directories( ${BLAS_INCLUDE_DIRS} )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${BLAS_LIBRARIES} )
endif()


# Add CIMG Flags to Compilation Flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CIMG_CFLAGS} ")



set  (project_SOURCES main.cpp
 ColorRGBA.hpp
 lgl/Error.cpp
 lgl/Shader.cpp
 lgl/FluidShader.cpp
 lgl/Util.cpp
 includes/imgui/imgui.cpp .
 includes/imgui/imgui_demo.cpp
 includes/imgui/imgui_draw.cpp
 includes/imgui/imgui_widgets.cpp
 includes/imgui/imgui_impl_glfw.cpp
 includes/imgui/imgui_impl_opengl3.cpp
)
set  (project_HEADERS
neighbordata.hh
sph.hh
Utilities/vector.hh
Cores.h
NeighborhoodSearch.h
Utilities/AVX_math.h
)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# first create relevant static libraries requried for other projects
add_library(STB_IMAGE "stb_image.cpp")
set(LIBS ${LIBS} STB_IMAGE)

add_library(GLAD "glad.c")
set(LIBS ${LIBS} ${OPENGL_LIBRARY} GLAD)

include_directories(${CMAKE_SOURCE_DIR}/includes/glew-1.13.0/include/)
include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories( ${CMAKE_SOURCE_DIR}/includes/imgui)
include_directories( ${CMAKE_SOURCE_DIR}/Utlities )
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${NEIGHBORHOOD_SEARCH_INCLUDE_DIR})
ADD_EXECUTABLE  (sph main.cpp
    lgl/Error.cpp
    lgl/Shader.cpp
    lgl/FluidShader.cpp
    lgl/Util.cpp
    includes/imgui/imgui.cpp
    includes/imgui/imgui_demo.cpp
    includes/imgui/imgui_draw.cpp
    includes/imgui/imgui_widgets.cpp
    includes/imgui/imgui_impl_glfw.cpp
    includes/imgui/imgui_impl_opengl3.cpp
    )

foreach(program ${CIMG_FILES})
  add_executable(${program} ${program}.cpp)
  target_link_libraries(${program} ${SYSTEM_LIBS} )
endforeach(program)
add_definitions( -DMAGICKCORE_HDRI_ENABLE=1 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lglfw -lGL  -mavx -Ofast -ffast-math -DIMGUI_IMPL_OPENGL_LOADER_GLAD -O4")

target_link_libraries(sph  ${LIBS} ${OPENGL_LIBRARIES}  ${NEIGHBORHOOD_SEARCH_LIBRARIES} ${SYSTEM_LIBS})


add_dependencies(sph
Ext_NeighborhoodSearch
)



