/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		Implementations of functions in raytracer.h

***********************************************************/


#include "raytracer.h"
#include "bmp_io.h"
#include <cmath>
#include <iostream>
#include <cstdlib>

const int MAX_REFLECTIONS = 2;

Raytracer::Raytracer() : _lightSource(nullptr) {
	_root = std::make_shared<SceneDagNode>();
}

Raytracer::~Raytracer() {
}

SceneDagNode::Ptr Raytracer::addObject( SceneDagNode::Ptr parent, 
		SceneObject::Ptr obj, Material::Ptr mat ) {
    SceneDagNode::Ptr node = std::make_shared<SceneDagNode>( obj, mat );
	node->parent = parent;
	node->next = nullptr;
	node->child = nullptr;
	
	// Add the object to the parent's child list, this means
	// whatever transformation applied to the parent will also
	// be applied to the child.
	if (parent->child == nullptr) {
		parent->child = node;
	}
	else {
		parent = parent->child;
		while (parent->next != nullptr) {
			parent = parent->next;
		}
		parent->next = node;
	}
	
	return node;
}

LightListNode::Ptr Raytracer::addLightSource( LightSource::Ptr light ) {
    LightListNode::Ptr tmp = _lightSource;
	_lightSource = std::make_shared<LightListNode>( light, tmp );
	return _lightSource;
}

void Raytracer::rotate( SceneDagNode::Ptr node, char axis, double angle ) {
	Matrix4x4 rotation;
	double toRadian = 2*M_PI/360.0;
	int i;
	
	for (i = 0; i < 2; i++) {
		switch(axis) {
			case 'x':
				rotation[0][0] = 1;
				rotation[1][1] = cos(angle*toRadian);
				rotation[1][2] = -sin(angle*toRadian);
				rotation[2][1] = sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'y':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][2] = sin(angle*toRadian);
				rotation[1][1] = 1;
				rotation[2][0] = -sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'z':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][1] = -sin(angle*toRadian);
				rotation[1][0] = sin(angle*toRadian);
				rotation[1][1] = cos(angle*toRadian);
				rotation[2][2] = 1;
				rotation[3][3] = 1;
			break;
		}
		if (i == 0) {
		    node->trans = node->trans*rotation; 	
			angle = -angle;
		} 
		else {
			node->invtrans = rotation*node->invtrans; 
		}	
	}
}

void Raytracer::translate( SceneDagNode::Ptr node, Vector3D trans ) {
	Matrix4x4 translation;
	
	translation[0][3] = trans[0];
	translation[1][3] = trans[1];
	translation[2][3] = trans[2];
	node->trans = node->trans*translation; 	
	translation[0][3] = -trans[0];
	translation[1][3] = -trans[1];
	translation[2][3] = -trans[2];
	node->invtrans = translation*node->invtrans; 
}

void Raytracer::scale( SceneDagNode::Ptr node, Point3D origin, double factor[3] ) {
	Matrix4x4 scale;
	
	scale[0][0] = factor[0];
	scale[0][3] = origin[0] - factor[0] * origin[0];
	scale[1][1] = factor[1];
	scale[1][3] = origin[1] - factor[1] * origin[1];
	scale[2][2] = factor[2];
	scale[2][3] = origin[2] - factor[2] * origin[2];
	node->trans = node->trans*scale; 	
	scale[0][0] = 1/factor[0];
	scale[0][3] = origin[0] - 1/factor[0] * origin[0];
	scale[1][1] = 1/factor[1];
	scale[1][3] = origin[1] - 1/factor[1] * origin[1];
	scale[2][2] = 1/factor[2];
	scale[2][3] = origin[2] - 1/factor[2] * origin[2];
	node->invtrans = scale*node->invtrans; 
}

