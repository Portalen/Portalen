#pragma once

#define USE_WEBCAM
//#define USE_SENDER
#define REMOTE_HOST "127.0.0.1"
=======
//#define USE_CANON_LIVEVIEW
//#define USE_GRABBER

#define REMOTE_HOST "25.91.17.173"

#define LOW_QUALITY_STREAM_PORT 9000
#define HIGH_QUALITY_STREAM_PORT 9100
#define OSC_DATA_PORT 9200

#include "ofMain.h"
#include "ofxStreamer.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxBiquadFilter.h"
#include "ofxGui.h"


#include "ofxOpticalFlowFarneback.h"
#include "ofxFlowTools.h"

#ifndef USE_WEBCAM
#include "Canon.h"
#endif

class RegionOfInterest {
public:
    ofVec2f center;
    ofVec2f rawCenter;

    ofVec2f lastCenter;
    float radius;
    float zoom;
    float alpha;
    
    bool local = true;
    
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
    
    ofxOscReceiver oscReceiver;
    ofxOscSender oscSender;
    
    ofxStreamerReceiver hqreceiver;
    ofxStreamerReceiver lqreceiver;
    
    ofxStreamerSender hqsender;
    ofxStreamerSender lqsender;
    
    float hqFrameLastTime;
    float lqFrameLastTime;
    float oscUpdateLastTime;
    
    float hqFrameRate = 14;
    float lqFrameRate = 14;
    float oscUpdateRate = 20;
    
    float streamWidth, streamHeight;
    
    ofFbo camFbo;
    ofFbo camOutFboHQ;
    ofFbo camOutFboLQ;
    
    ofFbo outFbo;
    
    ofPixels outPixelsHQ;
    ofPixels outPixelsLQ;
    
    RegionOfInterest * localRoi;
    RegionOfInterest * remoteRoi;
    
    float roiMaxRadius;
    
    vector <ofPoint> NormCirclePts;
    vector <ofPoint> NormCircleCoords;
    
    ofParameter<float> roiSize;
    ofParameter<float> roiZoom;
    
    ofShader shaderBlurX;
    ofShader shaderBlurY;
    
    ofShader shaderDesaturate;
    
    ofFbo fboBlurOnePass;
    ofFbo fboBlurTwoPass;
    
    ofxOpticalFlowFarneback flowSolver;
    ofPoint center;

    bool debugMode = false;
    
};