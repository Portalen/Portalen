/*
 *  untitled.cpp
 *  cylinder2
 *
 *  Created by explab on 11/1/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxKinectSimpleTracker.h"

#define SCALE_FACTOR 0.1

ofxKinectSimpleTracker::ofxKinectSimpleTracker()
{
    kinect = new ofxKinect();
    kinect->init();
    
    
    default_x=0;
    default_y=0;
    
    gesture=0;
    
    //		kinect=k;
    
    elevation=0.4;
    azimuth=0;
    radius=0;
    offset=0;
    
    width=100;
    height=100;
    maxsize=300;
    minsize=40;
    
    rooflevel=1500;
    floorlevel=3200;
    
    //		rooflevel=2.0;
    //		floorlevel=3.0;
    
    apex=floorlevel;
    
    clientWidth=kinect->getWidth();
    clientHeight=kinect->getHeight();
    
    x=320;
    y=240;
    
    target_filter=width;
    gain=0.00001;
    active=0;
    
    fbo = new ofFbo();
    fbo->allocate(clientWidth, clientHeight);
}


bool ofxKinectSimpleTracker::open(){
    return kinect->open();
}

void ofxKinectSimpleTracker::close(){
    return kinect->close();
}

void ofxKinectSimpleTracker::draw(int x, int y, int w, int h){
    fbo->draw(x, y, w, h);
}

void ofxKinectSimpleTracker::update(){
    kinect->update();
    
    if(kinect->isFrameNew()){
//        memset(rotatedPixels, 0, 640*480*sizeof(int));
        std::fill(rotatedPixels, rotatedPixels+640*480, 0);
        
        for(int y=0;y<480;y++){
            for(int x=0;x<640;x++){
                if(kinect->getDistanceAt(x, y) > 0) {
                    
                    ofPoint cur = kinect->getWorldCoordinateAt(x, y);
                    cur += ofPoint(0,0, translateWorldZ);
                    cur += ofPoint(translateWorldX, translateWorldY, 0);
                    cur.rotate(rotateWorldX, 0, 0);
                    cur *= SCALE_FACTOR;
                    cur += ofPoint(320, 320, 0);
                    
             
                    if(cur.x > 0 && cur.x < 640 && cur.y > 0 && cur.y  < 480){
                        rotatedPixels[int(cur.x) + 640 * int(cur.y)] = cur.z;
                    }
                }
            }
        }

        /*
        if(x<width/2)
            x=width/2;
        if(x>clientWidth-width/2)
            x=clientWidth-width/2;
        if(y<height/2)
            y=height/2;
        if(y>clientHeight-height/2)
            y=clientHeight-height/2;
        */
        float vx=0;
        float vy=0;
        float z=0;
        int x0=x-(width/2);
        int y0=y-(height/2);
        
        apex=0;
        long n=0;
        
        for(int ty=y0;ty<(y0+height);ty++)
        {
            for(int tx=x0;tx<(x0+width);tx++)
            {
                
                //				ofPoint cur = kinect->getWorldCoordinateFor(tx, ty);
                
                ofPoint cur = ofPoint(tx, ty, rotatedPixels[tx+640*ty]);
            
            /*    cur -= ofPoint(320,240,0);
                cur.x /= SCALE_FACTOR;
                cur.y /= SCALE_FACTOR;
                cur += ofPoint(320,240,0);*/

                //                ofPoint cur;
                //                cur.z = depth->getPixelDepth(tx, ty);
                
                float g=0;
                
                if((cur.z>rooflevel)&&(cur.z<floorlevel))
                {
                    apex+=cur.z;
                    g=255;
                    n++;
                }
                
                vx+=(float)cur.x*g;
                vy+=(float)cur.y*g;
                z+=g;
            }
        }
        
        if (n)
            apex/=n;
        else
            apex=floorlevel;
        
        if(z>1000)
        {
            vx/=z;
            vy/=z;
            //        x=x*.1+vx*.9;
            //        y=y*.1+vy*.9;
            
            x=vx;
            y=vy;
            active=1;
        }
        else
        {
            x=x*.95+(clientWidth/2)*0.05;
            y=y*.95+(clientHeight/2)*0.05;
            active=0;
        }
        
        
        //		if((default_x<clientWidth)&&(default_y<clientHeight))
        //		{
        //			x=default_x;
        //			y=default_y;
        //			active=1;
        //		}
        //		else
        //        {
        //            x=x*.95+(clientWidth/2)*0.05;
        //            y=y*.95+(clientHeight/2)*0.05;
        //            active=0;
        //        }
        
        
        
        target_filter+=(z-(float)(height*width*128))*gain;
        
        //				if(z>)
        //		 target_filter+=gain;
        //		 else
        //		 {
        //		 target_filter-=gain;
        //		 }
        //
        if(target_filter>maxsize)
            target_filter=maxsize;
        if(target_filter<minsize)
            target_filter=minsize;
        
        width=height=target_filter;
        
        
        
        //		       if(x<width/2)
        //		 x=width/2;
        //		 if(x>clientWidth-width/2)
        //		 x=clientWidth-width/2;
        //		 if(y<height/2)
        //		 y=height/2;
        //		 if(y>clientHeight-height/2)
        //		 y=clientHeight-height/2;
        //
        
        /*float xp=x-clientWidth/2.0;
        float yp=y-clientHeight/2.0;
        
        azimuth=ofRadToDeg(atan2(xp,-yp))+offset;
        radius=sqrt(xp*xp+yp*yp);
        
        if(radius>(clientHeight/2.0)-width/2.0)
            radius=(clientHeight/2.0)-width/2.0;
        
        radius=radius/((clientHeight/2.0)-width/2.0);
        
        */
        renderFbo();
    }
}




