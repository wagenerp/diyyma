
/** \file math.h
  * \author Peter Wagener
  * \brief Linear algebra package.
  *
  * All your headaches are cast away.
  */

#ifndef _DIYYMA_MATH_H
#define _DIYYMA_MATH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "diyyma/util.h"

#define MATRIX_COLUMN_FIRST 1

template<class T> struct Vector4;
template<class T> struct Vector3;
template<class T> struct Vector2;
template<class T> struct Matrix;
template<class T> struct Quaternion;

typedef Matrix<float> Matrixf;
typedef Vector4<float> Vector4f;
typedef Vector3<float> Vector3f;
typedef Vector2<float> Vector2f;
typedef Quaternion<float> Quaternionf;

typedef Matrix<double> Matrixd;
typedef Vector4<double> Vector4d;
typedef Vector3<double> Vector3d;
typedef Vector2<double> Vector2d;
typedef Quaternion<double> Quaterniond;

typedef Vector4<int> Vector4i;
typedef Vector3<int> Vector3i;
typedef Vector2<int> Vector2i;


#define PI 3.141592653589793238462643
#define DEGPERRAD (180.0/PI)
#define RADPERDEG (PI/180.0)
#define DEGTORAD(deg) ((deg)*(PI/180.0))
#define RADTODEG(rad) ((rad)*(180.0/PI))

float modf(float a, float b);
double modf(double a, double b);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"

