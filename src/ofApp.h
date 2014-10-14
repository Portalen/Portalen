#pragma once

#define USE_WEBCAM
#define USE_SENDER
#define REMOTE_HOST "127.0.0.1"

#include "ofMain.h"
#include "ofxStreamer.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxBiquadFilter.h"
#include "ofxGui.h"



#ifndef USE_WEBCAM
#include "Canon.h"
#endif

class RegionOfInterest {
public:
    ofVec2f center;
    ofVec2f lastCenter;
    float radius;
    float zoom;
    float alpha;
    
    ofxBiquadFilter2f centerFilter;
    
    ofxBiquadFilter2f highPass;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
#ifdef USE_WEBCAM
    ofVideoGrabber grabber;
#else
    roxlu::Canon canon;
#endif
    
    ofxOscReceiver oscReciver;
    ofxOscSender oscSender;
    
    ofxStreamerReceiver hqreceiver;
    ofxStreamerReceiver lqreceiver;
    
    ofxStreamerSender hqsender;
    ofxStreamerSender lqsender;
    
    float hqFrameRate = 30;
    
    float streamWidth, streamHeight;
    
    ofFbo camFbo;
    ofFbo camOutFboHQ;
    ofFbo camOutFboLQ;
    
    ofFbo outFbo;
    
    ofPixels outPixelsHQ;
    ofPixels outPixelsLQ;
    
    float lastTime;
    
    RegionOfInterest roi;
    
    float roiMaxRadius;
    
    vector <ofPoint> NormCirclePts;
    vector <ofPoint> NormCircleCoords;
    

    
    ofxPanel gui;
    ofParameterGroup params;
    
    ofParameter<float> roiSize;
    ofParameter<float> roiZoom;
    
    ofShader shaderBlurX;
    ofShader shaderBlurY;
    
    ofShader shaderDesaturate;
    
    ofFbo fboBlurOnePass;
    ofFbo fboBlurTwoPass;
    
    
    
    
};