#ifndef BoxTetIntersection_hh
#define BoxTetIntersection_hh

#include <algorithm>
#include <cassert>

namespace BoxTetIntersection { // I don't want to deal with name clashes between this and the main file.

struct Point2D
{
  typedef float type;
  float X;
  float Y;
};

struct Point3D
{
  typedef float type;
  float X;
  float Y;
  float Z;
};

struct Interval
{
  float min;
  float max;
  bool IsValid() const {
    return min <= max;
  }
};

/* Type comprehension */
/* HasInterval::type is Interval if one of the template arguments is Interval, float otherwise */
template <typename A, typename B> struct HasInterval2 { typedef float type; };
template <typename T> struct HasInterval2<Interval, T> { typedef Interval type; };
template <typename T> struct HasInterval2<T, Interval> { typedef Interval type; };
template <typename A, typename B, typename C> struct HasInterval3 // three arguments
{
  typedef typename HasInterval2<A, typename HasInterval2<B,C>::type>::type type;
};




/* Addition and subtraction */
Interval operator+(const Interval& I, const float& f)
{
  Interval ret;
  ret.min = I.min + f;
  ret.max = I.max + f;
  return ret;
}
Interval operator+(const float& f, const Interval& I) { return I + f; }
Interval operator-(const Interval& I, const float& f) { return I + (-1.0*f); }
Interval operator-(const float& f, const Interval& I)
{
  Interval ret;
  ret.min = f - I.max;
  ret.max = f - I.min;
  return ret;
}
Interval operator+(const Interval& I1, const Interval& I2)
{
  Interval ret;
  ret.min = I1.min + I2.min;
  ret.max = I1.max + I2.max;
  return ret;
}
Interval operator-(const Interval& I1, const Interval& I2)
{
  Interval ret;
  ret.min = I1.min - I2.max;
  ret.max = I1.max - I2.min;
  return ret;
}

/* Multiplication and division by float */
Interval operator*(const Interval& I, const float& f)
{
  Interval ret;
  ret.min = std::min(I.min*f, I.max*f);
  ret.max = std::max(I.min*f, I.max*f);
  return ret;
}
Interval operator*(const float& f, const Interval& I) { return I*f; }
Interval operator/(const Interval& I, const float& f)
{
  Interval ret;
  ret.min = std::min(I.min/f, I.max/f);
  ret.max = std::max(I.min/f, I.max/f);
  return ret;
}
//////////////////////////////////////////

bool operator<(const Interval& I, const float& f) { return I.max < f; }
bool operator<(const float& f, const Interval& I) { return f < I.min; }
bool operator>(const Interval& I, const float& f) { return f < I; }
bool operator>(const float& f, const Interval& I) { return I < f; }
bool operator<=(const Interval& I, const float& f) { return I.max <= f; }
bool operator<=(const float& f, const Interval& I) { return f <= I.min; }
bool operator>=(const Interval& I, const float& f) { return f <= I; }
bool operator>=(const float& f, const Interval& I) { return I <= f; }

Interval Intersection(Interval a, Interval b)
{
  Interval ret;
  ret.min = std::max(a.min, b.min);
  ret.max = std::min(a.max, b.max);
  return ret;
}

// Done with Interval arithmetic etc //

struct Box2D
{
  typedef Interval type;
  Interval X;
  Interval Y;
  bool IsValid() const { return X.IsValid() and Y.IsValid(); }
};

struct Box3D
{
  typedef Interval type;
  Interval X;
  Interval Y;
  Interval Z;
  bool Contains(Point3D p) const {
    return p.X >= X.min and p.X <= X.max and p.Y >= Y.min and p.Y <= Y.max and p.Z >= Z.min and p.Z <= Z.max;
  }
  bool IsValid() const { return X.IsValid() and Y.IsValid() and Z.IsValid(); }
  Box2D ProjectX() const {
    Box2D ret;
    ret.X = Y;
    ret.Y = Z;
    return ret;
  }
  Box2D ProjectY() const {
    Box2D ret;
    ret.X = X;
    ret.Y = Z;
    return ret;
  }
  Box2D ProjectZ() const {
    Box2D ret;
    ret.X = X;
    ret.Y = Y;
    return ret;
  }
};

Box2D Intersection(Box2D a, Box2D b)
{
  Box2D ret;
  ret.X = Intersection(a.X, b.X);
  ret.Y = Intersection(a.Y, b.Y);
  return ret;
}

Box3D Intersection(Box3D a, Box3D b)
{
  Box3D ret;
  ret.X = Intersection(a.X, b.X);
  ret.Y = Intersection(a.Y, b.Y);
  ret.Z = Intersection(a.Z, b.Z);
  return ret;
}

struct Tri
{
  Point2D vertex[3];
  Box2D GetHull() const {
    Box2D ret;

    ret.X.min = std::min(vertex[0].X, std::min(vertex[1].X, vertex[2].X));
    ret.X.max = std::max(vertex[0].X, std::max(vertex[1].X, vertex[2].X));

    ret.Y.min = std::min(vertex[0].Y, std::min(vertex[1].Y, vertex[2].Y));
    ret.Y.max = std::max(vertex[0].Y, std::max(vertex[1].Y, vertex[2].Y));
    return ret;
  }
};

struct Tet
{
  Point3D vertex[4];
  unsigned char NumVerticesInBox(Box3D box) const {
    unsigned char ret = 0;
    for(unsigned char i = 0; i < 4; i++) {
      if(box.Contains(vertex[i])) ret++;
    }
    return ret;
  }
  Box3D GetHull() const {
    Box3D ret;

    ret.X.min = std::min( std::min(vertex[0].X,vertex[1].X) , std::min(vertex[2].X,vertex[3].X) );
    ret.X.max = std::max( std::max(vertex[0].X,vertex[1].X) , std::max(vertex[2].X,vertex[3].X) );

    ret.Y.min = std::min( std::min(vertex[0].Y,vertex[1].Y) , std::min(vertex[2].Y,vertex[3].Y) );
    ret.Y.max = std::max( std::max(vertex[0].Y,vertex[1].Y) , std::max(vertex[2].Y,vertex[3].Y) );

    ret.Z.min = std::min( std::min(vertex[0].Z,vertex[1].Z) , std::min(vertex[2].Z,vertex[3].Z) );
    ret.Z.max = std::max( std::max(vertex[0].Z,vertex[1].Z) , std::max(vertex[2].Z,vertex[3].Z) );

    return ret;
  }
  Tri ProjectX(unsigned char i_v) const {
    Tri ret;
    for(unsigned char i = 0, j = 0; i < 4; i++) {
      if(i == i_v) continue;
      ret.vertex[j].X = vertex[i].Y;
      ret.vertex[j].Y = vertex[i].Z;
      j++;
    }
    return ret;
  }
  Tri ProjectY(unsigned char i_v) const {
    Tri ret;
    for(unsigned char i = 0, j = 0; i < 4; i++) {
      if(i == i_v) continue;
      ret.vertex[j].X = vertex[i].X;
      ret.vertex[j].Y = vertex[i].Z;
      j++;
    }
    return ret;
  }
  Tri ProjectZ(unsigned char i_v) const {
    Tri ret;
    for(unsigned char i = 0, j = 0; i < 4; i++) {
      if(i == i_v) continue;
      ret.vertex[j].X = vertex[i].X;
      ret.vertex[j].Y = vertex[i].Y;
      j++;
    }
    return ret;
  }
};

/* det2 takes the determinant of
a b
c d */
template <typename A, typename B, typename C, typename D>
typename HasInterval2<A, C>::type // infer the return type
det2(A a, B b, C c, D d)
{
  return a*d - b*c;
}

/* det3 takes the determinant of
a b c
d e f
g h i */
template <typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I>
typename HasInterval3<A,D,G>::type // infer the return type -- we assume that rows have equal type
det3(A a, B b, C c, D d, E e, F f, G g, H h, I i)
{
  return a*det2(e,f,h,i) + b*det2(f,d,i,g) + c*det2(d,e,g,h);
}

template <typename T>
typename T::type // Let T identify the correct return type -- T is either Point2D or Box2D
area(T a, Point2D b, Point2D c)
{
  return det2(a.X - c.X, a.Y - c.Y,
              b.X - c.X, b.Y - c.Y)/2.0;
}

// As defined in the paper.  This works for intervals or floats,
// since Point3D and Box3D both have members X, Y, Z.
template <typename T>
typename T::type // Let T identify the correct return type -- T is either Point3D or Box3D
vol(T a, Point3D b, Point3D c, Point3D d)
{
  return det3(a.X - d.X, a.Y - d.Y, a.Z - d.Z,
              b.X - d.X, b.Y - d.Y, b.Z - d.Z,
              c.X - d.X, c.Y - d.Y, c.Z - d.Z)/6.0;
}

bool TestIntersection(const Tri t, const Box2D b)
{

  Box2D H = t.GetHull();
  Box2D B_H = Intersection(b, H);
  if(not B_H.IsValid()) return false;
  float Chi_den = area(t.vertex[0], t.vertex[1], t.vertex[2]);
  if(Chi_den == 0.0) return false; // degenerate triangle
  Interval Chi1 = area(B_H, t.vertex[1], t.vertex[2])/Chi_den;
  Interval Chi2 = area(B_H, t.vertex[2], t.vertex[0])/Chi_den;
  Interval Chi3 = area(B_H, t.vertex[0], t.vertex[1])/Chi_den;
  if(Chi1 < 0.0 or Chi1 > 1.0) return false;
  if(Chi2 < 0.0 or Chi2 > 1.0) return false;
  if(Chi3 < 0.0 or Chi3 > 1.0) return false;
  return true;
}

bool TestIntersection(Tet t, Box3D b)
{
  // This function implements the algorithm from Ratschek and Rokne, International Journal of Computer Mathematics 65:3-4, 191-204, 1997.
  // Returns true if t and b intersect, false if they are disjoint.

  // Step 1
  if(t.NumVerticesInBox(b) > 0) return true;

  // Step 2
  Box3D H = t.GetHull();
  Box3D B_H = Intersection(H,b);

  // Step 3
  if(not B_H.IsValid()) return false;

  // Step 4
  float Gamma_den = vol(t.vertex[0], t.vertex[1], t.vertex[2], t.vertex[3]);
  assert(Gamma_den != 0.0);
  Interval Gamma1 =       vol(B_H, t.vertex[1], t.vertex[2], t.vertex[3])/Gamma_den;
  Interval Gamma2 = (-1.)*vol(B_H, t.vertex[2], t.vertex[3], t.vertex[0])/Gamma_den;
  Interval Gamma3 =       vol(B_H, t.vertex[3], t.vertex[0], t.vertex[1])/Gamma_den;
  Interval Gamma4 = (-1.)*vol(B_H, t.vertex[0], t.vertex[1], t.vertex[2])/Gamma_den;
  if(Gamma1 < 0.0 or Gamma1 > 1.0) return false;
  if(Gamma2 < 0.0 or Gamma2 > 1.0) return false;
  if(Gamma3 < 0.0 or Gamma3 > 1.0) return false;
  if(Gamma4 < 0.0 or Gamma4 > 1.0) return false;

  // Step 5
  if(Gamma1 >= 0.0 and Gamma2 >= 0.0 and Gamma3 >= 0.0 and Gamma4 >= 0.0) return true;

  // Step 6 (test projection along X, Y, Z in turn.
  bool Intersect;
  Intersect = false;
  for(unsigned char i = 0; i < 4; i++) Intersect = Intersect or TestIntersection(t.ProjectX(i), b.ProjectX());
  if(not Intersect) return false;
  Intersect = false;
  for(unsigned char i = 0; i < 4; i++) Intersect = Intersect or TestIntersection(t.ProjectY(i), b.ProjectY());
  if(not Intersect) return false;
  Intersect = false;
  for(unsigned char i = 0; i < 4; i++) Intersect = Intersect or TestIntersection(t.ProjectZ(i), b.ProjectZ());
  if(not Intersect) return false;

  // Step 7
  return true;
}

}; // end namespace


#endif /* BoxTetIntersection_hh */
