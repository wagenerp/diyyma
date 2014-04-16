
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

#include "GL/glew.h"

#include "diyyma/util.h"

#define MATRIX_COLUMN_FIRST 1

template<class T> struct Vector4;
template<class T> struct Vector3;
template<class T> struct Vector2;
template <class T> struct Matrix;

typedef Matrix<float> Matrixf;
typedef Vector4<float> Vector4f;
typedef Vector3<float> Vector3f;
typedef Vector2<float> Vector2f;

#define PI 3.141592653589793238462643

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
      set(x/f,y/f,z/f,w/f);
    }
    void operator/=(T f) {
      set(x/f,y/f,z/f,w/f);
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
      T f=1.0/length;
      return Vector4(x*f,y*f,z*f,w*f);
    }
    void normalize() {
      T f=1.0/length;
      set(x*f,y*f,z*f,w*f);
    }
    
    void print() const {
      printf("%g %g %g %g\n",x,y,z,w);
    }
    
};

template<class T> struct Vector3 {
  public:
    T x,y,z;
  public:
    Vector3(): x(0), y(0), z(0) { }
    Vector3(T a, T b, T c):
      x(a),y(b),z(c) { }
    
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
      set(x/f,y/f,z/f);
    }
    void operator/=(T f) {
      set(x/f,y/f,z/f);
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
      T f=1.0/length;
      return Vector3(x*f,y*f,z*f);
    }
    void normalize() {
      T f=1.0/length;
      set(x*f,y*f,z*f);
    }
    
    void print() const {
      printf("%g %g %g\n",x,y,z);
    }

};


template<class T> struct Vector2 {
  public:
    T x,y;
  public:
    Vector2(): x(0), y(0) { }
    Vector2(T a, T b):
      x(a),y(b) { }
    
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
      set(x/f,y/f);
    }
    void operator/=(T f) {
      set(x/f,y/f);
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
      T f=1.0/length;
      return Vector2(x*f,y*f);
    }
    void normalize() {
      T f=1.0/length;
      set(x*f,y*f);
    }
    
    void print() const {
      printf("%g %g\n",x,y);
    }

};

#endif