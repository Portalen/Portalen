#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofLogLevel(OF_LOG_WARNING);
    ofSetFrameRate(30);
    
    grabber.setDesiredFrameRate(2);
    
    grabber.initGrabber(1920/2, 1080/2);
    
    sender.setup(grabber.getWidth(), grabber.getHeight(), REMOTE_HOST);
    camOutFbo.allocate(grabber.getWidth(), grabber.getHeight());
    outputFbo.allocate(grabber.getWidth(), grabber.getHeight());
    
    receiver.setup(1234);

}

//--------------------------------------------------------------
void ofApp::update(){
    
    grabber.update();
    receiver.update();
    
    if(grabber.isInitialized() && grabber.isFrameNew()){
        
        newframes++;
        
        /*camOutFbo.begin();
        grabber.draw(0, 0);
        //ofBackground(0, 0, 0, 100);
        
        ofSetColor(255, 255, 255);
        ofCircle(150, 150, 100);
        
        camOutFbo.end();
        
        camOutFbo.readToPixels(outPixels);*/
        //image.setFromPixels(outPixels, grabber.getWidth(), grabber.getHeight());
        
        //sender.sendFrame(outPixels); //
        
        if(newframes > 20) {
            newframes = 0;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    sender.sendFrame(grabber.getPixelsRef());
    
    //outputFbo.begin();
    //outputFbo.end();
    
    grabber.draw(0, 0);

    receiver.draw(0, 400);

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