template <class T> struct Matrix {
  public:
    #if MATRIX_COLUMN_FIRST
      T a11, a21, a31, a41,
        a12, a22, a32, a42,
        a13, a23, a33, a43,
        a14, a24, a34, a44;
    #else
      T a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44;
    #endif
  
  public:
    Matrix() :
      a11(1), a12(0), a13(0), a14(0), 
      a21(0), a22(1), a23(0), a24(0), 
      a31(0), a32(0), a33(1), a34(0), 
      a41(0), a42(0), a43(0), a44(1) { }
    Matrix(
      T a,T b,T c,T d,
      T e,T f,T g,T h,
      T i,T j,T k,T l,
      T m,T n,T o,T p) :
      a11(a),a12(b),a13(c),a14(d),
      a21(e),a22(f),a23(g),a24(h),
      a31(i),a32(j),a33(k),a34(l),
      a41(m),a42(n),a43(o),a44(p) { }
    
    void set(
      T a,T b,T c,T d,
      T e,T f,T g,T h,
      T i,T j,T k,T l,
      T m,T n,T o,T p) {
        a11=a;a12=b;a13=c;a14=d;
        a21=e;a22=f;a23=g;a24=h;
        a31=i;a32=j;a33=k;a34=l;
        a41=m;a42=n;a43=o;a44=p;
      }
    
    void setDiagonal(
      T a, T b, T c, T d) {
      a11=a;a12=0;a13=0;a14=0;
      a21=0;a22=b;a23=0;a24=0;
      a31=0;a32=0;a33=c;a34=0;
      a41=0;a42=0;a43=0;a44=d; 
    }
    
    void setIdentity() {
      a11=1;a12=0;a13=0;a14=0;
      a21=0;a22=1;a23=0;a24=0;
      a31=0;a32=0;a33=1;a34=0;
      a41=0;a42=0;a43=0;a44=1;
    }
    
    void setPerspective(T fov, T aspectInv, T zNear, T zFar) {
      T a=1.0/tan(fov*(0.5*PI/180.0));
      T b=1.0/(zNear-zFar);
      set(
        0              ,-a,0          ,0,
        0              ,0 ,a*aspectInv,0,
        -(zFar+zNear)*b,0 ,0          ,2*zFar*zNear*b,
        1              ,0 ,0          ,0);
    }
    
    static Matrix<T> Perspective(T fov, T aspectInv, T zNear, T zFar) {
      T a=1.0/tan(fov*(0.5*PI/180.0));
      T b=1.0/(zNear-zFar);
      return Matrix<T>(
        0              ,-a,0          ,0,
        0              ,0 ,a*aspectInv,0,
        -(zFar+zNear)*b,0 ,0          ,2*zFar*zNear*b,
        1              ,0 ,0          ,0);
    }
    
    void setOrthogonalProjection(T l, T r, T b, T t, T n, T f) {
      set(
        2.0/(r-l),0         ,0         ,-(r+l)/(r-l),
        0        , 2.0/(t-b),0         ,-(t+b)/(t-b),
        0        ,0         ,-2.0/(f-n),-(f+n)/(f-n),
        0        ,0         ,0         ,1           );
    }
    
    static Matrix<T> OrthogonalProjection(T l, T r, T b, T t, T n, T f) {
      return Matrix<T>(
        2.0/(r-l),0         ,0         ,-(r+l)/(r-l),
        0        , 2.0/(t-b),0         ,-(t+b)/(t-b),
        0        ,0         ,-2.0/(f-n),-(f+n)/(f-n),
        0        ,0         ,0         ,1           );
    }
    
    void setRotationX(float ang) { 
      float s=sin(ang),c=cos(ang);
      set(
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 1);
    }
    static Matrix<T> RotationX(float ang) { 
      float s=sin(ang),c=cos(ang);
      return Matrix<T>(
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 1);
    }
    void setRotationY(float ang) { 
      float s=sin(ang),c=cos(ang);
      set(
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1);
    }
    static Matrix<T> RotationY(float ang) { 
      float s=sin(ang),c=cos(ang);
      return Matrix<T>(
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1);
    }
    void setRotationZ(float ang) { 
      float s=sin(ang),c=cos(ang);
      set(
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1);
    }
    static Matrix<T> RotationZ(float ang) { 
      float s=sin(ang),c=cos(ang);
      return Matrix<T>(
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1);
    }
    
    void setRotation(float ang, const Vector3<T> &axis) {
      Vector3<T> axis_u=axis.normal();
      float cs=cos(ang),sn=sin(ang);
      float x=axis_u.x, y=axis_u.y, z=axis_u.z;
      float cn=1-cs;
      set(
        cn*x*x+  cs, cn*x*y-z*sn, cn*x*z+y*sn, 0,
        cn*x*y+z*sn, cn*y*y+  cs, cn*y*z-x*sn, 0,
        cn*x*z-y*sn, cn*y*z+x*sn, cn*z*z+  cs, 0,
        0          , 0          , 0          , 1);
      
    }
    
    static Matrix<T> Rotation(T ang, const Vector3<T> &axis) {
      Vector3<T> axis_u=axis.normal();
      T cs=cos(ang),sn=sin(ang);
      T x=axis_u.x, y=axis_u.y, z=axis_u.z;
      T cn=1-cs;
      return Matrix<T>(
        cn*x*x+  cs, cn*x*y-z*sn, cn*x*z+y*sn, 0,
        cn*x*y+z*sn, cn*y*y+  cs, cn*y*z-x*sn, 0,
        cn*x*z-y*sn, cn*y*z+x*sn, cn*z*z+  cs, 0,
        0          , 0          , 0          , 1);
      
    }
    
    void setTranslation(T x, T y, T z) {
      set(
        1,0,0,x,
        0,1,0,y,
        0,0,1,z,
        0,0,0,1);
    }
    static Matrix<T> Translation(T x, T y, T z) {
      return Matrix<T>(
        1,0,0,x,
        0,1,0,y,
        0,0,1,z,
        0,0,0,1);
    }
    
    Matrix<T> operator+(const Matrix<T> &m) const {
      Matrix<T> r;
      r.a11=a11+m.a11;r.a12=a12+m.a12;r.a13=a13+m.a13;r.a14=a14+m.a14;
      r.a21=a21+m.a21;r.a22=a22+m.a22;r.a23=a23+m.a23;r.a24=a24+m.a24;
      r.a31=a31+m.a31;r.a32=a32+m.a32;r.a33=a33+m.a33;r.a34=a34+m.a34;
      r.a41=a41+m.a41;r.a42=a42+m.a42;r.a43=a43+m.a43;r.a44=a44+m.a44;
      return r;
    }
    
    void operator+=(const Matrix<T> &m) {
      a11+=m.a11;a12+=m.a12;a13+=m.a13;a14+=m.a14;
      a21+=m.a21;a22+=m.a22;a23+=m.a23;a24+=m.a24;
      a31+=m.a31;a32+=m.a32;a33+=m.a33;a34+=m.a34;
      a41+=m.a41;a42+=m.a42;a43+=m.a43;a44+=m.a44;
    }
    
    Matrix<T> operator-(const Matrix<T> &m) const {
      Matrix<T> r;
      r.a11=a11-m.a11;r.a12=a12-m.a12;r.a13=a13-m.a13;r.a14=a14-m.a14;
      r.a21=a21-m.a21;r.a22=a22-m.a22;r.a23=a23-m.a23;r.a24=a24-m.a24;
      r.a31=a31-m.a31;r.a32=a32-m.a32;r.a33=a33-m.a33;r.a34=a34-m.a34;
      r.a41=a41-m.a41;r.a42=a42-m.a42;r.a43=a43-m.a43;r.a44=a44-m.a44;
      return r;
    }
    
    void operator-=(const Matrix<T> &m) {
      a11-=m.a11;a12-=m.a12;a13-=m.a13;a14-=m.a14;
      a21-=m.a21;a22-=m.a22;a23-=m.a23;a24-=m.a24;
      a31-=m.a31;a32-=m.a32;a33-=m.a33;a34-=m.a34;
      a41-=m.a41;a42-=m.a42;a43-=m.a43;a44-=m.a44;
    }
    
    Matrix<T> operator-() {
      Matrix<T> r;
      r.a11=-a11;r.a12=-a12;r.a13=-a13;r.a14=-a14;
      r.a21=-a21;r.a22=-a22;r.a23=-a23;r.a24=-a24;
      r.a31=-a31;r.a32=-a32;r.a33=-a33;r.a34=-a34;
      r.a41=-a41;r.a42=-a42;r.a43=-a43;r.a44=-a44;
      return r;
    }
    
    
    Matrix<T> operator*(T f) const {
      Matrix<T> r;
      r.a11=a11*f;r.a12=a12*f;r.a13=a13*f;r.a14=a14*f;
      r.a21=a21*f;r.a22=a22*f;r.a23=a23*f;r.a24=a24*f;
      r.a31=a31*f;r.a32=a32*f;r.a33=a33*f;r.a34=a34*f;
      r.a41=a41*f;r.a42=a42*f;r.a43=a43*f;r.a44=a44*f;
      return r;
    }
    
    Matrix<T> operator*=(T f) {
      Matrix<T> r;
      a11*=f;a12*=f;a13*=f;a14*=f;
      a21*=f;a22*=f;a23*=f;a24*=f;
      a31*=f;a32*=f;a33*=f;a34*=f;
      a41*=f;a42*=f;a43*=f;a44*=f;
      
      return r;
    }
    
    Vector4<T> operator*(const Vector4<T> &v) const {
      Vector4<T> r;
      r.x=a11*v.x+a12*v.y+a13*v.z+a14*v.w; 
      r.y=a21*v.x+a22*v.y+a23*v.z+a24*v.w; 
      r.z=a31*v.x+a32*v.y+a33*v.z+a34*v.w; 
      r.w=a41*v.x+a42*v.y+a43*v.z+a44*v.w; 
      return r;
    }
    
    // assume (x,y,z,1) coordinates here, useful for transformation
    Vector3<T> operator*(const Vector3<T> &v) const {
      Vector3<T> r;
      r.x=a11*v.x+a12*v.y+a13*v.z+a14; 
      r.y=a21*v.x+a22*v.y+a23*v.z+a24; 
      r.z=a31*v.x+a32*v.y+a33*v.z+a34; 
      return r;
    }
    
    Matrix<T> operator*(const Matrix<T> &m) const {
      Matrix<T> r;
      r.a11=a11*m.a11+a12*m.a21+a13*m.a31+a14*m.a41;
      r.a12=a11*m.a12+a12*m.a22+a13*m.a32+a14*m.a42;
      r.a13=a11*m.a13+a12*m.a23+a13*m.a33+a14*m.a43;
      r.a14=a11*m.a14+a12*m.a24+a13*m.a34+a14*m.a44;
      
      r.a21=a21*m.a11+a22*m.a21+a23*m.a31+a24*m.a41;
      r.a22=a21*m.a12+a22*m.a22+a23*m.a32+a24*m.a42;
      r.a23=a21*m.a13+a22*m.a23+a23*m.a33+a24*m.a43;
      r.a24=a21*m.a14+a22*m.a24+a23*m.a34+a24*m.a44;
      
      r.a31=a31*m.a11+a32*m.a21+a33*m.a31+a34*m.a41;
      r.a32=a31*m.a12+a32*m.a22+a33*m.a32+a34*m.a42;
      r.a33=a31*m.a13+a32*m.a23+a33*m.a33+a34*m.a43;
      r.a34=a31*m.a14+a32*m.a24+a33*m.a34+a34*m.a44;
      
      r.a41=a41*m.a11+a42*m.a21+a43*m.a31+a44*m.a41;
      r.a42=a41*m.a12+a42*m.a22+a43*m.a32+a44*m.a42;
      r.a43=a41*m.a13+a42*m.a23+a43*m.a33+a44*m.a43;
      r.a44=a41*m.a14+a42*m.a24+a43*m.a34+a44*m.a44;
  
      return r;
    }
    
    void operator*=(const Matrix<T> &m) {
      set(
        a11*m.a11+a12*m.a21+a13*m.a31+a14*m.a41,
        a11*m.a12+a12*m.a22+a13*m.a32+a14*m.a42,
        a11*m.a13+a12*m.a23+a13*m.a33+a14*m.a43,
        a11*m.a14+a12*m.a24+a13*m.a34+a14*m.a44,
        
        a21*m.a11+a22*m.a21+a23*m.a31+a24*m.a41,
        a21*m.a12+a22*m.a22+a23*m.a32+a24*m.a42,
        a21*m.a13+a22*m.a23+a23*m.a33+a24*m.a43,
        a21*m.a14+a22*m.a24+a23*m.a34+a24*m.a44,
        
        a31*m.a11+a32*m.a21+a33*m.a31+a34*m.a41,
        a31*m.a12+a32*m.a22+a33*m.a32+a34*m.a42,
        a31*m.a13+a32*m.a23+a33*m.a33+a34*m.a43,
        a31*m.a14+a32*m.a24+a33*m.a34+a34*m.a44,
        
        a41*m.a11+a42*m.a21+a43*m.a31+a44*m.a41,
        a41*m.a12+a42*m.a22+a43*m.a32+a44*m.a42,
        a41*m.a13+a42*m.a23+a43*m.a33+a44*m.a43,
        a41*m.a14+a42*m.a24+a43*m.a34+a44*m.a44);
    }
    
    
    Matrix<T> operator/(T f) const {
      Matrix<T> r;
      r.a11=a11/f;r.a12=a12/f;r.a13=a13/f;r.a14=a14/f;
      r.a21=a21/f;r.a22=a22/f;r.a23=a23/f;r.a24=a24/f;
      r.a31=a31/f;r.a32=a32/f;r.a33=a33/f;r.a34=a34/f;
      r.a41=a41/f;r.a42=a42/f;r.a43=a43/f;r.a44=a44/f;
      return r;
    }
    
    Matrix<T> operator/=(T f) {
      Matrix<T> r;
      a11/=f;a12/=f;a13/=f;a14/=f;
      a21/=f;a22/=f;a23/=f;a24/=f;
      a31/=f;a32/=f;a33/=f;a34/=f;
      a41/=f;a42/=f;a43/=f;a44/=f;
      
      return r;
    }
    
    Matrix<T> transposed() const {
      return Matrix<T>(
        a11, a21, a31, a41,
        a12, a22, a32, a42,
        a13, a23, a33, a43,
        a14, a24, a34, a44);
    }
    
    void transpose() {
      set(
        a11, a21, a31, a41,
        a12, a22, a32, a42,
        a13, a23, a33, a43,
        a14, a24, a34, a44);
    }
    
    T det() const {
      return 
        a14*a23*a32*a41 - a13*a24*a32*a41 - a14*a22*a33*a41 + a12*a24*a33*a41 + 
        a13*a22*a34*a41 - a12*a23*a34*a41 - a14*a23*a31*a42 + a13*a24*a31*a42 + 
        a14*a21*a33*a42 - a11*a24*a33*a42 - a13*a21*a34*a42 + a11*a23*a34*a42 + 
        a14*a22*a31*a43 - a12*a24*a31*a43 - a14*a21*a32*a43 + a11*a24*a32*a43 + 
        a12*a21*a34*a43 - a11*a22*a34*a43 - a13*a22*a31*a44 + a12*a23*a31*a44 + 
        a13*a21*a32*a44 - a11*a23*a32*a44 - a12*a21*a33*a44 + a11*a22*a33*a44;
    }
    
    T tr() const {
      return a11+a12+a13+a14;
    }
    
    Matrix<T> inverse() const {
      Matrix<T> r; 
      T d = det();
      // If a matrix is not invertible, a wrong result is favorable to -inf.
      if (d==0) d=1; else d=1.0/d;
      
      r.a11=d*(a23*a34*a42-a24*a33*a42+a24*a32*a43-a22*a34*a43-a23*a32*a44+a22*a33*a44);
      r.a12=d*(a14*a33*a42-a13*a34*a42-a14*a32*a43+a12*a34*a43+a13*a32*a44-a12*a33*a44);
      r.a13=d*(a13*a24*a42-a14*a23*a42+a14*a22*a43-a12*a24*a43-a13*a22*a44+a12*a23*a44);
      r.a14=d*(a14*a23*a32-a13*a24*a32-a14*a22*a33+a12*a24*a33+a13*a22*a34-a12*a23*a34);
      r.a21=d*(a24*a33*a41-a23*a34*a41-a24*a31*a43+a21*a34*a43+a23*a31*a44-a21*a33*a44);
      r.a22=d*(a13*a34*a41-a14*a33*a41+a14*a31*a43-a11*a34*a43-a13*a31*a44+a11*a33*a44);
      r.a23=d*(a14*a23*a41-a13*a24*a41-a14*a21*a43+a11*a24*a43+a13*a21*a44-a11*a23*a44);
      r.a24=d*(a13*a24*a31-a14*a23*a31+a14*a21*a33-a11*a24*a33-a13*a21*a34+a11*a23*a34);
      r.a31=d*(a22*a34*a41-a24*a32*a41+a24*a31*a42-a21*a34*a42-a22*a31*a44+a21*a32*a44);
      r.a32=d*(a14*a32*a41-a12*a34*a41-a14*a31*a42+a11*a34*a42+a12*a31*a44-a11*a32*a44);
      r.a33=d*(a12*a24*a41-a14*a22*a41+a14*a21*a42-a11*a24*a42-a12*a21*a44+a11*a22*a44);
      r.a34=d*(a14*a22*a31-a12*a24*a31-a14*a21*a32+a11*a24*a32+a12*a21*a34-a11*a22*a34);
      r.a41=d*(a23*a32*a41-a22*a33*a41-a23*a31*a42+a21*a33*a42+a22*a31*a43-a21*a32*a43);
      r.a42=d*(a12*a33*a41-a13*a32*a41+a13*a31*a42-a11*a33*a42-a12*a31*a43+a11*a32*a43);
      r.a43=d*(a13*a22*a41-a12*a23*a41-a13*a21*a42+a11*a23*a42+a12*a21*a43-a11*a22*a43);
      r.a44=d*(a12*a23*a31-a13*a22*a31+a13*a21*a32-a11*a23*a32-a12*a21*a33+a11*a22*a33);
      return r;
    }
    
    void invert() {
      T d = det(); 
      if (d==0) return;
      d=1.0/d;
      set(
        d*(a23*a34*a42-a24*a33*a42+a24*a32*a43-a22*a34*a43-a23*a32*a44+a22*a33*a44),
        d*(a14*a33*a42-a13*a34*a42-a14*a32*a43+a12*a34*a43+a13*a32*a44-a12*a33*a44),
        d*(a13*a24*a42-a14*a23*a42+a14*a22*a43-a12*a24*a43-a13*a22*a44+a12*a23*a44),
        d*(a14*a23*a32-a13*a24*a32-a14*a22*a33+a12*a24*a33+a13*a22*a34-a12*a23*a34),
        d*(a24*a33*a41-a23*a34*a41-a24*a31*a43+a21*a34*a43+a23*a31*a44-a21*a33*a44),
        d*(a13*a34*a41-a14*a33*a41+a14*a31*a43-a11*a34*a43-a13*a31*a44+a11*a33*a44),
        d*(a14*a23*a41-a13*a24*a41-a14*a21*a43+a11*a24*a43+a13*a21*a44-a11*a23*a44),
        d*(a13*a24*a31-a14*a23*a31+a14*a21*a33-a11*a24*a33-a13*a21*a34+a11*a23*a34),
        d*(a22*a34*a41-a24*a32*a41+a24*a31*a42-a21*a34*a42-a22*a31*a44+a21*a32*a44),
        d*(a14*a32*a41-a12*a34*a41-a14*a31*a42+a11*a34*a42+a12*a31*a44-a11*a32*a44),
        d*(a12*a24*a41-a14*a22*a41+a14*a21*a42-a11*a24*a42-a12*a21*a44+a11*a22*a44),
        d*(a14*a22*a31-a12*a24*a31-a14*a21*a32+a11*a24*a32+a12*a21*a34-a11*a22*a34),
        d*(a23*a32*a41-a22*a33*a41-a23*a31*a42+a21*a33*a42+a22*a31*a43-a21*a32*a43),
        d*(a12*a33*a41-a13*a32*a41+a13*a31*a42-a11*a33*a42-a12*a31*a43+a11*a32*a43),
        d*(a13*a22*a41-a12*a23*a41-a13*a21*a42+a11*a23*a42+a12*a21*a43-a11*a22*a43),
        d*(a12*a23*a31-a13*a22*a31+a13*a21*a32-a11*a23*a32-a12*a21*a33+a11*a22*a33));
        
    }
    
    Vector4<T> col1() const { return Vector4<T>(a11,a21,a31,a41); }
    Vector4<T> col2() const { return Vector4<T>(a12,a22,a32,a42); }
    Vector4<T> col3() const { return Vector4<T>(a13,a23,a33,a43); }
    Vector4<T> col4() const { return Vector4<T>(a14,a24,a34,a44); }
    
    Vector4<T> row1() const { return Vector4<T>(a11,a12,a13,a14); }
    Vector4<T> row2() const { return Vector4<T>(a21,a22,a23,a24); }
    Vector4<T> row3() const { return Vector4<T>(a31,a32,a33,a34); }
    Vector4<T> row4() const { return Vector4<T>(a41,a42,a43,a44); }
    
    void print() {
      printf(
        "%g %g %g %g\n"
        "%g %g %g %g\n"
        "%g %g %g %g\n"
        "%g %g %g %g\n",
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44);
        
    }
    
};

