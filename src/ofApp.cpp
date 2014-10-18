#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofEnableSmoothing();

    syphonOut.setName("Portal");
    
    localRoi = new RegionOfInterest();
    remoteRoi =  new RegionOfInterest();
    
    localRoi->local = true;
    remoteRoi->local = false;
    
    ofSetCircleResolution(140);

    //ofLogLevel(OF_LOG_SILENT);
    ofSetFrameRate(30);
    
    streamWidth = 1056;
    streamHeight = 704;
    
    roiMaxRadius = 200;
    
    localRoi->center = ofVec2f(streamWidth,streamHeight);
    localRoi->radius = 200;
    localRoi->zoom = 1.09;
    
    localRoi->centerFilter.setFc(0.04);
    localRoi->centerFilter.setType(OFX_BIQUAD_TYPE_LOWPASS);
    
    localRoi->highPass.setFc(0.04);
    localRoi->highPass.setQ(0.44);
    localRoi->highPass.setType(OFX_BIQUAD_TYPE_BANDPASS);
    
    remoteRoi->center = ofVec2f(streamWidth,streamHeight);
    remoteRoi->radius = 200;
    remoteRoi->zoom = 1.09;
    
    remoteRoi->centerFilter.setFc(0.05);
    remoteRoi->centerFilter.setType(OFX_BIQUAD_TYPE_LOWPASS);
    
    remoteRoi->highPass.setFc(0.1);
    //remoteRoi->highPass.setQ(0.44);
    remoteRoi->highPass.setType(OFX_BIQUAD_TYPE_BANDPASS);
    
    
    int internalFormat = GL_RGB8;
    
#ifdef USE_WEBCAM
    grabber.initGrabber(streamWidth, streamHeight);
#endif

#ifdef USE_BLACK_MAGIC
    blackMagicCam.setup(1920, 1080, 59.9400000000);
#endif
    
#ifdef USE_CANON_LIVEVIEW
    canon.start();
#endif
    
    if(ofIsGLProgrammableRenderer()){
        shaderBlurX.load("shadersGL3/shaderBlurX");
        shaderBlurY.load("shadersGL3/shaderBlurY");
    }else{
        shaderBlurX.load("shadersGL2/shaderBlurX");
        shaderBlurY.load("shadersGL2/shaderBlurY");
        shaderDesaturate.load("shadersGL2/desaturate");
        
        //deinterlace.load("shadersGL2/deinterlace");
        
    }
    
    fboBlurOnePass.allocate(streamWidth, streamHeight);
    fboBlurTwoPass.allocate(streamWidth, streamHeight);
    
    camFbo.allocate(streamWidth, streamHeight, internalFormat);
    camOutFboHQ.allocate(roiMaxRadius*2, roiMaxRadius*2, internalFormat);
    camOutFboLQ.allocate(streamWidth/8, streamHeight/8, internalFormat);
    
    outFbo.allocate(streamWidth, streamHeight, internalFormat);
    
#ifdef USE_SENDER
    hqsender.setup(roiMaxRadius*2, roiMaxRadius*2, REMOTE_HOST, HIGH_QUALITY_STREAM_PORT, "slow", "zerolatency");//, "placebo", "zerolatency");
    lqsender.setup(streamWidth/8, streamHeight/8, REMOTE_HOST, LOW_QUALITY_STREAM_PORT, "veryslow", "zerolatency");//, "ultrafast", "zerolatency");

    hqreceiver.setup(HIGH_QUALITY_STREAM_PORT);
    lqreceiver.setup(LOW_QUALITY_STREAM_PORT);
    
#endif
    
    flowSolver.setup(outFbo.getWidth(), outFbo.getHeight(), 0.35, 5, 10, 1, 3, 2.25, false, false);
   
    oscSender.setup(REMOTE_HOST, OSC_DATA_PORT);
    oscReceiver.setup(OSC_DATA_PORT);

    
    camFbo.begin();
    ofBackground(0,255,0);
    camFbo.end();
    
    outFbo.begin();
    ofBackground(255,0,0);
    outFbo.end();
    
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
    
   
#ifdef USE_SENDER
    lqreceiver.update();
    hqreceiver.update();
