#pragma once

#include "ofMain.h"
#include "pong.h"

#include "ofxOsc.h"
#include "ofxSyphon.h"
#include "ofxStreamer.h"
#include "ofxKinectSimpleTracker.h"
#include "ofxUI.h"

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    
    void connectToRemote(string ip, int port);
    void listenOnPort(int localPort);
    
    ofxStreamerReceiver * streamerRecv;
    ofxStreamerSender * streamerSend;
    
    ofImage inputImage;
    unsigned char * data ;

    ofVideoGrabber * grabber;
    
    ofxOscReceiver oscRecv;
    ofxOscSender oscSend;

    ofxOscReceiver oscRecvServer;
    ofxOscSender oscSendServer;
    
    int clientListenPortStart;
    int clientSendPortStart;
    string clientIp;
    
    int serverListenPort;
    string serverIp;
    
    bool serverConnected;
    bool clientConnected;
    
    ofxKinectSimpleTracker tracker;
    
    ofxUICanvas *gui;
	void guiEvent(ofxUIEventArgs &e);

    // Syphon
    bool useSyphon;
    ofxSyphonServer thisCamSy;
    ofxSyphonServer remoteCamSy;

    Pong pong;
};
