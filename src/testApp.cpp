#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    streamerSend = new ofxStreamerSender();
    streamerSend->setup(640, 480, "127.0.0.1", 8888);
    
    streamerRecv = new ofxStreamerReceiver();
    streamerRecv->setup(8888);
    
    grabber = new ofVideoGrabber();
    grabber->initGrabber(640, 480);
    
    data = (unsigned char*) malloc(sizeof(char)* 640 * 480 * 3*10);

}

//--------------------------------------------------------------
void testApp::update(){
    grabber->update();
    
    if(grabber->isFrameNew()){
        
        ofBuffer buffer;
        buffer.set((char*)data, 640 * 480 * 3);
        
        inputImage.setFromPixels(data, 640, 480, OF_IMAGE_COLOR);
        
        streamerSend->encodeFrame(grabber->getPixels(),  640 * 480 * 3);
        streamerSend->sendFrame();
    }
    
    
    streamerRecv->update();

    
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