#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofLogLevel(OF_LOG_WARNING);
    ofSetFrameRate(40);
    
    receiver.setup(1234);
    
    grabber.initGrabber(320, 240);
    
    sender.setup(grabber.getWidth(), grabber.getHeight(), "127.0.0.1");
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    grabber.update();
    
    if(grabber.isFrameNew()){
        sender.sendFrame(grabber.getPixelsRef());
    }
    
    receiver.update();
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    receiver.draw(0, 0, 640, 480);
    grabber.draw(0, 480, 640, 480);

}

void ofApp::exit(){
    receiver.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
