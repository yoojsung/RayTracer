//Jaesung Yoo

#include "ofApp.h"

// Intersect Ray with Plane  (wrapper on glm::intersect*
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
	}
	return (hit);
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

//--------------------------------------------------------------
// sets up cameras and axis
// allocates image with size and color setting
// pushes sceneobjects into the scene vector: spheres, and a plane
void ofApp::setup() {
	gui.setup();
	gui.add(intens.setup("intensity", 10, 0.1, 1000.0));
	gui.add(power.setup("power", 35, 10.0, 10000.0));

	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;

	mainCam.setDistance(10);
	mainCam.setPosition(glm::vec3(0, 0, 10));
	mainCam.lookAt(glm::vec3(0, 0, 0));
	mainCam.setNearClip(.1);

	sideCam.setPosition(glm::vec3(10, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);

	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(renderCam.aim);
	previewCam.setNearClip(.1);

	drawAxis(glm::vec3(0, 0, 0));

	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);

	addLight(new PointLight(glm::vec3(2, 3, 5)));
	addLight(new PointLight(glm::vec3(-3, 3, 4)));
	addLight(new PointLight(glm::vec3(0, 0, 7)));

	SceneObject* ground = new Plane(glm::vec3(0, -3, 0), glm::vec3(0, 1, 0), ofColor::darkGray);
	SceneObject* wall = new Plane(glm::vec3(0, 0, -20), glm::vec3(0, 0, 1));

	ofImage texture;

	texture.load("texture5.jpg");
	ground->applyTexture(texture);

	texture.load("texture4.jpg");
	wall->applyTexture(texture);

	scene.push_back(ground);
	scene.push_back(wall);
	scene.push_back(new Sphere(glm::vec3(0, -1, 0), 3, ofColor::blueSteel));
	scene.push_back(new Sphere(glm::vec3(1.5, 1, 2), 0.5, ofColor::ghostWhite));
	scene.push_back(new Sphere(glm::vec3(-1.5, 1, 2), 0.5, ofColor::ghostWhite));
	scene.push_back(new Sphere(glm::vec3(0, 0, 2), 1, ofColor::orangeRed));
	scene.push_back(new Sphere(glm::vec3(0, -1.5, 2), 1.3, ofColor::darkRed));

	/*
	texture.load("texture3.jpg");
	ground->applyTexture(texture);

	texture.load("texture2.jpg");
	wall->applyTexture(texture);

	scene.push_back(ground);
	scene.push_back(wall);
	scene.push_back(new Sphere(glm::vec3(0, 1, 0), 1.5, ofColor::lightGoldenRodYellow));
	scene.push_back(new Sphere(glm::vec3(0, 3, 0), 1, ofColor::hotPink));
	scene.push_back(new Sphere(glm::vec3(0, -1, 0), 1, ofColor::deepPink));
	scene.push_back(new Sphere(glm::vec3(-1.7, 2, 0), 1, ofColor::deepPink));
	scene.push_back(new Sphere(glm::vec3(1.7, 2, 0), 1, ofColor::deepPink));
	scene.push_back(new Sphere(glm::vec3(-1.7, 0, 0), 1, ofColor::hotPink));
	scene.push_back(new Sphere(glm::vec3(1.7, 0, 0), 1, ofColor::hotPink));

	scene.push_back(new Sphere(glm::vec3(0.5, 1.15, 2), 0.1, ofColor::yellow));
	scene.push_back(new Sphere(glm::vec3(0.65, 1.1, 2), 0.1, ofColor::black));
	*/

	//scene.push_back(new Sphere(glm::vec3(-2, 0, 0), 2, ofColor::red));
	//scene.push_back(new Sphere(glm::vec3(0, -1, 2), 1, ofColor::green));
	//scene.push_back(new Sphere(glm::vec3(0, 2, -3), 3, ofColor::blue));
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
// starts camera with no fill
// sets color to yellow to draw the render cam
// draws all sceneobjects calling draw function which is virtual function
// if drawImg (bool) is true, draw image at (100, 100)
void ofApp::draw() {
	ofDisableDepthTest();
	gui.draw();
	ofEnableDepthTest();

	theCam->begin();
	ofNoFill();
	
	ofSetColor(ofColor::yellow);
	renderCam.draw();
	renderCam.view.draw();

	for (SceneObject* o : scene)
		o->draw();

	theCam->end();

	if (drawImg)
		image.draw(100, 100);
}

// ray tracing function - goes through nested loop to go through all pixels (600x400 in this case)
// then another nested loop to check if the ray created from that pixel is intersecting any sceneobject
// if intersects, check if it is the closest object, then saves the closest object and mark hit as true
// if hit, changes color to the color of the closest object; set it to blackground color if not hit
void ofApp::rayTrace() {
	for (auto l : lightList)
		l->setIntensity(intens);

	for (int j = 0; j < imageHeight; j++) // nested loops to visit all pixels
	{
		for (int i = 0; i < imageWidth; i++)
		{
			float u = (i + 0.5) / imageWidth; // x,y system to normalized coordinates (u, v) 
			float v = (j + 0.5) / imageHeight;

			Ray ray = renderCam.getRay(u, v); // gets ray using normalized coordinates

			float closeDistance = FLT_MAX;
			bool hit = false;

			SceneObject *closeObject = NULL;

			glm::vec3 closePoint;
			glm::vec3 closeNorm;

			glm::vec3 intersectPoint;
			glm::vec3 normal;

			for (SceneObject* obj : scene) // loop to all sceneobjects in scene
			{
				if (obj->intersect(ray, intersectPoint, normal))
				{
					float distance = sqrt(pow((intersectPoint.x - ray.p.x), 2) + pow((intersectPoint.y - ray.p.y), 2) + pow((intersectPoint.z - ray.p.z), 2)); // calculate distance between intersect point and ray starting point
					if (distance < closeDistance)
					{
						closeDistance = distance;
						closeObject = obj;
						closePoint = intersectPoint;
						closeNorm = normal;

						hit = true;
					}
				}
			}

			if (hit)
			{
				ofColor color = closeObject->getColor(closePoint);
				image.setColor(i, imageHeight - j - 1, phong(closePoint, closeNorm, color, ofColor::white, power));
				//image.setColor(i, imageHeight - j - 1, lambert(closePoint, closeNorm, color)); //important to flip the image because the top left should be 0 0 in this system
			}

			else
				image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor());
		}
	}

	cout << "ray trace done" << endl;

	image.save("res.jpeg"); //save the image into jpeg file called res.jpeg
}

ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse) {
	float ambientIntensity = 0.3f;
	ofColor k = diffuse;
	ofColor ambient = k * ambientIntensity;
	ofColor lam = ofColor::black;

	for (auto light : lightList)
	{
		glm::vec3 l = light->position - p;
		float r = glm::length(l);
		float maxDotLight = glm::max((float)0, glm::dot(glm::normalize(norm), glm::normalize(l)));

		if (!inShadow(Ray(p, glm::normalize(l)), glm::normalize(norm), light->position))
			lam += k * (light->intensity / glm::pow(r, 2)) * maxDotLight;
	}
	return ambient + lam;
}

ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power) {
	ofColor phong = ofColor::black;
	ofColor lam = lambert(p, norm, diffuse);
	ofColor k = specular;
	glm::vec3 v = renderCam.position - p;

	for (auto light : lightList) 
	{
		glm::vec3 l = light->position - p;
		glm::vec3 h = normalize(v + l);
		float r = glm::length(l);
		float maxDotLight = glm::max((float)0, glm::dot(normalize(norm), h));

		if (!inShadow(Ray(p, glm::normalize(l)), glm::normalize(norm), light->position))
			phong += k * (light->intensity / glm::pow(r, 2)) * glm::pow(maxDotLight, power);
	}
	return lam + phong;
}

