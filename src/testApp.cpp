#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
//    string serverIp = "jive.local";
    string serverIp = "127.0.0.1";
    int serverPort = 8000;
    serverListenPort = ofRandom(6001,6999);
    
    streamerSend = new ofxStreamerSender();
    
    streamerRecv = new ofxStreamerReceiver();
    
    grabber = new ofVideoGrabber();
    grabber->initGrabber(640, 480);
    
    data = (unsigned char*) malloc(sizeof(char)* 640 * 480 * 3*10);
    
    
    oscRecvServer.setup(serverListenPort);
    oscSendServer.setup(serverIp, serverPort);
    
    
    ofxOscMessage msg;
    msg.setAddress("/hello");
    msg.addIntArg(serverListenPort);
    oscSendServer.sendMessage(msg);

    
}

void testApp::connectToRemote(string remoteIp, int remotePort){
    cout<<"Connect to other client on "<<remoteIp<<":"<<remotePort<<endl;
    
    oscSend.setup(remoteIp, remotePort);
    
    streamerSend->setup(640, 480, remoteIp, remotePort+1);

}

void testApp::listenOnPort(int _remotePort){
    remotePort = _remotePort;
    
    cout<<"Listen on port "<<_remotePort<<endl;

    oscRecv.setup(remotePort);
    streamerRecv->setup(remotePort+1);

}

//--------------------------------------------------------------
void testApp::update(){
    
    ofxOscMessage msg;
    msg.setAddress("/hello");
    msg.addIntArg(serverListenPort);

/*    ofxOscBundle bundle;
    bundle.addMessage(msg);

    oscSendServer.sendBundle(bundle);
*/
    oscSendServer.sendMessage(msg);
    
    
    ofSleepMillis(100);

    grabber->update();
    
    //   if(grabber->isFrameNew()){
    
    ofBuffer buffer;
    buffer.set((char*)data, 640 * 480 * 3);
    
    inputImage.setFromPixels(data, 640, 480, OF_IMAGE_COLOR);
    
    streamerSend->encodeFrame(grabber->getPixels(),  640 * 480 * 3);
    streamerSend->sendFrame();
    // }
    
    
    streamerRecv->update();
    
    while(oscRecv.hasWaitingMessages()){
        ofxOscMessage msg;
        oscRecv.getNextMessage(&msg);
        
        cout<<msg.getAddress()<<"  "<<msg.getRemotePort()<<endl;
    }
    
    while(oscRecvServer.hasWaitingMessages()){
        ofxOscMessage msg;
        oscRecv.getNextMessage(&msg);

        cout<<"Server says: "<<msg.getAddress()<<endl;

        if(msg.getAddress().compare("/setRemote") == 0){
            connectToRemote(msg.getArgAsString(0), msg.getArgAsInt32(1));
        }
        
        if(msg.getAddress().compare("/setPort") == 0){
            listenOnPort(msg.getArgAsInt32(0));

        }

    }
    
    
}

//--------------------------------------------------------------
void testApp::draw(){
    grabber->draw(0, 0, 640, 480);
    
    streamerRecv->draw(640, 0, 640, 480);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

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