void ofxKinectSimpleTracker::renderFbo(){
    
    fbo->begin();{
        ofClear(0);
        ofSetColor(255,255,255);
           //    kinect->drawDepth(0, 0, clientWidth, clientHeight);
        
        glPushMatrix();
        ofScale(1, 1, -1);
        //ofTranslate(translateWorldX, translateWorldY, -translateWorldZ); // center the points a bit
        
        glPointSize(2);
        glBegin(GL_POINTS);
        for(int y=0;y<kinect->height;y++){
            for(int x=0;x<kinect->width;x++){
                /*if(kinect->getDistanceAt(x, y) > 0) {
                    
                    ofPoint cur = kinect->getWorldCoordinateAt(x, y);
                    cur += ofPoint(0,0, translateWorldZ);
                    cur.rotate(rotateWorldX, 0, 0);
                    cur += ofPoint(translateWorldX+320, translateWorldY+320, 0);

                    if(cur.z < rooflevel){
                        ofSetColor(255,0,0);
                    } else if(cur.z > floorlevel){
                        ofSetColor(0, 255, 0);
                    } else {
                        ofSetColor(255);
                    }
                    glVertex3d(cur.x, cur.y, cur.z);
                }*/
                
                ofPoint cur = ofPoint(x, y, rotatedPixels[x + 640*y]);
                if(cur.z != 0){
                    if(cur.z < rooflevel){
                        ofSetColor(50,0,0);
                    } else if(cur.z > floorlevel){
                        ofSetColor(0, 50, 0);
                    } else {
                        ofSetColor(255);
                    }
                    
                    glVertex3d(cur.x, cur.y, cur.z);
                }
            }
        }
        glEnd();
        glPopMatrix();
        glPointSize(1);
        
        
        glPushMatrix();
        glTranslated(0, 0, 0.5*(floorlevel+rooflevel));
         ofPushStyle();
         if(this->active)
        {
            ofSetColor(0, 255, 0);
        }
        else
        {
            ofSetColor(255, 0, 0);
        }
        ofNoFill();
        ofRect(this->getX() - this->width/2,this->getY() - this->height/2,this->width,this->height);
        ofPopStyle();
        
        //	kinect.getDepthTextureReference().draw(0,0,kinect.width,kinect.height);
        //	kinect.getTextureReference().draw(0,0,kinect.width,kinect.height);
        //	kinectGrayImage.draw(0, 0, kinect.width, kinect.height);
        
        ofPushStyle();
        
        if(this->active)
        {
            
            ofSetColor(0,255,0);
            ofEllipse(this->getX(),this->getY(),20,20);
        }
        else
        {
            ofSetColor(255,0,0);
            ofRect(this->getX()-10,this->getY()-10,20,20);
        }
        ofPopStyle();
        
        glPopMatrix();
    } fbo->end();
}

ofTexture& ofxKinectSimpleTracker::getTextureReference(){
    return fbo->getTextureReference();
}

float ofxKinectSimpleTracker::getX(){
    float ret = x;
    ret -= 320;
    ret /= SCALE_FACTOR;
    ret += 320;
    
    return ret;
    
}

float ofxKinectSimpleTracker::getY(){
    float ret = y;
    ret -= 240;
    ret /= SCALE_FACTOR;
    ret += 240 ;
    
    return ret;
    
}