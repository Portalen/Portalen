#pragma once

#include "ofMain.h"
#include "ofxStreamer.h"
#include "ofxOpenCv.h"
#include "Canon.h"
#include "ofxOsc.h"

#define REMOTE_HOST "127.0.0.1"

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
    
    
    ofVideoGrabber grabber;
    
    ofxOscReceiver oscReciver;
    ofxOscSender oscSender;
    
    ofxStreamerReceiver hqreceiver;
    ofxStreamerReceiver lqreceiver;
    
    ofxStreamerSender hqsender;
    ofxStreamerSender lqsender;
    
    float hqFrameRate = 25;
    
    float streamWidth, streamHeight;
    
    ofFbo camFbo;
    ofFbo camOutFboHQ;
    ofFbo camOutFboLQ;
    ofFbo outputFbo;
    
    ofPixels outPixelsHQ;
    ofPixels outPixelsLQ;
    ofxCvColorImage image;
    
    int newframes = 0;
    
    roxlu::Canon canon;
    
    float lastTime;
    
    RegionOfInterest roi;
    
    float roiMaxRadius;
    
};