bool ofApp::inShadow(Ray ray, glm::vec3 norm, glm::vec3 lightp) {
	const float epsilon = 0.0001;

	glm::vec3 intersectPoint;
	glm::vec3 normal;

	for (SceneObject* obj : scene)
	{
		if (obj->intersect(Ray(ray.p + (epsilon * norm), ray.d), intersectPoint, normal) && glm::length(lightp) > glm::length(intersectPoint - ray.p))
			return true;
	}
	return false;
}

// function for drawing axis
void ofApp::drawAxis(glm::vec3 position) {
	ofSetColor(ofColor::red);
	ofDrawLine(position, position + glm::vec3(1, 0, 0));
	ofSetColor(ofColor::green);
	ofDrawLine(position, position + glm::vec3(0, 1, 0));
	ofSetColor(ofColor::blue);
	ofDrawLine(position, position + glm::vec3(0, 0, 1));
	ofSetColor(ofColor::white);
}

//--------------------------------------------------------------
// function when keys are pressed: F1 to main cam (easycam), F2 to sideCam, F3 to previewCam
// r to raytrace and load the image from data folder to screen (t to turn it on and off like a switch)
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		break;
	case 'r':
		rayTrace();
		image.load("res.jpeg");
		break;
	case 't':
		drawImg = !drawImg;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
