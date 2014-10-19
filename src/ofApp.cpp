#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofEnableSmoothing();
    //ofEnableAlphaBlending();
    
    syphonOut.setName("Portal");
    
    localRoi = new RegionOfInterest();
    remoteRoi =  new RegionOfInterest();
    
    localRoi->local = true;
    remoteRoi->local = false;
    
    ofSetCircleResolution(140);
    
    //ofLogLevel(OF_LOG_SILENT);
    ofSetFrameRate(30);
    
    flowMagnitude = 0.0f;
    streamWidth = 1056;
    streamHeight = 704;
    
    roiMaxRadius = 200;
    
    localRoi->center = ofVec2f(streamWidth,streamHeight);
    localRoi->radius = 200;
    localRoi->zoom = 1.09;
    
    localRoi->centerFilter.setFc(0.02);
    localRoi->centerFilter.setType(OFX_BIQUAD_TYPE_LOWPASS);
    
    localRoi->highPass.setFc(0.04);
    localRoi->highPass.setQ(0.44);
    localRoi->highPass.setType(OFX_BIQUAD_TYPE_BANDPASS);
    
    remoteRoi->center = ofVec2f(streamWidth/2,streamHeight/2);
    remoteRoi->radius = 200;
    remoteRoi->zoom = 1.09;
    
    remoteRoi->centerFilter.setFc(0.02);
    remoteRoi->centerFilter.setType(OFX_BIQUAD_TYPE_LOWPASS);
    
    remoteRoi->highPass.setFc(0.1);
    //remoteRoi->highPass.setQ(0.44);
    remoteRoi->highPass.setType(OFX_BIQUAD_TYPE_BANDPASS);
    
    int internalFormat = GL_RGB8;
    
    // Initiate input
#ifdef USE_WEBCAM
    grabber.initGrabber(streamWidth, streamHeight);
#endif
    
#ifdef USE_BLACK_MAGIC
    blackMagicCam.setup(1920, 1080, 59.9400000000);
#endif
    
#ifdef USE_CANON_LIVEVIEW
    canon.start();
#endif
    
    
    // Load shaders
    if(ofIsGLProgrammableRenderer()){
        shaderBlurX.load("shadersGL3/shaderBlurX");
        shaderBlurY.load("shadersGL3/shaderBlurY");
    }else{
        shaderBlurX.load("shadersGL2/shaderBlurX");
        shaderBlurY.load("shadersGL2/shaderBlurY");
        shaderDesaturate.load("shadersGL2/desaturate");
        shaderBlend.load("shadersGL2/blend");
    }
    
    
    fboBlurOnePass.allocate(streamWidth, streamHeight);
    fboBlurTwoPass.allocate(streamWidth, streamHeight);
    
    camFbo.allocate(streamWidth, streamHeight, internalFormat);
    camOutFboHQ.allocate(roiMaxRadius*2, roiMaxRadius*2, internalFormat);
    camOutFboLQ.allocate(streamWidth/8, streamHeight/8, internalFormat);
    
    // Optica flow setup
    flowFbo.allocate(200, 200, internalFormat);
    flowFboBlurX.allocate(flowFbo.getWidth(), flowFbo.getHeight(), internalFormat);
    flowFboBlurY.allocate(flowFbo.getWidth(), flowFbo.getHeight(), internalFormat);
    flowBlurAmount = 0.5;
    
    outFbo.allocate(streamWidth, streamHeight, internalFormat);
    
    // alpha mask
    portalFbo.allocate(streamWidth, streamHeight, internalFormat);
    portalImage.allocate(streamWidth, streamHeight);
    // outfbo with above alpha mask applied
    blendFbo.allocate(streamWidth, streamHeight, internalFormat);
    
    flowSolver.setup(flowFbo.getWidth(), flowFbo.getHeight(), 0.45, 3, 5, 5, 6, 4.25, false, true);
    
    
#ifdef USE_SENDER
    hqsender.setup(roiMaxRadius*2, roiMaxRadius*2, REMOTE_HOST, HIGH_QUALITY_STREAM_PORT, "slow", "zerolatency");//, "placebo", "zerolatency");
    lqsender.setup(streamWidth/8, streamHeight/8, REMOTE_HOST, LOW_QUALITY_STREAM_PORT, "veryslow", "zerolatency");//, "ultrafast", "zerolatency");
    
    hqreceiver.setup(HIGH_QUALITY_STREAM_PORT);
    lqreceiver.setup(LOW_QUALITY_STREAM_PORT);
