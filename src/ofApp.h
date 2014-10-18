#pragma once


//#define USE_WEBCAM
//#define USE_CANON_LIVEVIEW
#define USE_BLACK_MAGIC

#define USE_SENDER

#define REMOTE_HOST "25.79.159.131" //25.91.17.173"

#define LOW_QUALITY_STREAM_PORT 9000
#define HIGH_QUALITY_STREAM_PORT 9100
#define OSC_DATA_PORT 9200

#include "ofMain.h"
#include "ofxStreamer.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxBiquadFilter.h"
#include "ofxFlowTools.h"
#include "ofxSyphon.h"

#ifdef USE_CANON_LIVEVIEW
#include "Canon.h"
#endif

#ifdef USE_BLACK_MAGIC
#include "ofxBlackMagic.h" // needs a modification to support out framerate
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
#endif
    
#ifdef USE_CANON_LIVEVIEW
    roxlu::Canon canon;
#endif
    
#ifdef USE_BLACK_MAGIC
    ofxBlackMagic blackMagicCam;
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
    
    //ofParameter<float> roiSize;
    //ofParameter<float> roiZoom;
    
    ofShader shaderBlurX;
    ofShader shaderBlurY;
    
    ofShader deinterlace;
    
    ofShader shaderDesaturate;
    
    ofFbo fboBlurOnePass;
    ofFbo fboBlurTwoPass;
    
    ofxSyphonServer syphonOut;
    
    bool debugView = true;
    bool enableSyphonOut = true;
    bool fullscreen = true;
    
    float fadeRemote = 0;
};