template<class T> struct Vector4 {
  public:
    T x,y,z,w;
  public:
    Vector4(): x(0), y(0), z(0), w(0) { }
    Vector4(T a, T b, T c, T d):
      x(a),y(b),z(c),w(d) { }
    
    Vector4(const Vector2<T> &v, T c, T d) :
      x(v.x), y(v.y), z(c), w(d) { }
    
    Vector4(const Vector3<T> &v, T d) :
      x(v.x), y(v.y), z(v.z), w(d) { }
    
    
    void set(T a, T b, T c, T d) {
      x=a; y=b; z=c; w=d;
    }
    
    Vector4<T> operator+(const Vector4<T> &v) const {
      return Vector4<T>(x+v.x,y+v.y,z+v.z,w+v.w);
    }
    
    void operator+=(const Vector4<T> &v) {
      set(x+v.x,y+v.y,z+v.z,w+v.w);
    }
    
    Vector4<T> operator-(const Vector4<T> &v) const {
      return Vector4<T>(x-v.x,y-v.y,z-v.z,w-v.w);
    }
    Vector4<T> operator-() {
      return Vector4<T>(-x,-y,-z,-w);
    }
    
    void operator-=(const Vector4<T> &v) {
      set(x-v.x,y-v.y,z-v.z,w-v.w);
    }
    
    Vector4<T> operator*(T f) const {
      return Vector4(x*f,y*f,z*f,w*f);
    }
    void operator*=(T f) {
      set(x*f,y*f,z*f,w*f);
    }
    Vector4<T> operator/(T f) const {
      return Vector4<T>(x/f,y/f,z/f,w/f);
    }
    void operator/=(T f) {
      set(x/f,y/f,z/f,w/f);
    }
    
    T operator*(const Vector4<T> &v) const {
      return x*v.x+y*v.y+z*v.z+w*v.w;
    }
    
    T length() const {
      return (T)sqrt(x*x+y*y+z*z+w*w);
    }
    void length(T l) {
      this*=l/length;
    }
    T sqr() const {
      return x*x+y*y+z*z+w*w;
    }
    
    Vector4<T> normal() const {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      return Vector4(x*f,y*f,z*f,w*f);
    }
    void normalize() {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      set(x*f,y*f,z*f,w*f);
    }
    
    void print() const {
      printf("%g %g %g %g\n",x,y,z,w);
    }
    
};

