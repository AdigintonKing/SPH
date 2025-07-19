#ifndef VECTOR_HH
#define VECTOR_HH

#include <math.h>
//#include <GL/glew.h>
//#include <GL/glut.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

// #include <pcl/PCLPointCloud2.h>
// #include <pcl/io/pcd_io.h>
// #include <pcl/io/vtk_io.h>
// #include <pcl/features/normal_3d_omp.h>
// #include <pcl/surface/mls.h>
// #include <pcl/surface/poisson.h>
// #include <pcl/features/normal_3d.h>
// #include <pcl/surface/marching_cubes_hoppe.h>
// #include <pcl/surface/marching_cubes_rbf.h>
// #include <pcl/console/print.h>
// #include <pcl/console/parse.h>
// #include <pcl/console/time.h>
// #include <pcl/conversions.h>



// using namespace pcl;
// using namespace pcl::io;
// using namespace pcl::console;



//#include <CGAL/trace.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>


#include <CGAL/convex_hull_2.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Scale_space_surface_reconstruction_3.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>



typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>
#include <CGAL/Spatial_sort_traits_adapter_2.h>
#include <CGAL/Timer.h>
#include <CGAL/Memory_sizer.h>
#include "taichi.h"
typedef K::FT FT;
typedef K::Point_2 Point_2;
typedef std::vector<Point_2> Points;
typedef K::Point_3 Point_3;
typedef K::Vector_3 VectorCGal;
typedef std::vector<Point_3> Points3D;
// Point with normal vector stored in a std::pair.
typedef std::pair<Point_3, VectorCGal> PointVectorPair;




typedef CGAL::Polyhedron_3<K>                     Polyhedron_3;
typedef Polyhedron_3::Facet_iterator                   Facet_iterator;
typedef Polyhedron_3::Halfedge_around_facet_circulator Halfedge_facet_circulator;
typedef Polyhedron_3::Point_iterator                   Point_iterator;
typedef CGAL::Triangulation_vertex_base_with_info_2<unsigned, K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K>           Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>              TDS;
typedef CGAL::Exact_predicates_tag                               Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag> CDT;
typedef CDT::Point PointCDT_2;
typedef CDT::Vertex_handle Vertex_handle;
typedef CDT::Face_handle Face_handle;
typedef CDT::Finite_faces_iterator Finite_faces_iterator;
typedef CGAL::Spatial_sort_traits_adapter_2<K,PointCDT_2*> Search_traits;
//#include "rx_utility.h"
#include "../rx_matrix.hh"
typedef taichi::real Real ;

// Concurrency
#ifdef CGAL_LINKED_WITH_TBB
typedef CGAL::Parallel_tag Concurrency_tag;
#else
typedef CGAL::Sequential_tag Concurrency_tag;
#endif


typedef unsigned int uint;

	//! Each cell of the search space division grid
struct rxCell
{
uint *hSortedIndex; //!<Particle index sorted by hash value
uint *hGridParticleHash; //!<Grid hash value of each particle
uint *hCellStart; //!<Start index of each cell in the sort list
uint *hCellEnd; //!<End index of each cell in the sort list
uint uNumCells; //!<Total number of cells

uint *hSortedPolyIdx; //!<Polygon index sorted by hash value (with duplication)
uint *hGridPolyHash; //!<Grid hash value of each polygon
uint *hPolyCellStart; //!<Start index of each cell in the sort list
uint *hPolyCellEnd; //!<End index of each cell in the sort list

uint uNumPolyHash; //!<Number of cells containing polygons

};

// ------------------------------------------------------------------ -----------------------------
// structure for sorting by hash value
// ------------------------------------------------------------------ -----------------------------
struct rxHashSort
{
  uint hash;
  uint value;
};

/*!
  * Hash value comparison function
  * @ Param [in] left, right Value to compare
  @ Returnurn left < right
  */
inline bool LessHash (const rxHashSort &left, const rxHashSort &right)
{
return left.hash <right.hash;
};


class Sphere3D
{

protected:
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> color;
    std::vector<GLushort> indices;
    unsigned int sectors_, rings_;

public:
  
  
  void SetColor (float R, float G, float B, float A)
  {
    int r,s;
    std::vector<GLfloat>::iterator c = color.begin();
    for(r = 0; r < rings_; r++) for(s = 0; s < sectors_; s++)
    {
      *c++ = R;
      *c++ = G;
      *c++ = B;
      *c++ = A;
    }
  }
  
