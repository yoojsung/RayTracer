//Jaesung Yoo

//  RayCaster - Set of simple classes to create a camera/view setup for our Ray Tracer HW Project
//
//  I've included these classes as a mini-framework for our introductory ray tracer.
//  You are free to modify/change.   
//
//  These classes provide a simple render camera which can can return a ray starting from
//  it's position to a (u, v) coordinate on the view plane.
//
//  The view plane is where we can locate our photorealistic image we are rendering.
//  The field-of-view of the camera by moving it closer/further 
//  from the view plane.  The viewplane can be also resized.  When ray tracing an image, the aspect
//  ratio of the view plane should the be same as your image. So for example, the current view plane
//  default size is ( 6.0 width by 4.0 height ).   A 1200x800 pixel image would have the same
//  aspect ratio.
//
//  This is not a complete ray tracer - just a set of skelton classes to start.  The current
//  base scene object only stores a value for the diffuse/specular color of the object (defaut is gray).
//  at some point, we will want to replace this with a Material class that contains these (and other 
//  parameters)
//  
//  (c) Kevin M. Smith  - 24 September 2018
//
#pragma once

#include <glm/gtx/intersect.hpp>
#include "ofMain.h"
#include "ofxGui.h"

//  General Purpose Ray class 
//

class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual ofColor getColor(glm::vec3 p) { return diffuseColor; }

	void applyTexture(ofImage map) {
		texture = map;
		textWidth = texture.getWidth();
		textHeight = texture.getHeight();
		textured = true;
	}

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
	ofImage texture;
	float textWidth; 
	float textHeight;
	bool textured = false;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
		//reset color to default color: white in this case
		ofSetColor(ofColor::white);
	}
	ofColor getColor(glm::vec3 p) {
		return diffuseColor;
	}
	float radius = 1.0;
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
	ofColor getColor(glm::vec3 p) { return diffuseColor; }
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() { }
	glm::vec3 normal = glm::vec3(0, 1, 0);
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofColor getColor(glm::vec3 p) {
		if (textured) {
			if (normal == glm::vec3(0, 1, 0)) {
				int i = abs(round((p.x / 15 * textWidth) - 0.5));
				int j = abs(round((p.z / 15 * textHeight) - 0.5));
				return texture.getColor((int)fmod(i, textWidth), (int)fmod(j, textHeight));
			}
			else if (normal == glm::vec3(0, 0, 1)) {
				int i = abs(round((p.x / 15 * textWidth) - 0.5));
				int j = abs(round((p.y / 15 * textHeight) - 0.5));
				return texture.getColor((int)fmod(i, textWidth), (int)fmod(j, textHeight));
			}
		}
		return diffuseColor;
	}
	ofPlanePrimitive plane;
	float width = 20;
	float height = 20;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	ofColor getColor(glm::vec3 p) { return ofColor::black; }
	//void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

class Light : public SceneObject {
public:
	float intensity = 10;
	virtual void draw() = 0;
	bool mouseIntersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, .2, point, normal));
	}

	void setIntensity(float i) {
		intensity = i;
	}

	ofColor getColor(glm::vec3 p) { return ofColor::black; }
	ofLight cpLight;
};

class PointLight : public Light {
public:
	PointLight(glm::vec3 p) { position = p; };

	void draw() {
		ofDrawSphere(position, radius);
	}

	float radius = 0.1;
};

class SpotLight : public Light {
public: 
	SpotLight(glm::vec3 p) { position = p; };

	void draw() {
		ofDrawSphere(position, radius);
	}

	float radius = 0.1;
};

class Triangle {
public:
	Triangle(int i1, int i2, int i3) { vertInd[0] = i1, vertInd[1] = i2, vertInd[2] = i3; }
	int vertInd[3];
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse);
	ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power);
	bool ofApp::inShadow(Ray ray, glm::vec3 norm, glm::vec3 lightp);
	void addLight(Light *light) {
		lightList.push_back(light);
	}
	void drawAxis(glm::vec3 position);

	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera  *theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image throughn
	//
	RenderCam renderCam;
	ofImage image;

	vector<SceneObject *> scene;
	vector<Light *> lightList;


	int imageWidth = 1800;
	int imageHeight = 1200;

	bool drawImg = false;

	ofxPanel gui;

	ofxFloatSlider intens;
	ofxFloatSlider power;
};