#endif
    
#ifdef USE_WEBCAM
    grabber.update();
    if(grabber.isInitialized() && grabber.isFrameNew()){
        camFbo.begin();
        grabber.draw(0, 0, camFbo.getWidth(), camFbo.getHeight());
        camFbo.end();
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
        camFbo.begin();
        canon.drawLiveView();
        camFbo.end();
    }
#endif
    //flowSolver.update(grabber);
    
    
    ofPoint vel  = ofPoint(0.0,0.0);
    float totalVel = 0.0;
    int count = 0;
    ofPoint tmpCenter = ofPoint(0.0,0.0);
   /*for(int x = 0; x < grabber.width; x++){
        for(int y = 0; y < grabber.height; y++){
            vel = flowSolver.getVelAtPixel(x, y);
            totalVel += vel.length();

            if(vel.length()>5.0f)
            {
                
                tmpCenter += ofPoint(x,y);
                count ++;
            }
        }
    }
    if(count>grabber.width*grabber.height*0.01f) // at least 5 percent
    {
        tmpCenter /= count;
        tmpCenter.x /= grabber.width;
        tmpCenter.y /= grabber.height;
        
        center = tmpCenter;
        
        cout << center.x << "   ,   " <<  center.y << endl;
    }
    */
    
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

    
    
    timePerFrame = 1.0 / hqFrameRate;
    if (currentTime - hqFrameLastTime > timePerFrame){
        
        camOutFboHQ.readToPixels(outPixelsHQ);
#ifdef USE_SENDER
        hqsender.sendFrame(outPixelsHQ);
#endif
        hqFrameLastTime = currentTime;
    }
    
    localRoi->rawCenter = ofVec2f(center.x*camFbo.getWidth(), center.y*camFbo.getHeight());
    localRoi->center = localRoi->centerFilter.update(localRoi->rawCenter);
    localRoi->highPass.update(ofVec2f(center.x*camFbo.getWidth(), center.y*camFbo.getHeight()));

  
    
    localRoi->alpha = ofMap((abs(localRoi->highPass.value().x)+abs(localRoi->highPass.value().y))/2, 0, 50, 240, 0, true);
    localRoi->zoom = ofMap(localRoi->alpha, 0, 240, 0.6, 1.15, true);
    localRoi->radius = ofMap(localRoi->alpha, 0, 190, roiMaxRadius*0.8, roiMaxRadius, true);
    
    // send OSC data

    timePerFrame = 1.0 / oscUpdateRate;
    if (currentTime - oscUpdateLastTime > timePerFrame){
        
        ofxOscMessage m;
        m.setAddress("/roi/center");
        m.addFloatArg(localRoi->rawCenter.x);
        m.addFloatArg(localRoi->rawCenter.y);
        oscSender.sendMessage(m);
        
        oscUpdateLastTime = currentTime;
    }
    
    while(oscReceiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        
        // check for mouse moved message
        if(m.getAddress() == "/roi/center"){
            // both the arguments are int32's
            remoteRoi->center = remoteRoi->centerFilter.update(ofVec2f(m.getArgAsFloat(0),m.getArgAsFloat(1)));
            remoteRoi->highPass.update(remoteRoi->center);
            
            remoteRoi->alpha = ofMap((abs(remoteRoi->highPass.value().x)+abs(remoteRoi->highPass.value().y))/2, 0, 50, 240, 0, true);
            remoteRoi->zoom = ofMap(remoteRoi->alpha, 0, 240, 0.6, 1.15, true);
            remoteRoi->radius = ofMap(remoteRoi->alpha, 0, 190, roiMaxRadius*0.8, roiMaxRadius, true);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetFullscreen(fullscreen);

    ofBackground(20, 20, 20);

    ofSetColor(ofColor::white);
    
    float blur = 2.2f;
    
#ifdef USE_BLACK_MAGIC
    camFbo.begin();
    ofPushMatrix();
    ofTranslate(camFbo.getWidth()/2, camFbo.getHeight()/2);
    ofScale(1.18,1.18);
    ofTranslate(-camFbo.getWidth()/2, -camFbo.getHeight()/2);
    
    blackMagicCam.getColorTexture().draw(0, 0, camFbo.getWidth(), camFbo.getHeight());
    ofPopMatrix();
    
    camFbo.end();
#endif

    
    //ofSetWindowTitle(ofToString(ofGetFrameRate()));
    ofSetColor(255, 255, 255, 255);
    
        camOutFboHQ.begin();
        ofBackground(0,0,0);
        ofTranslate(-localRoi->center+roiMaxRadius);
        camFbo.draw(0,0);
        camOutFboHQ.end();
        
        camOutFboLQ.begin();
        shaderDesaturate.begin();
        camFbo.draw(0,0,camOutFboLQ.getWidth(),camOutFboLQ.getHeight());
        shaderDesaturate.end();
        camOutFboLQ.end();
    
    camFbo.draw(0,0,streamWidth/2, streamHeight/2);
    camOutFboHQ.draw(0,streamHeight/2,camOutFboHQ.getWidth(),camOutFboHQ.getHeight());
    
    ofPushMatrix(); {
        ofScale(0.5,0.5);
        ofNoFill();
        ofRect(localRoi->center.x - roiMaxRadius, localRoi->center.y - roiMaxRadius, roiMaxRadius*2, roiMaxRadius*2);
        ofCircle(localRoi->center, localRoi->radius);
        ofFill();
    } ofPopMatrix();
    
    fboBlurOnePass.begin();
    
    shaderBlurX.begin();
    shaderBlurX.setUniform1f("blurAmnt", blur);
    if(lqreceiver.isConnected()) {
        lqreceiver.draw(0, 0, outFbo.getWidth(), outFbo.getHeight());
    } else {
        ofBackground(255, 0, 0);
    }
    shaderBlurX.end();
    fboBlurOnePass.end();
    
    fboBlurTwoPass.begin();
    shaderBlurY.begin();
    shaderBlurY.setUniform1f("blurAmnt", blur);
    fboBlurOnePass.draw(0, 0);
    shaderBlurY.end();
    fboBlurTwoPass.end();
    
    outFbo.begin();
    
    camFbo.draw(0,0,outFbo.getWidth(),outFbo.getHeight());
    
    ofSetColor(255,255,255,fadeRemote*255);
    fboBlurTwoPass.draw(0, 0);
    
    flowSolver.drawColored(camFbo.getWidth(), camFbo.getHeight(), 10, 3);
    
    ofFill();
    ofCircle(center.x*camFbo.getWidth() , center.y*camFbo.getHeight(), 50);
    
    //ofScale(0.5,0.5);
    //hqreceiver.draw(roi.center - roiMaxRadius);
    //hqreceiver.getTextureReference().setAlphaMask(<#ofTexture &mask#>)

    ofSetColor(255,255,255,255);

    
#ifdef USE_SENDER
    ofPushMatrix();{
    ofSetColor(255,255,255,remoteRoi->alpha);
    
    if(hqreceiver.isConnected()) {
    hqreceiver.getTextureReference().bind();

    //ofCircle(localRoi->center*ofVec2f(streamWidth/2, streamHeight/2), localRoi->radius*streamHeight);
    ofTranslate(remoteRoi->center);
    ofScale(remoteRoi->zoom, remoteRoi->zoom);
    
    glBegin(GL_POLYGON);
    for(int i = 0; i < NormCirclePts.size(); i++){
        glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
        glVertex2f( NormCirclePts[i].x * remoteRoi->radius,  NormCirclePts[i].y * remoteRoi->radius);
    }
    glEnd();
    
    hqreceiver.getTextureReference().unbind();
    }
    
    }ofPopMatrix();
#endif
        
    outFbo.end();
    
    
    if(debugView) {

        ofPushMatrix();{
        ofSetColor(255,255,255,255);

        ofTranslate(streamWidth/2,  0);
        ofScale(0.9,0.9);
        outFbo.draw(0,0);
    
    
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