    void SolidSphere(float radius, unsigned int rings, unsigned int sectors)
    {
        sectors_ = sectors;
	rings_ = rings;
        float const R = 1./(float)(rings-1);
        float const S = 1./(float)(sectors-1);
        int r, s;

        vertices.resize(rings * sectors * 3);
        normals.resize(rings * sectors * 3);
        color.resize(rings * sectors * 4);
        std::vector<GLfloat>::iterator v = vertices.begin();
        std::vector<GLfloat>::iterator n = normals.begin();
        
        for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
                float const y = sin( -M_PI_2 + M_PI * r * R );
                float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
                float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );

                

                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                *n++ = x;
                *n++ = y;
                *n++ = z;
		//std::cout <<  " v["<< r*sectors+s << "] = ( " << vertices[r*sectors+s] << " , " << vertices[r*sectors+s+1] << " , " << vertices[r*sectors+s+2] << " )" << std::endl;
        }

        indices.resize(rings * sectors * 4);
        std::vector<GLushort>::iterator i = indices.begin();
         for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
                *i++ = r * sectors + s;
                *i++ = r * sectors + (s+1);
                *i++ = (r+1) * sectors + (s+1);
                *i++ = (r+1) * sectors + s;
		//std::cout <<  " q["<< r*sectors+s << "] = ( " << indices[r*sectors+s] << " , " << indices[r*sectors+s+1] << " , " << indices[r*sectors+s+2] << " , " << indices[r*sectors+s+3]<< " )" << std::endl;
        }
    }

    void draw(GLfloat x, GLfloat y, GLfloat z)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(x,y,z);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
        glNormalPointer(GL_FLOAT, 0, &normals[0]);
        glColorPointer(4, GL_FLOAT, 0, &color[0]);
        glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
        glPopMatrix();
	
	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
    }
};


template <class InputIterator>
void insert_with_info(CDT& cdt, InputIterator first,InputIterator last)
{
  std::vector<std::ptrdiff_t> indices;
  std::vector<PointCDT_2> points;
  std::ptrdiff_t index=0;

  for (InputIterator it=first;it!=last;++it){
    points.push_back( *it);
    indices.push_back(index++);
  }

  CGAL::spatial_sort(indices.begin(),indices.end(),Search_traits(&(points[0]),cdt.geom_traits()));

  CDT::Vertex_handle v_hint;
  CDT::Face_handle hint;
  for (typename std::vector<std::ptrdiff_t>::const_iterator
    it = indices.begin(), end = indices.end();
    it != end; ++it){
    v_hint = cdt.insert(points[*it], hint);
    if (v_hint!=CDT::Vertex_handle()){
      v_hint->info()=*it;
      hint=v_hint->face();
    }
  }
}



 float RxPythag( float a,  float b)
{
	float absa = abs(a), absb = abs(b);
    return (absa > absb ? absa*(float)sqrt((Real)(1.0+(absb/absa)*(absb/absa))) :
           (absb == 0.0 ? 0.0 : absb*(float)sqrt((Real)(1.0+(absa/absb)*(absa/absb)))));
}




#include <iostream>
#include <math.h>


struct Vector
{
  Real x, y, z;
  uint o;
  
  Vector(Real _x = 0, Real _y = 0, Real _z = 0,int _o=0): x(_x), y(_y), z(_z),o(_o)
  {}
  
  inline Real const &operator[](unsigned int i) const
  {
    return *(&x + i);
  }
  
  inline Real &operator[](unsigned int i)
  {
    return *(&x + i);
  }
  
  inline Vector operator-(Vector const &v) const
  {
    return Vector(x-v.x, y-v.y, z-v.z);
  }
  
  inline Vector operator+(Vector const &v) const
  {
    return Vector(x+v.x, y+v.y, z+v.z);
  }
  
  inline Vector operator*(Real s) const
  {
    return Vector(s*x, s*y, s*z);
  }
  
  inline Vector operator/(Real s) const
  {
    return operator*(1/s);
  }
  
  inline Vector &operator+=(Vector const &v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  
  inline Vector &operator-=(Vector const &v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  
  inline Vector &operator*=(Real d)
  {
    x *= d;
    y *= d;
    z *= d;
    return *this;
  }
  
  inline Vector &operator/=(Real d)
  {
    return *this *= 1.0/d;
  }

  inline Vector &operator=(Vector const &v) 
  {
    x = v.x;
    y = v.y;
    z = v.z;
    o = v.o;
    return *this;
  }
  
  // dot product
  inline Real operator*(Vector const &v) const
  {
    return v.x*x + v.y*y + v.z*z;
  }
  
  // order 1 menor; 2 maior; 0 igual;
  inline int operator<(Vector const &v) const
  {
    if(x<v.x)
     return 1;
    else if (x>v.x) return 2;
    else if(y<v.y) return 1;
    else if (y>v.y) return 2;
    else if (z<v.z) return 1;
    else if (z>v.z) return 2;
    else return 0;
  }
  
  // cross product
  inline Vector operator^(Vector const &v) const
  {
    return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
  }
  
  inline Real sqrnorm() const
  {
    return *this * *this;
  }
  
  inline Real norm() const
  {
    return sqrt(sqrnorm());
  }
  
  inline Vector &normalize()
  {
    Real n = this->norm();
    if (n != 0)
      *this *= (1.0/n);
    
    return *this;
  }
};

inline Vector operator*(Real d, Vector const &v)
{
  return v * d;
}

inline std::ostream &operator<<(std::ostream &os, Vector const &v)
{
  return os << '[' << v.x << ", " << v.y << ", " << v.z << ", " << v.o << ']';
}

#endif

