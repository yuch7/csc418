/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSquare, which is
	// defined on the xy-plane, with vertices (0.5, 0.5, 0), 
	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
	// (0, 0, 1).
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.

	Ray3D r;
	r.origin = worldToModel * ray.origin;
	r.dir = worldToModel * ray.dir;

	double t = -r.origin[2] / r.dir[2];
	if (t <= 0) return false;

	double xint = r.origin[0] + t * r.dir[0];
	double yint = r.origin[1] + t * r.dir[1];

	if ((xint >= -0.5) && (xint <= 0.5) && (yint >= -0.5) && (yint <= 0.5)){
		if (ray.intersection.none || t < ray.intersection.t_value) {

			Vector3D n(0,0,1);
			Point3D i(xint, yint, 0.0);

			ray.intersection.t_value = t;
			ray.intersection.point = modelToWorld * i;
			ray.intersection.normal = worldToModel.transpose() * n;
			ray.intersection.none = false;
			return true;

		}
	}	

	return false;
}


bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSphere, which is centred 
	// on the origin.  
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.
	
	Ray3D r;
	r.origin = worldToModel * ray.origin;
	r.dir = worldToModel * ray.dir;

	Point3D sphere(0,0,0);
	Vector3D s_dist = r.origin - sphere;
	double int0 = -1, int1;

	double a = r.dir.dot(r.dir);
	double b = s_dist.dot(r.dir);
	double c = s_dist.dot(s_dist) - 1;
	double d = b * b - a * c;

	if (d == 0) int0 = -b/a;
	else if (d > 0) {
		int0 = (-b + sqrt(d)) / a;
		int1 = (-b - sqrt(d)) / a;
		int0 = int0 < int1 ? int0 : int1;
	}


	if (int0 <= 0) return false;

	if (ray.intersection.none || int0 < ray.intersection.t_value) {

		Point3D ip = r.origin + int0 * r.dir;
		Vector3D n = 2 * (ip - sphere);
		n.normalize();

		ray.intersection.t_value = int0;
		ray.intersection.point = modelToWorld * ip;
		ray.intersection.normal = transNorm(worldToModel, n);
		ray.intersection.normal.normalize();
		ray.intersection.none = false;
		return true;
	}

	return false;
}


bool UnitCylinder::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	Ray3D r;
	r.origin = worldToModel * ray.origin;
	r.dir = worldToModel * ray.dir;

	Point3D cylinder(0,0,0);

	double int0, int1, int2, int3, t, t1;
	double a = r.dir[0] * r.dir[0] + r.dir[1] * r.dir[1];
	double b = r.origin[0] * r.dir[0] + r.origin[1] * r.dir[1];
	double c = r.origin[0] * r.origin[0] + r.origin[1] * r.origin[1] - 1;
	double d = b * b - a * c;

	if (d < 0) return false;

	int0 = (-b + sqrt(d)) / a;
	int1 = (-b - sqrt(d)) / a;
	if (int0 < 0 && int1 < 0) return false;
	else if (int0 > 0 && int1 < 0) t = int0;
	else t = int1;

	int2 = ( -0.5 - r.origin[2])/r.dir[2];
	int3 = ( 0.5 - r.origin[2])/r.dir[2];

	t1 = int2 < int3 ? int2 : int3;
	if (t1 * t1 < 0.001) return false;
	if (t * t < 0.001) return false;
	Point3D n0(0,0,1);
	Point3D n1(0,0,-1);
	Vector3D tn0;
	if (int2 < int3) tn0 = n1 - cylinder;
	else tn0 = n0 - cylinder;
	tn0.normalize();

	Point3D pint = r.origin + t1 * r.dir;

	if (pint[0]* pint[0] + pint[1] * pint[1] <= 1) {
		
		if (ray.intersection.none || ray.intersection.t_value > int3) {

			ray.intersection.t_value = t1;
			ray.intersection.point = pint;
			ray.intersection.none = false;
			ray.intersection.normal = tn0;
			return true;
		} 
	}

	pint = r.origin + t * r.dir;

	Vector3D tn1(pint[0], pint[1], 0);
	tn1.normalize();

	if (pint[2] < 0.5 && pint[2] > -0.5) {

		if (ray.intersection.none || ray.intersection.t_value > int3) {

			ray.intersection.t_value = t;
			ray.intersection.point = modelToWorld * pint;
			ray.intersection.none = false;
			ray.intersection.normal = modelToWorld * tn1;
			return true;

		} 	
	}

	return false;

	
}