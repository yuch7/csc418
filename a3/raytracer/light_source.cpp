/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

void PointLight::shade( Ray3D& ray ) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.
	// surface normal
	Vector3D normal = ray.intersection.normal;
	normal.normalize();

	// light direction
	Vector3D l = _pos - ray.intersection.point;
	l.normalize();

	// direction of intersection to eye
	Vector3D v = -ray.dir;
	v.normalize();

	// reflection dir
	Vector3D r = 2 * (l.dot(normal) * normal) - l;
	r.normalize();

	//calculate colors
	Colour ai = (*ray.intersection.mat).ambient * _col_ambient;
	Colour di = (*ray.intersection.mat).diffuse * (fmax(0.0,normal.dot(l)) * _col_diffuse);
	Colour si = (*ray.intersection.mat).specular * (fmax(0.0,pow(v.dot(r),(*ray.intersection.mat).specular_exp)) * _col_specular);

	// phong shading add the intensities
	ray.col = ai + di + si;
	ray.col.clamp();

}
