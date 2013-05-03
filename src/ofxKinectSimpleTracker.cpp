/*
 *  untitled.cpp
 *  cylinder2
 *
 *  Created by explab on 11/1/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxKinectSimpleTracker.h"

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
    
    x=100;
    y=100;
    
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
        
        if(x<width/2)
            x=width/2;
        if(x>clientWidth-width/2)
            x=clientWidth-width/2;
        if(y<height/2)
            y=height/2;
        if(y>clientHeight-height/2)
            y=clientHeight-height/2;
        
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
                
                ofPoint cur = kinect->getWorldCoordinateAt(tx,ty,0);
                //                ofPoint cur;
                //                cur.z = depth->getPixelDepth(tx, ty);
                
                float g=0;
                
                if((cur.z>rooflevel)&&(cur.z<floorlevel))
                {
                    apex+=cur.z;
                    g=255;
                    n++;
                }
                
                vx+=(float)tx*g;
                vy+=(float)ty*g;
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
        
        float xp=x-clientWidth/2.0;
        float yp=y-clientHeight/2.0;
        
        azimuth=ofRadToDeg(atan2(xp,-yp))+offset;
        radius=sqrt(xp*xp+yp*yp);
        
        if(radius>(clientHeight/2.0)-width/2.0)
            radius=(clientHeight/2.0)-width/2.0;
        
        radius=radius/((clientHeight/2.0)-width/2.0);
        
        
        renderFbo();
    }
}




void ofxKinectSimpleTracker::renderFbo(){
    fbo->begin();{
        ofSetColor(255,255,255);
        kinect->drawDepth(0, 0, clientWidth, clientHeight);
        
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
        ofRect(this->x - this->width/2,this->y - this->height/2,this->width,this->height);
        ofPopStyle();
        
        //	kinect.getDepthTextureReference().draw(0,0,kinect.width,kinect.height);
        //	kinect.getTextureReference().draw(0,0,kinect.width,kinect.height);
        //	kinectGrayImage.draw(0, 0, kinect.width, kinect.height);
        
        ofPushStyle();
        
        if(this->active)
        {
            
            ofSetColor(0,255,0);
            ofEllipse(this->x,this->y,20,20);
        }
        else
        {
            ofSetColor(255,0,0);
            ofRect(this->x-10,this->y-10,20,20);
        }
        ofPopStyle();
    } fbo->end();
}

ofTexture& ofxKinectSimpleTracker::getTextureReference(){
    return fbo->getTextureReference();
}