Matrix4x4 Raytracer::initInvViewMatrix( Point3D eye, Vector3D view, 
		Vector3D up ) {
	Matrix4x4 mat; 
	Vector3D w;
	view.normalize();
	up = up - up.dot(view)*view;
	up.normalize();
	w = view.cross(up);

	mat[0][0] = w[0];
	mat[1][0] = w[1];
	mat[2][0] = w[2];
	mat[0][1] = up[0];
	mat[1][1] = up[1];
	mat[2][1] = up[2];
	mat[0][2] = -view[0];
	mat[1][2] = -view[1];
	mat[2][2] = -view[2];
	mat[0][3] = eye[0];
	mat[1][3] = eye[1];
	mat[2][3] = eye[2];

	return mat; 
}


void Raytracer::computeTransforms( SceneDagNode::Ptr node )
{
    SceneDagNode::Ptr childPtr;
    if (node->parent != nullptr)
    {
        node->modelToWorld = node->parent->modelToWorld*node->trans;
        node->worldToModel = node->invtrans*node->parent->worldToModel; 
    }
    else
    {
        node->modelToWorld = node->trans;
        node->worldToModel = node->invtrans; 
    }
    // Traverse the children.
    childPtr = node->child;
    while (childPtr != NULL) {
        computeTransforms(childPtr);
        childPtr = childPtr->next;
    }



}

void Raytracer::traverseScene( SceneDagNode::Ptr node, Ray3D& ray ) {
    SceneDagNode::Ptr childPtr;

    // Applies transformation of the current node to the global
    // transformation matrices.
    if (node->obj) {
        // Perform intersection.
        if (node->obj->intersect(ray, node->worldToModel, node->modelToWorld)) {
            ray.intersection.mat = node->mat;
        }
    }
    // Traverse the children.
    childPtr = node->child;
    while (childPtr != nullptr) {
        traverseScene(childPtr, ray);
        childPtr = childPtr->next;
    }

}

void Raytracer::computeShading( Ray3D& ray ) {
    LightListNode::Ptr curLight = _lightSource;
    for (;;) {
        if (curLight == nullptr) break;
        // Each lightSource provides its own shading function.

        // Implement shadows here if needed.

        Point3D light = curLight->light->get_position();
        Colour col_ambient = curLight->light->get_ambient();
        Vector3D lightdir = light - ray.intersection.point;
        double t = lightdir.length();
        lightdir.normalize();
        Ray3D shadow(ray.intersection.point + 0.01 * lightdir, lightdir);

        traverseScene(_root, shadow);

        if (!shadow.intersection.none && t >= shadow.intersection.t_value) {
        	ray.col = ray.intersection.mat->ambient * col_ambient;
        	ray.col.clamp();	
        } else curLight->light->shade(ray);
        curLight = curLight->next;
    }
}

void Raytracer::initPixelBuffer() {
    int numbytes = _scrWidth * _scrHeight * sizeof(unsigned char);
    _rbuffer = new unsigned char[numbytes];
    std::fill_n(_rbuffer, numbytes,0);
    _gbuffer = new unsigned char[numbytes];
    std::fill_n(_gbuffer, numbytes,0);
    _bbuffer = new unsigned char[numbytes];
    std::fill_n(_bbuffer, numbytes,0);
}

void Raytracer::flushPixelBuffer( std::string file_name ) {
    bmp_write( file_name.c_str(), _scrWidth, _scrHeight, _rbuffer, _gbuffer, _bbuffer );
    delete _rbuffer;
    delete _gbuffer;
    delete _bbuffer;
}

