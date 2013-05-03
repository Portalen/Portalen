#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

class Pong {
public:
    
    ofFbo fbo;
    
    void setup();
    void update();
    
    
    ofPoint playerPosition;
    ofPoint ballPosition;
    ofVec2f ballDir;
    
    ofxOscSender * oscSender;
    
    void receiveOsvMessage(ofxOscMessage msg);
    
};