static Vector4f operator*(float f, const Vector4f &v) { return v*f; }
static Vector4d operator*(double f, const Vector4d &v) { return v*f; }

template<class T> struct Vector3 {
  public:
    T x,y,z;
  public:
    Vector3(): x(0), y(0), z(0) { }
    Vector3(T a, T b, T c):
      x(a),y(b),z(c) { }
    
    /** \brief Copy constructor which cuts of last element 
      * in 4 dimensional vector
      */
    Vector3(const Vector4<T> &v) : 
      x(v.x), y(v.y), z(v.z) { } 
    
    Vector3(const Vector2<T> &v, T z) : 
      x(v.x), y(v.y), z(z) { }
    
    void set(T a, T b, T c) {
      x=a; y=b; z=c;
    }
    
    Vector3<T> operator+(const Vector3<T> &v) const {
      return Vector3<T>(x+v.x,y+v.y,z+v.z);
    }
    
    void operator+=(const Vector3<T> &v) {
      set(x+v.x,y+v.y,z+v.z);
    }
    
    Vector3<T> operator-(const Vector3<T> &v) const {
      return Vector3<T>(x-v.x,y-v.y,z-v.z);
    }
    Vector3<T> operator-() {
      return Vector3<T>(-x,-y,-z);
    }
    
    void operator-=(const Vector3<T> &v) {
      set(x-v.x,y-v.y,z-v.z);
    }
    
    Vector3<T> operator*(T f) const {
      return Vector3(x*f,y*f,z*f);
    }
    void operator*=(T f) {
      set(x*f,y*f,z*f);
    }
    Vector3<T> operator/(T f) const {
      return Vector3<T>(x/f,y/f,z/f);
    }
    void operator/=(T f) {
      set(x/f,y/f,z/f);
    }
    T operator*(const Vector3<T> &v) const {
      return x*v.x+y*v.y+z*v.z;
    }
    
    /** \brief cross-product */
    Vector3<T> operator%(const Vector3<T> &v) const {
      return Vector3(
        y*v.z-z*v.y,
        z*v.x-x*v.z,
        x*v.y-y*v.x
      );
    }
    
    void operator%=(const Vector3<T> &v) {
      set(
        y*v.z-z*v.y,
        z*v.x-x*v.z,
        x*v.y-y*v.x
      );
    }
    
    
    Vector3<T> operator^(const Vector3<T> &v) const {
      return Vector3(
        x*v.x,
        y*v.y,
        z*v.z
      );
    }
    
    void operator^=(const Vector3<T> &v) {
      set(
        x*v.x,
        y*v.y,
        z*v.z
      );
    }
    
    
    T length() const {
      return (T)sqrt(x*x+y*y+z*z);
    }
    void length(T l) {
      this*=l/length;
    }
    T sqr() const {
      return x*x+y*y+z*z;
    }
    
    Vector3<T> normal() const {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      return Vector3(x*f,y*f,z*f);
    }
    
    void normalize() {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      set(x*f,y*f,z*f);
    }
    
    void print() const {
      printf("%g %g %g\n",x,y,z);
    }
    
    
    Vector3<T> rgbToHsv() const {
      T a,b;
      if (x>y) {
        if      (y>z) { a=x-z; b=y-z; return Vector3<T>(60.0*(  b/a),1-z/x,x); }
        else if (z>x) { a=z-y; b=x-y; return Vector3<T>(60.0*(4+b/a),1-y/z,z); }
        else          { a=x-y; b=z-y; return Vector3<T>(60.0*(6-b/a),1-y/x,x); }
      } else { 
        if      (x>z) { a=y-z; b=x-z; return Vector3<T>(60.0*(2-b/a),1-z/y,z); }
        else if (y>z) { a=z-x; b=y-x; return Vector3<T>(60.0*(4-b/a),1-x/z,y); }
        else if ((x==y) && (y==z))    return Vector3<T>(0,0,0);
        else          { a=y-x; b=z-x; return Vector3<T>(60.0*(2+b/a),1-x/y,y); }
      }
    }
    
    Vector3<T> hsvToRgb() const {
      T c=z*y;
      T m=z-c;
      T h1=modf(x,360);
      T a=c*(1-fabs(modf((h1/60),2)-1));
      c+=m; a+=m;
      
      if      (h1< 60) return Vector3<T>(c,a,m);
      else if (h1<120) return Vector3<T>(a,c,m);
      else if (h1<180) return Vector3<T>(m,c,a);
      else if (h1<240) return Vector3<T>(m,a,c);
      else if (h1<300) return Vector3<T>(a,m,c);
      else             return Vector3<T>(c,m,a);
    }
    
    static Vector3<T> RGBFromHSV(T h, T s, T v) {
      T c=v*s, m=v-c;
      T h1=modf(h,360);
      T x=c*(1.0-fabs(modf((h1/60.0),2.0)-1.0));
      c+=m; x+=m;
      
      if      (h1< 60) return Vector3<T>(c,x,m);
      else if (h1<120) return Vector3<T>(x,c,m);
      else if (h1<180) return Vector3<T>(m,c,x);
      else if (h1<240) return Vector3<T>(m,x,c);
      else if (h1<300) return Vector3<T>(x,m,c);
      else             return Vector3<T>(c,m,x);
    }
    
    static Vector3<T> HSVFromRGB(T r, T g, T b) {
      T c,x;
      if (r>g) {
        if      (g>b) { c=r-b; x=g-b; return Vector3<T>(60.0*(  x/c),1-b/r,r); }
        else if (b>r) { c=b-g; x=r-g; return Vector3<T>(60.0*(4+x/c),1-g/b,b); }
        else          { c=r-g; x=b-g; return Vector3<T>(60.0*(6-x/c),1-g/r,r); }
      } else { 
        if      (r>b) { c=g-b; x=r-b; return Vector3<T>(60.0*(2-x/c),1-b/g,g); }
        else if (b>g) { c=b-r; x=g-r; return Vector3<T>(60.0*(4-x/c),1-r/b,b); }
        else if ((r==g) && (g==b))    return Vector3<T>(0,0,0);
        else          { c=g-r; x=b-r; return Vector3<T>(60.0*(2+x/c),1-r/g,g); }
      }
    }
    
    
    /** \brief Interpolates on a quadratic bezier curve.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control point p2.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> Bezier(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      T t) {
      T s=1-t;
      return s*(s*p0+t*p1) + t*(s*p1+t*p2);
    }
    
    /** \brief Interpolates on a cubic bezier curve.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control points p2 and p3.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> Bezier(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      const Vector3 &p3,
      T t) {
      
      T s=1-t;
      
      return 
        s*(s*(s*p0+t*p1) + t*(s*p1+t*p2))  +  t*(s*(s*p1+t*p2) + t*(s*p2+t*p3));
    }
    
    
    /** \brief Interpolates on a quadratic bezier curve, returning the velocity.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control point p2.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> BezierVelocity(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      T t) {
      return (2*(1-t))*(p1-p0)+(2*t)*(p2-p1);
    }
    
    /** \brief Interpolates on a cubic bezier curve, returning the velocity.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control points p2 and p3.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> BezierVelocity(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      const Vector3 &p3,
      T t) {
      
      T s=1-t;
      
      return (3*s*s)*(p1-p0)+(6*s*t)*(p2-p1)+(3*t*t)*(p3-p2);
    }
    
    /** \brief Interpolates on a quadratic bezier curve, returning the acceleration.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control point p2.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> BezierAcceleration(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      T t) {
      return 2*(p2-p1-p1+p0);
    }
    
    /** \brief Interpolates on a cubic bezier curve, returning the acceleration.
      * 
      * The curve goes from p0 to p1, using (but generally not touching)
      * the control points p2 and p3.
      * 
      * \param t Interpolation step. Must be within [0,1].
      */
    static Vector3<T> BezierAcceleration(
      const Vector3 &p0,
      const Vector3 &p1,
      const Vector3 &p2,
      const Vector3 &p3,
      T t) {
      
      T s=1-t;
      
      return (6*s)*(p2-p1-p1+p0) + (6*t)*(p3-p2-p2+p1);
    }
    
    /** \brief Returns a uniformly picked vector on the unit sphere.
      */
    
    static Vector3<T> UniformSphereSurface() {
      T t,ct,st, p,cp,sp;
      p=acos(randf()*2.0-1.0); cp=cos(p); sp=sin(p);
      t=randf()*(2.0*PI);      ct=cos(t); st=sin(t);
      
      return Vector3<T>(ct*sp,st*sp,cp);
      
    }
};

