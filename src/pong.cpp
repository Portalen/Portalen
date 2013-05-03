#include "pong.h"

#define PADDLE_WIDTH 0.1
#define PADDLE_HEIGHT 0.05

#define BALL_SIZE 0.05

void Pong::setup(){
    fbo.allocate(1024, 768);
    
    ballPosition = ofPoint(0.5,0.5);
    ballDir = ofVec2f(ofRandom(-1,1), ofRandom(-1,1));
}

void Pong::update(){
    ballPosition += ballDir * 1.0/ofGetFrameRate();
    
    if(ballPosition.x < 0 &&  ballDir.x < 0){
        ballDir.x *= -1;
    }
    if(ballPosition.x > 1 &&  ballDir.x > 0){
        ballDir.x *= -1;
    }
    
    if(ballPosition.y < 0 &&  ballDir.y < 0){
        ballDir.y *= -1;
    }
    if(ballPosition.y > 1 &&  ballDir.y > 0){
        ballDir.y *= -1;
    }
    
    if(ballPosition.x > playerPosition.x-PADDLE_WIDTH*0.5 &&
       ballPosition.x < playerPosition.x+PADDLE_WIDTH*0.5 &&
       ballPosition.y > playerPosition.y &&
       ballPosition.y < playerPosition.y+PADDLE_HEIGHT &&
       ballDir.y < 0){
        ballDir.y *= -1;
    
    
    }
    
    fbo.begin();{
        ofClear(255,0,0);
        glScaled(1024, 768, 1);
        ofSetColor(255,255,255);
        ofRect(playerPosition.x-PADDLE_WIDTH*0.5, playerPosition.y, PADDLE_WIDTH, PADDLE_HEIGHT);
        
        
        ofRect(ballPosition.x-BALL_SIZE, ballPosition.y-BALL_SIZE*0.5, BALL_SIZE, BALL_SIZE);
        
    }fbo.end();
    
}

void receiveOsvMessage(ofxOscMessage msg){
    
}
