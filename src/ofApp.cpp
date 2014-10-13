#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    canon.start();

    //ofLogLevel(OF_LOG_SILENT);
    ofSetFrameRate(30);
    
    //grabber.setDesiredFrameRate(2);
    //grabber.initGrabber(1920/2, 1080/2);
    
    streamWidth = 1056;
    streamHeight = 704;
    
    roiMaxRadius = 200;
    
    hqsender.setup(roiMaxRadius, roiMaxRadius, REMOTE_HOST, 1234);//, "placebo", "zerolatency");
    lqsender.setup(streamWidth/4, streamHeight/4, REMOTE_HOST, 1235);//, "ultrafast", "zerolatency");
    
    camFbo.allocate(streamWidth, streamHeight, GL_RGB8);
    
    camOutFboHQ.allocate(roiMaxRadius, roiMaxRadius, GL_RGB8);
    camOutFboLQ.allocate(streamWidth/4, streamHeight/4, GL_RGB8);

    hqreceiver.setup(1234);
    lqreceiver.setup(1235);
    
    camFbo.begin();
    ofClear(0,0,0);
    camFbo.end();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //grabber.update();
    lqreceiver.update();
    hqreceiver.update();
    
    if(!canon.isLiveViewActive() && canon.isSessionOpen()) {
        canon.startLiveView();
    }
    
    //if(grabber.isInitialized() && grabber.isFrameNew()){
        
       // newframes++;
        
        /*camOutFbo.begin();
         canon.drawLiveView();

        grabber.draw(0, 0);
        //ofBackground(0, 0, 0, 100);
        
        ofSetColor(255, 255, 255);
        ofCircle(150, 150, 100);
        
        camOutFbo.end();
        
        camOutFbo.readToPixels(outPixels);*/
        //image.setFromPixels(outPixels, grabber.getWidth(), grabber.getHeight());
        
        //sender.sendFrame(outPixels); //
        
        
        
       /* if(newframes > 20) {
            newframes = 0;
        }*/
    //}
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    //sender.sendFrame(grabber.getPixelsRef());
    
    //outputFbo.begin();
    //outputFbo.end();
    ofSetColor(255, 255, 255, 255);
    //grabber.draw(0, 0);
    if(canon.isLiveViewActive()) {
        
        camFbo.begin();
        canon.drawLiveView();
        camFbo.end();
        
        camOutFboHQ.begin();
        camFbo.draw(roi.center);
        camOutFboHQ.end();
        
        camOutFboLQ.begin();
        camFbo.draw(0,0,camOutFboLQ.getWidth(),camOutFboLQ.getHeight());
        camOutFboLQ.end();
    }
    
    camOutFboLQ.readToPixels(outPixelsLQ);
    lqsender.sendFrame(outPixelsLQ);
    
    float timePerFrame = 1.0 / hqFrameRate;
    float currentTime = ofGetElapsedTimef();
    if (currentTime - lastTime > timePerFrame){
        
        camOutFboHQ.readToPixels(outPixelsHQ);
        hqsender.sendFrame(outPixelsHQ);
        lastTime = currentTime;
    }  
    
    camFbo.draw(0,0,streamWidth/2, streamHeight/2);
    camOutFboHQ.draw(0,streamHeight/2,camOutFboHQ.getWidth(),camOutFboHQ.getHeight());
    
    ofPushMatrix();
    
    ofTranslate(streamWidth/2,  0);
    lqreceiver.draw(0, 0, streamWidth/2  , streamHeight/2 );
    ofSetColor(255,255,255,255);
    hqreceiver.draw(0, 0, roiMaxRadius, roiMaxRadius);
    
    ofPopMatrix();

    //cout<<receiver.isConnected()<<endl;
    
    
    

}

void ofApp::exit(){
    canon.endLiveView();
    //receiver.close();
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