static Vector3f operator*(float f, const Vector3f &v) { return v*f; }
static Vector3d operator*(double f, const Vector3d &v) { return v*f; }


template<class T> struct Vector2 {
  public:
    T x,y;
  public:
    Vector2(): x(0), y(0) { }
    Vector2(T a, T b):
      x(a),y(b) { }
    /** \brief Copy constructor which cuts of last element 
      * in 4 dimensional vector
      */
    Vector2(const Vector4<T> &v) : 
      x(v.x), y(v.y) { }
    /** \brief Copy constructor which cuts of last element 
      * in 4 dimensional vector
      */
    Vector2(const Vector3<T> &v) : 
      x(v.x), y(v.y) { } 
    
    void set(T a, T b) {
      x=a; y=b;
    }
    
    Vector2<T> operator+(const Vector2<T> &v) const {
      return Vector2<T>(x+v.x,y+v.y);
    }
    
    void operator+=(const Vector2<T> &v) {
      set(x+v.x,y+v.y);
    }
    
    Vector2<T> operator-(const Vector2<T> &v) const {
      return Vector2<T>(x-v.x,y-v.y);
    }
    Vector2<T> operator-() {
      return Vector2<T>(-x,-y);
    }
    
    void operator-=(const Vector2<T> &v) {
      set(x-v.x,y-v.y);
    }
    
    Vector2<T> operator*(T f) const {
      return Vector2(x*f,y*f);
    }
    void operator*=(T f) {
      set(x*f,y*f);
    }
    Vector2<T> operator/(T f) const {
      return Vector2<T>(x/f,y/f);
    }
    void operator/=(T f) {
      set(x/f,y/f);
    }
    T operator*(const Vector2<T> &v) const {
      return x*v.x+y*v.y;
    }
    
    
    T length() const {
      return (T)sqrt(x*x+y*y);
    }
    void length(T l) {
      this*=l/length;
    }
    T sqr() const {
      return x*x+y*y;
    }
    
    Vector2<T> normal() const {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      return Vector2(x*f,y*f);
    }
    void normalize() {
      T f=length();
      if (f==0) f=1; else f=1.0/f;
      set(x*f,y*f);
    }
    
    void print() const {
      printf("%g %g\n",x,y);
    }

};
static Vector2f operator*(float f, const Vector2f &v) { return v*f; }
static Vector2d operator*(double f, const Vector2d &v) { return v*f; }

