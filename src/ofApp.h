#pragma once

#define USE_WEBCAM
#define REMOTE_HOST "127.0.0.1"

#include "ofMain.h"
#include "ofxStreamer.h"
#include "ofxOpenCv.h"
#ifndef USE_WEBCAM
#include "Canon.h"
#endif
#include "ofxOsc.h"



class RegionOfInterest {
public:
    ofVec2f center;
    float radius;
    float zoom;
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
    
    ofFbo outputFbo;
    
    ofPixels outPixelsHQ;
    ofPixels outPixelsLQ;
    ofxCvColorImage image;
    
    int newframes = 0;
    
    
    float lastTime;
    
    RegionOfInterest roi;
    
    float roiMaxRadius;
    
    vector <ofPoint> NormCirclePts;
    vector <ofPoint> NormCircleCoords;
    
};