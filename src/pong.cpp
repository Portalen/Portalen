#include "pong.h"

#define PADDLE_WIDTH 0.3
#define PADDLE_HEIGHT 0.03

#define BALL_SIZE 0.02

void Pong::setup(){
    fbo.allocate(512, 1024);
    
    
    oscSender = nil;
}


void Pong::launchBall(){
    ballPosition = ofPoint(0.5,0.5);
    ballDir = ofVec2f(ofRandom(-1,1), ofRandom(-1,1));
    ballDir.normalize();
    
    ballDir *= 0.3;
    
    publishBall();
}

void Pong::publishBall(){
    if(oscSender){
        ofxOscMessage msg;
        msg.setAddress("/pong/ball");
        msg.addFloatArg(ballPosition.x);
        msg.addFloatArg(ballPosition.y);
        msg.addFloatArg(ballDir.x);
        msg.addFloatArg(ballDir.y);
        
        oscSender->sendMessage(msg);
    }
}

void Pong::update(){
    ballPosition += ballDir * 1.0/ofGetFrameRate();
    
    if(ballPosition.x < 0 &&  ballDir.x < 0){
        ballDir.x *= -1;
//        publishBall();

    }
    if(ballPosition.x > 1 &&  ballDir.x > 0){
        ballDir.x *= -1;
//        publishBall();

    }
    
    if(ballPosition.y < 0 &&  ballDir.y < 0){
        ballDir.y *= -1;
     //   publishBall();

    }
    if(ballPosition.y > 1 &&  ballDir.y > 0){
        ballDir.y *= -1;
        //    publishBall();
        
    }
    
    if(ballPosition.x > playerPosition.x-PADDLE_WIDTH*0.5 &&
       ballPosition.x < playerPosition.x+PADDLE_WIDTH*0.5 &&
       ballPosition.y > playerPosition.y-PADDLE_HEIGHT &&
       ballPosition.y < playerPosition.y+PADDLE_HEIGHT &&
       ballDir.y < 0){
        ballDir.y *= -1.1;
        
        publishBall();
        
    }
    
    fbo.begin();{
        ofClear(0,0,0,0);
        glScaled(fbo.getWidth(), fbo.getHeight(), 1);
        render();
        
    }fbo.end();
       
    if(oscSender){
        ofxOscMessage msg;
        msg.setAddress("/pong/playerPosition");
        msg.addFloatArg(playerPosition.x);
        msg.addFloatArg(playerPosition.y);
        
        oscSender->sendMessage(msg);
    }
    
}


void Pong::render(){
    ofSetColor(255,255,255);
    ofRect(playerPosition.x-PADDLE_WIDTH*0.5, playerPosition.y, PADDLE_WIDTH, PADDLE_HEIGHT);
    ofRect(otherPlayerPosition.x-PADDLE_WIDTH*0.5, (1-otherPlayerPosition.y), PADDLE_WIDTH, PADDLE_HEIGHT);
    
    //Ball
    ofRect(ballPosition.x-BALL_SIZE, ballPosition.y-BALL_SIZE*0.5, BALL_SIZE, BALL_SIZE);
    
    //Line
    ofLine(0, 0.5, 1, 0.5);
    
}


void Pong::receiveOscMessage(ofxOscMessage msg){
    cout<<"Pong message "<<msg.getAddress()<<endl;
    
    if(msg.getAddress().compare("/pong/ball") == 0){
        ballPosition.x = msg.getArgAsFloat(0);
        ballPosition.y = 1-msg.getArgAsFloat(1);
        ballDir.x = msg.getArgAsFloat(2);
        ballDir.y = -1*msg.getArgAsFloat(3);
        
    }
    
    if(msg.getAddress().compare("/pong/playerPosition") == 0){
        otherPlayerPosition.x = msg.getArgAsFloat(0);
        otherPlayerPosition.y = msg.getArgAsFloat(1);
    }
}