#endif
    
    
    oscSender.setup(REMOTE_HOST, OSC_DATA_PORT);
    oscSenderAudio.setup("127.0.0.1", 12345);
    oscReceiver.setup(OSC_DATA_PORT);
    
    
    camFbo.begin(); {
    ofBackground(0,255,0);
    } camFbo.end();
    
    outFbo.begin(); {
    ofBackground(255,0,0);
    } outFbo.end();
    
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
void ofApp::sendStreams(){
    
    // Stream low quality frames
    float currentTime = ofGetElapsedTimef();
    float timePerFrame = 1.0 / lqFrameRate;
    if (currentTime - lqFrameLastTime > timePerFrame){
        camOutFboLQ.readToPixels(outPixelsLQ);
#ifdef USE_SENDER
        lqsender.sendFrame(outPixelsLQ);
#endif
        lqFrameLastTime = currentTime;
    }
    
    // Stream high quality frames
    timePerFrame = 1.0 / hqFrameRate;
    if (currentTime - hqFrameLastTime > timePerFrame){
        
        camOutFboHQ.readToPixels(outPixelsHQ);
#ifdef USE_SENDER
        hqsender.sendFrame(outPixelsHQ);
#endif
        hqFrameLastTime = currentTime;
    }
    
    
}


void ofApp::receiveOsc(){
    while(oscReceiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        
        // check for mouse moved message
        if(m.getAddress() == "/roi/center"){
            // both the arguments are int32's
            remoteActiveRegionOfInterest = m.getArgAsInt32(2)==1;
            remoteRoi->center = remoteRoi->centerFilter.update(ofVec2f(m.getArgAsFloat(0),m.getArgAsFloat(1)));
            remoteRoi->highPass.update(remoteRoi->center);
            
            remoteRoi->alpha = ofMap((abs(remoteRoi->highPass.value().x)+abs(remoteRoi->highPass.value().y))/2, 0, 50, 240, 0, true);
            remoteRoi->zoom = ofMap(remoteRoi->alpha, 0, 240, 0.6, 1.15, true);
            remoteRoi->radius = ofMap(remoteRoi->alpha, 0, 190, roiMaxRadius*0.8, roiMaxRadius, true);
        }
    }
}

//--------------------------------------------------------------
void ofApp::sendOsc(){
    
    // send OSC data
    float currentTime = ofGetElapsedTimef();
    float timePerFrame = 1.0 / oscUpdateRate;
    if (currentTime - oscUpdateLastTime > timePerFrame){
        
        ofxOscMessage m;
        m.setAddress("/roi/center");
        
        m.addFloatArg(localRoi->rawCenter.x);
        m.addFloatArg(localRoi->rawCenter.y);
        m.addIntArg(activeRegionOfInterest?1:0);
        
        oscSender.sendMessage(m);
        
        oscUpdateLastTime = currentTime;
    }
    
    
}

