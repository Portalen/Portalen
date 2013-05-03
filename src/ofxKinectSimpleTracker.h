#pragma once

#include "ofxKinect.h"

class ofxKinectSimpleTracker
{
public:
    ofxKinectSimpleTracker();
    
    bool open();
    void close();
    
    void update();
    void draw(int x, int y, int w=640, int h=480);
    
    
    ofxKinect *kinect;
    
    ofTexture & getTextureReference();

    unsigned char active;

    float elevation;
    float azimuth;
	float radius;
	float offset;
    
    
private:
    void renderFbo();
    ofFbo * fbo;

	float x;
    float y;

	float default_x;
	float default_y;
	int gesture;



	int width;
    int height;
	int minsize;
	int maxsize;

	float target_filter;
	float gain;

	int clientWidth;
    int clientHeight;

	float rooflevel;
	float floorlevel;
	float apex;


};