/** \brief Mirror point along plane
  * \param point Point which shall be mirrored
  * \param plane Plane along point shall be mirrored
  */
static Vector3f mirrorPoint(const Vector3f &point, const Vector4f &plane){
  return point - (2*(plane*Vector4f(point, 1.f))*(Vector3f(plane).normal()));
}

/** \brief Computes intersection point between line and plane
  * \param origin Starting point of the line
  * \param direction Direction the line is pointing
  * \param plane Plane which is tested for intersection
  * \return Returns the intersection point in 3D space. The fourth
  *         component may by -1 : Intersection point lies behind origin
  *                          +1 : Intersection point lies in front of origin
  *                           0 : No intersection
  */
static Vector4f intersectPlane(const Vector3f &origin, const Vector3f &direction,
	                       const Vector4f &plane){
  if(Vector3f(plane)*Vector3f(direction) == 0){
    //No intersection Point
    return Vector4f(0.f, 0.f, 0.f, 0.f);
  } else {
    float t = -1.f*(Vector4f(plane)*Vector4f(origin, 1.f))/
		           (Vector3f(plane)*Vector3f(direction));
    Vector4f intersectionPoint;

    intersectionPoint = Vector4f(origin+(t*direction), 1.f);
	if(t < 0.f){
		intersectionPoint.w = -1.f;
	}

	return intersectionPoint;
  }
}

