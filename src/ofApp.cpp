#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetCircleResolution(100);

    //ofLogLevel(OF_LOG_SILENT);
    ofSetFrameRate(30);
    
    
    streamWidth = 1056;
    streamHeight = 704;
    
    
    roiMaxRadius = 200;
    
    roi.center = ofVec2f(streamWidth,streamHeight);
    roi.radius = 200;
    roi.zoom = 1.15;
    
    roiCenterFilter.setFc(0.04);
    roiCenterFilter.setType(OFX_BIQUAD_TYPE_LOWPASS);
    
#ifdef USE_WEBCAM
    grabber.initGrabber(streamWidth, streamHeight);
    int internalFormat = GL_RGB8;

    

#else

    canon.start();
    int internalFormat = GL_RGB8;

    
#endif

    
    camFbo.allocate(streamWidth, streamHeight, internalFormat);
    camOutFboHQ.allocate(roiMaxRadius*2, roiMaxRadius*2, internalFormat);
    camOutFboLQ.allocate(streamWidth/8, streamHeight/8, internalFormat);
    
    
    hqsender.setup(roiMaxRadius*2, roiMaxRadius*2, REMOTE_HOST, 1234);//, "placebo", "zerolatency");
    lqsender.setup(streamWidth/8, streamHeight/8, REMOTE_HOST, 1235);//, "ultrafast", "zerolatency");
    

    hqreceiver.setup(1234);
    lqreceiver.setup(1235);
    
    camFbo.begin();
    ofClear(0,0,0);
    camFbo.end();
    
    int numPts  = 64;
    float angle = 0.0;
    float step  = TWO_PI / (float)(numPts-1);
    
    
    for(int i = 0; i < numPts; i++){
        
        //get the -1 to 1 values - we will use these for drawing our pts.
        float px = cos(angle);
        float py = sin(angle);
        
        NormCirclePts.push_back( ofPoint(px, py) );
        
        //map the -1 to 1 range produced by cos/sin
        //to the dimensions of the image we need for our texture coords
        float tx = ofMap( px, -1.0, 1.0, 0, roiMaxRadius*2);
        float ty = ofMap( py, -1.0, 1.0, 0, roiMaxRadius*2);
        
        NormCircleCoords.push_back( ofPoint(tx, ty) );  
        
        angle += step;  
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    roi.center = roiCenterFilter.update(ofVec2f(mouseX*2, mouseY*2));
    
    lqreceiver.update();
    hqreceiver.update();
    
#ifdef USE_WEBCAM
    grabber.update();
    if(grabber.isInitialized() && grabber.isFrameNew()){
    
        camFbo.begin();
        grabber.draw(0, 0, camFbo.getWidth(), camFbo.getHeight());
        camFbo.end();
    }

    
#else
    if(!canon.isLiveViewActive() && canon.isSessionOpen()) {
        canon.startLiveView();
    }
#endif
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    

    ofSetWindowTitle(ofToString(ofGetFrameRate()));

    ofSetColor(255, 255, 255, 255);
    
    
#ifdef USE_WEBCAM
#else
    if(canon.isLiveViewActive()) {
        camFbo.begin();
        canon.drawLiveView();
        camFbo.end();
    }
#endif

    
        camOutFboHQ.begin();
        ofBackground(0,0,0);
        ofTranslate(-roi.center+roiMaxRadius);
        camFbo.draw(0,0);
        //camFbo.getTextureReference().drawSubsection(0, 0, roiMaxRadius, roiMaxRadius, 0, 0);
        camOutFboHQ.end();
        
        camOutFboLQ.begin();
        camFbo.draw(0,0,camOutFboLQ.getWidth(),camOutFboLQ.getHeight());
        camOutFboLQ.end();
    
    
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
    ofScale(0.5,0.5);
    ofNoFill();
    ofRect(roi.center.x - roiMaxRadius, roi.center.y - roiMaxRadius, roiMaxRadius*2, roiMaxRadius*2);
    ofCircle(roi.center, roi.radius);
    ofFill();
    ofPopMatrix();
    
    ofPushMatrix();
    
    ofTranslate(streamWidth/2,  0);
    
    lqreceiver.draw(0, 0, streamWidth/2  , streamHeight/2 );
    ofSetColor(255,255,255,255);
    
    ofScale(0.5,0.5);
    //hqreceiver.draw(roi.center - roiMaxRadius);
    
    hqreceiver.getTextureReference().bind();
    
    //ofCircle(roi.center*ofVec2f(streamWidth/2, streamHeight/2), roi.radius*streamHeight);
    ofTranslate(roi.center);
    ofScale(roi.zoom, roi.zoom);

    glBegin(GL_POLYGON);
    for(int i = 0; i < NormCirclePts.size(); i++){
        glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
        glVertex2f( NormCirclePts[i].x * roi.radius,  NormCirclePts[i].y * roi.radius);
    }
    glEnd();
    
    hqreceiver.getTextureReference().unbind();
    
    
    
    ofPopMatrix();

    
}

void ofApp::exit(){
    
#ifdef USE_WEBCAM
#else
    canon.endLiveView();
#endif
    
    hqreceiver.close();
    lqreceiver.close();
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
