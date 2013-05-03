#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

class Pong {
public:
    
    ofFbo fbo;
    
    void setup();
    void render();
    void update();
    
    
    ofPoint playerPosition;
    ofPoint ballPosition;
    ofVec2f ballDir;
    
    ofPoint otherPlayerPosition;
    
    ofxOscSender * oscSender;
    
    void receiveOscMessage(ofxOscMessage msg);
    void launchBall();
    void publishBall();
};