#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    string serverIp = "jive.local";
//  serverIp = "127.0.0.1";
    int serverPort = 8000;
    serverListenPort = ofRandom(6001,6999);
    
    //Server connection
    oscRecvServer.setup(serverListenPort);
    oscSendServer.setup(serverIp, serverPort);
    
    //Establish connection
    ofxOscMessage msg;
    msg.setAddress("/hello");
    msg.addIntArg(serverListenPort);
    oscSendServer.sendMessage(msg);

    //Video
    streamerSend = new ofxStreamerSender();
    streamerRecv = new ofxStreamerReceiver();
    
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    grabber = new ofVideoGrabber();
    grabber->listDevices();
    grabber->initGrabber(640, 480);

    
    data = (unsigned char*) malloc(sizeof(char)* 640 * 480 * 3*10);

    ofSetFrameRate(50);
    ofSetWindowTitle("Portalen");
    
    // Syphon
    useSyphon = true;
    
    ofSetFrameRate(50);
    
    tracker.open();
    
    
    
    gui = new ofxUICanvas();
    gui->setFont("NewMedia Fett.ttf");
    gui->setHeight(ofGetHeight());
	
    gui->addLabel("PORTALEN", OFX_UI_FONT_LARGE);
    gui->addSpacer();
    gui->addSlider("WORLD ROTATE X", -90, 90, &tracker.rotateWorldX);
    gui->addSlider("WORLD TRANSLATE X", -640, 640, &tracker.translateWorldX);
    gui->addSlider("WORLD TRANSLATE Y", -1640, 1640, &tracker.translateWorldY);
    gui->addSlider("WORLD TRANSLATE Z", -2000, 0, &tracker.translateWorldZ);

    gui->addSpacer();

    gui->addSlider("FLOOR LEVEL", -2000, 5000, &tracker.floorlevel);
    gui->addSlider("ROOF LEVEL", -2000, 5000, &tracker.rooflevel);

    gui->loadSettings("settings.xml");


    gui->setColorBack(ofColor(255,100));
    gui->setWidgetColor(OFX_UI_WIDGET_COLOR_BACK, ofColor(255,100));

    if (useSyphon) {
        thisCamSy.setName("Portalen: Camera");
        remoteCamSy.setName("Portalen: Remote Camera");
    }
    
    pong.setup();

}

void testApp::guiEvent(ofxUIEventArgs &e){
    
}

void testApp::connectToRemote(string ip, int port){
    clientSendPortStart = port;
    clientIp = ip;
    
    cout<<"Connect to other client on "<<clientIp<<":"<<clientSendPortStart<<endl;
    
    oscSend.setup(clientIp, clientSendPortStart);
    streamerSend->setup(640, 480, clientIp, clientSendPortStart+1);

}

void testApp::listenOnPort(int port){
    clientListenPortStart = port;
    
    cout<<"Listen on port "<<clientListenPortStart<<endl;

    oscRecv.setup(clientListenPortStart);
    streamerRecv->setup(clientListenPortStart+1);

}

//--------------------------------------------------------------
void testApp::update(){
    
    //tracker.rotateWorldX = sin(ofGetElapsedTimeMillis()/1000.0)*30;
    tracker.update();
    grabber->update();
    
   // if(grabber->isFrameNew()){
        
        ofBuffer buffer;
        buffer.set((char*)data, 640 * 480 * 3);
        
        inputImage.setFromPixels(data, 640, 480, OF_IMAGE_COLOR);
        
        streamerSend->encodeFrame(grabber->getPixels(),  640 * 480 * 3);
        streamerSend->sendFrame();
    //}
    
    
    streamerRecv->update();
    
    while(oscRecv.hasWaitingMessages()){
        ofxOscMessage msg;
        oscRecv.getNextMessage(&msg);
        
        cout<<"Client says: "<<msg.getAddress()<<endl;
    }
    
    while(oscRecvServer.hasWaitingMessages()){
        ofxOscMessage msg;
        oscRecvServer.getNextMessage(&msg);

        cout<<"Server says: "<<msg.getAddress()<<endl;

        if(msg.getAddress().compare("/setRemote") == 0){
            string cIp = msg.getArgAsString(0);
            if(cIp.compare("127.0.0.1") == 0 && msg.getRemoteIp().compare("127.0.0.1") != 0){
                cIp = msg.getRemoteIp();
            }
            
            connectToRemote(cIp, msg.getArgAsInt32(1));
        }
        
        if(msg.getAddress().compare("/setPort") == 0){
            listenOnPort(msg.getArgAsInt32(0));

        }
    }
    
    pong.update();
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0, 0, 0);
    ofSetColor(255, 255, 255);
 //   grabber->draw(220, 0, 640*0.5, 480*0.5);
    tracker.draw(220, 480*0.5, 640*0.5, 480*0.5);
    
  /*  ofDrawBitmapString("FPS: "+ofToString(ofGetFrameRate(),0), 5,15);
   
    ofDrawBitmapString("FPS: "+ofToString(ofGetFrameRate(),0), 5,15);
    ofDrawBitmapString("StreamSend FPS: "+ofToString(streamerSend->frameRate,0), 5,30);
    ofDrawBitmapString("Server IP: "+serverIp, 5,45);
    ofDrawBitmapString("Listen Port: "+ofToString(clientListenPortStart), 5,60);
    */
    streamerRecv->draw(640, 0, 640, 480);
   /*
    ofDrawBitmapString("StreamRecv FPS: "+ofToString(streamerRecv->frameRate,0), 640+10,15);
    ofDrawBitmapString("Client: "+clientIp+":"+ofToString(clientSendPortStart), 640+10,30);
    */
    if (useSyphon) {
        
        thisCamSy.publishTexture(&grabber->getTextureReference());
        if(streamerRecv->isConnected()) {
            remoteCamSy.publishTexture(&streamerRecv->getTextureReference());
        }
    }
    
    pong.fbo.draw(220, 0, 640*0.5, 480*0.5);

}

void testApp::exit(){
    gui->saveSettings("settings.xml");
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    pong.playerPosition = ofPoint((float)x/ofGetWidth(),(float)y/ofGetHeight());
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}