template<class T> struct Quaternion {
  public:
    T r;     ///< \brief Real component.
    union {
      struct {
        T x,y,z; ///< \brief Complex components.
      };
      Vector3<T> v;
    };
  
  public:
    Quaternion(): r(0), x(0), y(0), z(0) { }
    Quaternion(T re): r(re), x(0), y(0), z(0) { }
    Quaternion(T re, T ix, T iy, T iz): r(re), x(ix), y(iy), z(iz) { }
    Quaternion(T re,const Vector3<T> &im) : r(re), x(im.x), y(im.y), z(im.z) { }
    Quaternion(const Vector4<T> &v) : r(v.w), x(v.x), y(v.y), z(v.z) { }
    
    void set(T re, T ix, T iy, T iz) {
      r=re; x=ix; y=iy; z=iz;
    }
    
    void set(T re) {
      r=re; x=0; y=0; z=0;
    }
    void set(T re, const Vector3<T> &im) {
      r=re; x=im.x; y=im.y; z=im.z;
    }
    void set(const Vector4<T> &v) {
      r=v.w; x=v.x; y=v.y; z=v.z;
    }
    
    Quaternion<T> &operator=(T re) {
      r=re; x=0; y=0; z=0;
      return *this;
    }
    
    static Quaternion<T> RotationX(T ang) {
      T c=cos(ang*0.5), s=sin(ang*0.5);
      return Quaternion<T>(c,s,0,0);
    }
    static Quaternion<T> RotationY(T ang) {
      T c=cos(ang*0.5), s=sin(ang*0.5);
      return Quaternion<T>(c,0,0,s);
    }
    static Quaternion<T> RotationZ(T ang) {
      T c=cos(ang*0.5), s=sin(ang*0.5);
      return Quaternion<T>(c,0,0,s);
    }
    
    static Quaternion<T> FromVectors(
      const Vector3<T> &from, const Vector3<T> &to) {
      
      Vector3f a=from.normal(),b=to.normal();
      Vector3f n=b%a;
      
      T s=asin(n.length())*0.5;
      T c=cos(s);
      s=sin(s);
      
      return Quaternion<T>(c,n*s);
    }
    
    static Quaternion<T> FromMatrix(const Matrix<T> &m) {
      T f=m.a11+m.a22+m.a33;
      Quaternion<T> r;
      
      if (f>0) {
        f=sqrt(f+1)*2;
        r.r=0.25*f; f=1.0/f;
        r.x=(m.a32-m.a23)*f;
        r.y=(m.a13-m.a31)*f;
        r.z=(m.a21-m.a12)*f;
      } else if ((m.a11>m.a22)&&(m.a11>m.a33)) {
        f=sqrt(1+m.a11-m.a22-m.a33)*2;
        r.x=0.25*f; f=1.0/f;
        r.r=(m.a32-m.a23)*f;
        r.y=(m.a12-m.a21)*f;
        r.z=(m.a13-m.a31)*f;
      } else if (m.a22>m.a33) {
        f=sqrt(1+m.a22-m.a11-m.a33)*2;
        r.y=0.25*f; f=1.0/f;
        r.r=(m.a13-m.a31)*f;
        r.x=(m.a12-m.a21)*f;
        r.z=(m.a23-m.a32)*f;
      } else {
        f=sqrt(1+m.a33-m.a11-m.a22)*2;
        r.z=0.25*f; f=1.0/f;
        r.r=(m.a21-m.a12)*f;
        r.x=(m.a13-m.a31)*f;
        r.y=(m.a23-m.a32)*f;
      }
      
      return r;
    }
    
    Quaternion<T> operator-() const { return Quaternion<T>(-r,-x,-y,-z); }
    
    Quaternion<T> operator+(const Quaternion<T> &q) const {
      return Quaternion<T>(r+q.r,x+q.x,y+q.y,z+q.z);
    }
    
    Quaternion<T> operator+(T re) const { 
      return Quaternion<T>(r+re,x,y,z); 
    }
    
    Quaternion<T> operator+(const Vector3<T> &v) const {
      return Quaternion<T>(r,x+v.x,y+v.y,z+v.z);
    }
    
    Quaternion<T> operator-(const Quaternion<T> &q) const {
      return Quaternion<T>(r-q.r,x-q.x,y-q.y,z-q.z);
    }
    
    Quaternion<T> operator-(T re) const { 
      return Quaternion<T>(r-re,x,y,z); 
    }
    
    Quaternion<T> operator-(const Vector3<T> &v) const {
      return Quaternion<T>(r,x-v.x,y-v.y,z-v.z);
    }
    
    
    void operator+=(const Quaternion<T> &q) {
      r+=q.r; x+=q.x; y+=q.y; z+=q.z;
    }
    
    void operator+=(T re) { 
      r+=re;
    }
    
    void operator+=(const Vector3<T> &v) {
      x+=v.x; y+=v.y; z+=v.z;
    }
    
    void operator-=(const Quaternion<T> &q) {
      r-=q.r; x-=q.x; y-=q.y; z-=q.z;
    }
    
    void operator-=(T re) { 
      r-=re;
    }
    
    void operator-=(const Vector3<T> &v) {
      x-=v.x; y-=v.y; z-=v.z;
    }
    
    /** \brief Grassman Product. */
    Quaternion<T> operator%(const Quaternion<T> &q) const {
      return Quaternion<T>(r*q.r-v*q.v,q.r*v+r*q.v+v%q.v);
    }
    
    /** \brief Dot Product. */
    T operator*(const Quaternion<T> &q) const {
      return r*q.r+x*q.x+y*q.y+z*q.z;
    }
    
    Quaternion<T> operator*(const Vector3<T> &q) const {
      return Quaternion<T>(-v*q,r*q+v%q);
    }
    
    Quaternion<T> operator*(T f) const {
      return Quaternion<T>(r*f,x*f,y*f,z*f);
    }
    
    Quaternion<T> operator/(T f) const {
      return Quaternion<T>(r/f,x/f,y/f,z/f);
    }
    
    /** \brief Grassman Product. */
    void operator%=(const Quaternion<T> &q) {
      set(r*q.r-v*q.v,q.r*v+r*q.v+v%q.v);
    }
    
    void operator*=(const Vector3<T> &q) {
      set(-v*q,r*q+v%q);
    }
    
    void operator*=(T f) {
      r*=f; x*=f; y*=f; z*=f;
    }
    void operator/=(T f) {
      r/=f; x/=f; y/=f; z/=f;
    }
    
    Quaternion<T> conjugated() const {
      return Quaternion<T>(r,-x,-y,-z);
    }
    
    void conjugate() { 
      x=-x; y=-y; z=-z;
    }
    
};

template<class T> Quaternion<T> slerp(
  const Quaternion<T> &q0,
  const Quaternion<T> &q1_in,
  T t) {
  
  T a = q0*q1_in;
  Quaternion<T> q1;
  
  if (a<0) {
    a=-a;
    q1=-q1_in;
  } else
    q1=q1_in;
  
  
  a=acos(a);
  
  return (q0*sin(a*(1-t)) + q1*sin(a*t))/sin(a);
}

template<class T> Quaternion<T> lerp(
  const Quaternion<T> &q0,
  const Quaternion<T> &q1,
  T t) {
  
  T a = q0*q1;
  if (a<0) return q0*(1-t)-q1*t;
  else     return q0*(1-t)+q1*t;
}

// ignored -Wreorder
#pragma GCC diagnostic pop

#endif