Colour Raytracer::shadeRay( Ray3D& ray, int reflections = 0 ) {
    Colour col(0.0, 0.0, 0.0); 
    traverseScene(_root, ray); 

    // Don't bother shading if the ray didn't hit 
    // anything.
    if (!ray.intersection.none) {
        computeShading(ray); 
        col = ray.col;  
    }

    // You'll want to call shadeRay recursively (with a different ray, 
    // of course) here to implement reflection/refraction effects.  

    if (!ray.intersection.none) {
	    if (ray.intersection.mat->reflective > 0 && reflections < MAX_REFLECTIONS) {
	    	Ray3D r;
	    	r.origin = ray.intersection.point;
	    	r.dir = ray.intersection.normal;
	    	r.dir.normalize();

	    	Colour ref = shadeRay(r, reflections+1);
	    	return ray.intersection.mat->reflective * ref + (1 - ray.intersection.mat->reflective) * col;
	    }
	}
    return col; 
}	

void Raytracer::render( int width, int height, Point3D eye, Vector3D view, 
        Vector3D up, double fov, std::string fileName ) {
    computeTransforms(_root);
    Matrix4x4 viewToWorld;
    _scrWidth = width;
    _scrHeight = height;
    double factor = (double(height)/2)/tan(fov*M_PI/360.0);

    initPixelBuffer();
    viewToWorld = initInvViewMatrix(eye, view, up);
    Colour newcol = Colour(0,0,0);

    int num_dov = 99;
    double newx = 0., newy = 0., newz = 0.;
    Point3D neworigin;
    bool starteddraw = false;

    for (int k = 0; k < num_dov; k++) {

    	neworigin[0] = newx;
    	neworigin[1] = newy;
    	neworigin[2] = newz;

    	switch (k % 3) {
    		case 0:
    			if (k % 6 == 0) 
    				newx = -newx;
    			else
    				newx = -newx;
    				newx += 0.01;
    			break;
    		case 1:
    			if (k % 2*6 == 0) 
    				newy = -newy;
    			else
    				newy = -newy;
    				newy += 0.01;
    			break;
    		case 2:
    			if (k % 2*6 == 0) 
    				newz = -newz;
    			else
    				newz = -newz;
    				newz += 0.01;
    			break;
    	}

	    // Construct a ray for each pixel.
	    for (int i = 0; i < _scrHeight * 2; i++) {
	        for (int j = 0; j < _scrWidth * 2; j++) {
	            // Sets up ray origin and direction in view space, 
	            // image plane is at z = -1.
	            Point3D origin(0., 0., 0.);
				Point3D imagePlane;
				imagePlane[0] = (-double(width)/2 + 0.5 + j)/factor;
				imagePlane[1] = (-double(height)/2 + 0.5 + i)/factor;
				imagePlane[2] = -1;

				// TODO: Convert ray to world space and call 
				// shadeRay(ray) to generate pixel colour. 	
				
				Ray3D ray;

				ray.origin = viewToWorld * neworigin;
				ray.dir = viewToWorld * (imagePlane - neworigin);
				ray.dir.normalize();


				Colour col = shadeRay(ray);

				//antialiasing by supersampling
				if (j % 2 == 1) {
					newcol = 0.5 * col + 0.5 * newcol;
					if (starteddraw){
						_rbuffer[int(floor(i/2)*width+floor(j/2))] = int((0.5 * newcol[0] * 255) + 0.5 * _rbuffer[int(floor(i/2)*width+floor(j/2))]);
						_gbuffer[int(floor(i/2)*width+floor(j/2))] = int((0.5 * newcol[1] * 255) + 0.5 * _rbuffer[int(floor(i/2)*width+floor(j/2))]);
						_bbuffer[int(floor(i/2)*width+floor(j/2))] = int((0.5 * newcol[2] * 255) + 0.5 * _rbuffer[int(floor(i/2)*width+floor(j/2))]);
					} else {
						_rbuffer[i/2 * width + j/2] = int(newcol[0] * 255);
						_gbuffer[i/2 * width + j/2] = int(newcol[1] * 255);
						_bbuffer[i/2 * width + j/2] = int(newcol[2] * 255);
						starteddraw = true;
					}
					newcol = Colour(0,0,0);
				} else {
					newcol = col;
				}
			}
		}
	}

	flushPixelBuffer(fileName);
}

