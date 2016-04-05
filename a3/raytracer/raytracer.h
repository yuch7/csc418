/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		This file contains the interface and 
		datastructures of the raytracer.  
		Simple traversal and addition code to 
		the datastructures are given to you.

***********************************************************/

#include "util.h"
#include "scene_object.h"
#include "light_source.h"
#include <memory>

// Linked list containing light sources in the scene.
struct LightListNode {
    using Ptr = std::shared_ptr<LightListNode>;
	LightListNode() : light(nullptr), next(nullptr) {}
	LightListNode( LightSource::Ptr light, LightListNode::Ptr next = nullptr ) : 
		light(light), next(next) {}

    LightSource::Ptr light;
	Ptr next;
};

// The scene graph, containing objects in the scene.
struct SceneDagNode {
    using Ptr = std::shared_ptr<SceneDagNode>;
	SceneDagNode() : 
		obj(nullptr), mat(nullptr), 
		next(nullptr), parent(nullptr), child(nullptr) {
	}	

	SceneDagNode( SceneObject::Ptr obj, Material::Ptr mat ) : 
		obj(obj), mat(mat), next(nullptr), parent(nullptr), child(nullptr) {
		}
	

	// Pointer to geometry primitive, used for intersection.
    SceneObject::Ptr obj;
	// Pointer to material of the object, used in shading.
    Material::Ptr mat;
	// Each node maintains a transformation matrix, which maps the 
	// geometry from object space to world space and the inverse.
	Matrix4x4 trans;
	Matrix4x4 invtrans;
	Matrix4x4 modelToWorld;
	Matrix4x4 worldToModel;
	
	// Internal structure of the tree, you shouldn't have to worry 
	// about them.
	Ptr next;
	Ptr parent;
	Ptr child;
};

class Raytracer {
public:
	Raytracer();
	~Raytracer();

	// Renders an image fileName with width and height and a camera
	// positioned at eye, with view vector view, up vector up, and 
	// field of view fov.
	void render( int width, int height, Point3D eye, Vector3D view, 
			Vector3D up, double fov, std::string fileName );

	// Add an object into the scene, with material mat.  The function
	// returns a handle to the object node you just added, use the 
	// handle to apply transformations to the object.
    SceneDagNode::Ptr addObject( SceneObject::Ptr obj, Material::Ptr mat ) {
		return addObject(_root, obj, mat);
	}
	
	// Add an object into the scene with a specific parent node, 
	// don't worry about this unless you want to do hierarchical 
	// modeling.  You could create nodes with NULL obj and mat, 
	// in which case they just represent transformations.  
    SceneDagNode::Ptr addObject( SceneDagNode::Ptr parent, SceneObject::Ptr obj, 
			Material::Ptr mat );

	// Add a light source.
    LightListNode::Ptr addLightSource( LightSource::Ptr light );

	// Transformation functions are implemented by right-multiplying 
	// the transformation matrix to the node's transformation matrix.
	
	// Apply rotation about axis 'x', 'y', 'z' angle degrees to node.
	void rotate( SceneDagNode::Ptr node, char axis, double angle );

	// Apply translation in the direction of trans to node.
	void translate( SceneDagNode::Ptr node, Vector3D trans );

	// Apply scaling about a fixed point origin.
	void scale( SceneDagNode::Ptr node, Point3D origin, double factor[3] );
	
private:
	// Allocates and initializes the pixel buffer for rendering, you
	// could add an interesting background to your scene by modifying 
	// this function.
	void initPixelBuffer();

	// Saves the pixel buffer to a file and deletes the buffer.
	void flushPixelBuffer(std::string file_name);

	// Return the colour of the ray after intersection and shading, call 
	// this function recursively for reflection and refraction.  
	Colour shadeRay( Ray3D& ray, int intersections ); 

	// Constructs a view to world transformation matrix based on the
	// camera parameters.
	Matrix4x4 initInvViewMatrix( Point3D eye, Vector3D view, Vector3D up );

	// Traversal code for the scene graph, the ray is transformed into 
	// the object space of each node where intersection is performed.
	void traverseScene( SceneDagNode::Ptr node, Ray3D& ray );

	// After intersection, calculate the colour of the ray by shading it
	// with all light sources in the scene.
    void computeShading( Ray3D& ray );

    // Precompute the modelToWorld and worldToModel transformations for each
    // object in the scene.
    void computeTransforms( SceneDagNode::Ptr node );


    // Width and height of the viewport.
    int _scrWidth;
    int _scrHeight;

    // Light list and scene graph.
    LightListNode::Ptr _lightSource;
    SceneDagNode::Ptr _root;

    // Pixel buffer.
    unsigned char* _rbuffer;
    unsigned char* _gbuffer;
    unsigned char* _bbuffer;

    // Maintain global transformation matrices similar to OpenGL's matrix
    // stack.  These are used during scene traversal. 
    Matrix4x4 _modelToWorld;
    Matrix4x4 _worldToModel;
};