//--------------------------------------------------------------
void ofApp::updateFlow(){
    
    ofPixels portalPixels;
    portalFbo.readToPixels(portalPixels);
    portalImage.setFromPixels(portalPixels);
    portalImage.dilate();
    portalImage.dilate();
    portalImage.dilate();
    portalImage.dilate();
    portalImage.erode();
    portalImage.erode();
    portalImage.blur();
    
    flowFbo.readToPixels(flowPixels);
    
    ofPoint vel  = ofPoint(0.0,0.0);
    float totalVel = 0.0;
    float totalVelOfActivePixels = 0.0;
    int count = 0;
    int countMovingPixels = 0;
    float totalHoriozontalFlow = 0.0;
    ofPoint tmpCenter = ofPoint(0.0,0.0);
    for(int x = 0; x < flowFbo.getWidth(); x++){
        for(int y = 0; y < flowFbo.getHeight(); y++){
            vel = flowSolver.getVelAtPixel(x, y);
            if(vel.length()>0.1) // measure average of all flow that is moving
            {
                totalVel += vel.length();
                countMovingPixels++;
                totalHoriozontalFlow += vel.x;
            }
            
            if(vel.length()>avgFlowMagnitude)
            {
                tmpCenter += ofPoint(x,y)*(vel.length()*vel.length());
                count ++;
                totalVelOfActivePixels +=(vel.length()*vel.length());
                
            }
        }
    }
    float oscFlowMagnitude = 0.0;
   
    if(countMovingPixels > 0)
    {
        avgFlowMagnitude = fmaxf(10.0, ofLerp(avgFlowMagnitude, totalVel/countMovingPixels, 0.05));
    }
    totalHoriozontalFlow = 0.5+ofClamp(totalHoriozontalFlow/5000.0,-0.5,0.5);
    oscFlowMagnitude = 0.5+ofClamp((15.0*count)/(flowFbo.getWidth()*flowFbo.getHeight()),0.0,1.0)*0.5;
    
    ofxOscMessage m;
    m.setAddress("/flowMagnitude");
    m.addFloatArg(oscFlowMagnitude);
    oscSenderAudio.sendMessage(m);
    
    ofxOscMessage m2;
    m2.setAddress("/flowHorizontal");
    m2.addFloatArg(totalHoriozontalFlow);
    oscSenderAudio.sendMessage(m2);
    
    ofxOscMessage m3;
    m3.setAddress("/MicrophoneVolume");
    m3.addFloatArg(flowMagnitude);
    oscSenderAudio.sendMessage(m3);

    if(count>flowFbo.getWidth()*flowFbo.getHeight()*0.01f) // at least some percent
    {
        flowMagnitude = ofLerp(flowMagnitude, 1.0, 0.5);
        tmpCenter /= totalVelOfActivePixels;
        tmpCenter.x /= flowFbo.getWidth();
        tmpCenter.y /= flowFbo.getHeight();
        
        center = tmpCenter;
        activeRegionOfInterest = true;
    }
    else
    {
        flowMagnitude = ofLerp(flowMagnitude, 0.0, 0.05);
        activeRegionOfInterest = false;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
#ifdef USE_SENDER
    lqreceiver.update();
    hqreceiver.update();
#endif
    
#ifdef USE_WEBCAM
    grabber.update();
    if(grabber.isInitialized() && grabber.isFrameNew()){
        camFbo.begin();{
            grabber.draw(0, 0, camFbo.getWidth(), camFbo.getHeight());
        }camFbo.end();
    }
#endif
    
#ifdef USE_BLACK_MAGIC
    blackMagicCam.update();
#endif
    
#ifdef USE_CANON_LIVEVIEW
    if(!canon.isLiveViewActive() && canon.isSessionOpen()) {
        canon.startLiveView();
    }
    
    if(canon.isLiveViewActive()) {
        camFbo.begin();{
            canon.drawLiveView();
        }camFbo.end();
    }
#endif
    
    updateFlow();
    sendStreams();
    sendOsc();
    receiveOsc();
    
    localRoi->rawCenter = ofVec2f(center.x*camFbo.getWidth(), center.y*camFbo.getHeight());
    
    // clamp width and height so the ROI is not grabbing pixels that doesnt exist.
    localRoi->rawCenter.x = ofClamp(localRoi->rawCenter.x, localRoi->radius, camFbo.getWidth()-localRoi->radius);
    
    localRoi->rawCenter.y = ofClamp(localRoi->rawCenter.y, localRoi->radius, camFbo.getHeight()-localRoi->radius);
    
    // filter the ROI location
    localRoi->center = localRoi->centerFilter.update(localRoi->rawCenter);
    
    // highpass filter for alpha and size
    localRoi->highPass.update(ofVec2f(center.x*camFbo.getWidth(), center.y*camFbo.getHeight()));
    
    localRoi->alpha = 100+ofMap((abs(localRoi->highPass.value().x)+abs(localRoi->highPass.value().y))/2, 0, 50, 240, 0, true);
    localRoi->zoom = ofMap(localRoi->alpha, 0, 240, 0.6, 1.15, true);
    localRoi->radius = ofMap(localRoi->alpha, 0, 190, roiMaxRadius*0.8, roiMaxRadius, true);
    
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofSetFullscreen(fullscreen);
    
    ofBackground(20, 20, 20);
    
    ofSetColor(ofColor::white);
    
    float blur = 2.2f;
    
#ifdef USE_BLACK_MAGIC
    camFbo.begin();{
        ofPushMatrix();{
            ofTranslate(camFbo.getWidth()/2, camFbo.getHeight()/2);
            ofScale(1.18,1.18);
            ofTranslate(-camFbo.getWidth()/2, -camFbo.getHeight()/2);
            blackMagicCam.getColorTexture().draw(0, 0, camFbo.getWidth(), camFbo.getHeight());
        }ofPopMatrix();
    }camFbo.end();
#endif
    
#ifdef USE_WEBCAM
#endif
    
#ifdef USE_CANON_LIVEVIEW
    if(canon.isLiveViewActive()) {
        camFbo.begin();{
            canon.drawLiveView();
        }camFbo.end();
    }
#endif
    
    
    ofSetColor(255, 255, 255, 255);
    
    camOutFboHQ.begin();{
        ofBackground(0,0,0);
        ofTranslate(-localRoi->center+roiMaxRadius);
        camFbo.draw(0,0);
    }camOutFboHQ.end();
    
    camOutFboLQ.begin();{
        shaderDesaturate.begin();{
            camFbo.draw(0,0,camOutFboLQ.getWidth(),camOutFboLQ.getHeight());
        }shaderDesaturate.end();
    }camOutFboLQ.end();
    
    
   /* // Blue lq receiver
    fboBlurOnePass.begin();{
        shaderBlurX.begin();{
            shaderBlurX.setUniform1f("blurAmnt", blur);
            if(lqreceiver.isConnected()) {
                lqreceiver.draw(0, 0, outFbo.getWidth(), outFbo.getHeight());
            } else {
                ofBackground(255, 0, 0);
            }
        }shaderBlurX.end();
    }fboBlurOnePass.end();
    
    fboBlurTwoPass.begin();{
        shaderBlurY.begin();{
            shaderBlurY.setUniform1f("blurAmnt", blur);
            fboBlurOnePass.draw(0, 0);
        }shaderBlurY.end();
    }fboBlurTwoPass.end();
    // done blurring lq receiver
    */
    
    /*
    portalFbo.begin();{
        ofSetColor(0, 0, 0,4);
        ofRect(0, 0, portalFbo.getWidth(), portalFbo.getHeight());
        ofSetColor(255, 255, 255, 255);
        
        ofSetLineWidth(85.0);
        ofSetColor(255, 255, 255, 100+155*flowMagnitude);
        
        flowSolver.draw(portalFbo.getWidth(), portalFbo.getHeight(),1.5,8);
        ofSetLineWidth(1.0);
        
        ofSetColor(255, 255, 255);
    }portalFbo.end();*/
    
    outFbo.begin();{
        
        //camFbo.draw(0,0,outFbo.getWidth(),outFbo.getHeight());
        //fboBlurTwoPass.draw(0, 0);

        ofSetColor(255,255,255,255);
        //TODO: Draw blur image> fboBlurTwoPass.draw(0, 0);
        lqreceiver.draw(outFbo.getWidth(), 0,-outFbo.getWidth(),outFbo.getHeight());
        
    }outFbo.end();
    
    blendFbo.begin();{
        shaderBlend.begin();{
            shaderBlend.setUniformTexture("imageMask", portalFbo.getTextureReference(), 1);
            shaderBlend.setUniformTexture("bg", outFbo.getTextureReference(), 2);
            
            camFbo.draw(0,0,camFbo.getWidth(),camFbo.getHeight());
        }shaderBlend.end();
    }blendFbo.end();
    
    
    outFbo.begin(); {
    
    ofSetColor(255,255,255,255);
        blendFbo.draw(0,0);
#ifdef USE_SENDER
    ofPushMatrix();{
        ofSetColor(255,255,255,remoteRoi->alpha);
        if(true || remoteActiveRegionOfInterest) //always true what
        {
            if(hqreceiver.isConnected()) {
                                
                hqreceiver.getTextureReference().bind();
                
                //ofCircle(localRoi->center*ofVec2f(streamWidth/2, streamHeight/2), localRoi->radius*streamHeight);
                ofTranslate(remoteRoi->center);
                ofScale(remoteRoi->zoom, remoteRoi->zoom);
                
                glBegin(GL_POLYGON); {
                    for(int i = 0; i < NormCirclePts.size(); i++){
                        glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
                        glVertex2f( NormCirclePts[i].x * remoteRoi->radius,  NormCirclePts[i].y * remoteRoi->radius);
                    }
                } glEnd();
                
                hqreceiver.getTextureReference().unbind();
            }
        }
        
    }ofPopMatrix();
#endif
    
} outFbo.end();
    
    flowFbo.begin();{
        camFbo.draw(0,0,flowFbo.getWidth(),flowFbo.getHeight());
    }flowFbo.end();
    
    if(debugView) {

        
        camFbo.draw(0,0,streamWidth/2, streamHeight/2);
        camOutFboHQ.draw(0,streamHeight/2,camOutFboHQ.getWidth(),camOutFboHQ.getHeight());
        
        ofPushMatrix(); {
            ofScale(0.5,0.5);
            ofNoFill();
            ofRect(localRoi->center.x - roiMaxRadius, localRoi->center.y - roiMaxRadius, roiMaxRadius*2, roiMaxRadius*2);
            ofCircle(localRoi->center, localRoi->radius);
            ofFill();
        } ofPopMatrix();
    
        

        ofPushMatrix();{
            ofSetColor(255,255,255,255);
            
            ofTranslate(streamWidth/2,  0);
            ofScale(0.9,0.9);
            
            camFbo.draw(0,0);
            //lqreceiver.draw(0,0,500,500);
            //outFbo.draw(0,0);
            //fboBlurOnePass.draw(0, 0,outFbo.getWidth(),outFbo.getHeight());
            //lqreceiver.draw(0,0,500,500);
            
        }ofPopMatrix();
    }
    
    if(enableSyphonOut) {
        syphonOut.publishTexture(&outFbo.getTextureReference());
    }
    
}

void ofApp::exit(){
    
#ifdef USE_WEBCAM
#endif
    
#ifdef USE_CANON_LIVEVIEW
    canon.endLiveView();
#endif
    
#ifdef USE_BLACK_MAGIC
    blackMagicCam.close();
#endif
    
#ifdef USE_SENDER
    hqreceiver.close();
    lqreceiver.close();
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == 'd') {
        debugView = !debugView;
    }
    
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
