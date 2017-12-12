#include "ofApp.h"
#include "ofApp_loadData.impl"

void ofApp::setup(){

    reaper.sendNoteOffAll();
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetWindowPosition(0,0);
    
    ofEnableAlphaBlending();
    ofSetCircleResolution(60);
    ofSetFrameRate(30);

    // camera settings
    cam.setNearClip(1);
    cam.setFarClip(10000);
    
    // osc
    receiver.setup(9000);

    // csv data load
    mesh.push_back(ofVboMesh());
    mesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
    
    poly.push_back(ofPolyline());
    magnitude.push_back(ofPolyline());
    points.setMode(OF_PRIMITIVE_POINTS);
    prmLine.setMode(OF_PRIMITIVE_LINES);
    triggerPoint.setMode(OF_PRIMITIVE_POINTS);
    //loadData("musical_cylinder_04_strm_01.csv");
    loadData("musical_cylinder_04_strm_02.csv");
    
    // construct surface
    for(int i=0; i<11; i++){
        surface.push_back(TriggerSurface());
        surface.back().set(glm::vec3(0,0, i*100-500), 100, 100);
    }
}

void ofApp::update(){
    
    reaper.update();
    
    int duration = ofGetTargetFrameRate() * 60 * 6;
    int frame = ofGetFrameNum() % duration;
    float percent = (float)frame/duration;
    
    points.clear();
    prmLine.clear();
    
    triggerPoint.clear();
    
    int numPoly = poly.size();
    
    for( int i=0; i<numPoly; i++){
        ofPolyline & p = poly[i];
        ofPolyline & m = magnitude[i];
        
        glm::vec3 v = p.getPointAtPercent(percent);
        glm::vec3 mag = m.getPointAtPercent(percent);
        
        //glm::vec3 v = p.getPointAtLength(frame*0.5);
        //glm::vec3 mag = m.getPointAtLength(frame*0.5);
        
        ofFloatColor red(1,0,0);
        ofFloatColor blue(0,0,1);
        ofFloatColor col;
        float len = mag.y * 0.004;
        col =  red*len + blue*(1.0-len);
        col.a = 0.9;
        
        points.addVertex(v);
        points.addColor(col);
        
        {
            glm::vec3 zerovec(0,0,v.z);
            prmLine.addVertex(zerovec);
            prmLine.addVertex(v);
            col.a = 0.4;
            prmLine.addColor(ofFloatColor(col));
            prmLine.addColor(ofFloatColor(col));
        }
        
        // Reaper
        // Track Number : 1 ~ ...
        // Midi Ch      : 0 ~ 15
        // fx           :
        // fxparam      :
        int track = i/100 + 1;
        int lineNum = i%100;
        bool bAmbix = false;
        int ambixSlot = 3;
        
        // check intersect
        float intersectWidth = 0.5;
        for(int j=0; j<surface.size(); j++){
            bool on = surface[j].intersect(v, intersectWidth);
            if(on){
                triggerPoint.addVertex(v);
                triggerPoint.addColor(ofFloatColor(0));
                
                // MIDI noteOn
                int midiCh  = track-1;
                int note    = ofRandom(36, 72);
                int vel     = ofRandom(50,100);
                int dur     = ofRandom(10,50);
                reaper.sendNoteOn(midiCh, note, vel, dur);
            }
        }
        
        // send OSC
        if(1){
            if(lineNum == 0){
                if(bAmbix){
                    glm::vec3 yAxis(0,1,0);
                    glm::vec3 v1 = v;
                    v1.z = 0;
                    glm::vec3 v1n = glm::normalize(v1);
                    float len = glm::length(v1);
                    float angle = glm::angle(yAxis, v1n);
                    
                    len = ofMap(len, 0, 100.0, 0, 1.0);
                    angle = ofMap(angle, -PI, PI, 0, 1.0);
                    
                    // azimuth -180 ~ 180
                    reaper.sendFxParam(track, ambixSlot, 1, angle);
                    
                    // elevation -180 ~ 180
                    reaper.sendFxParam(track, ambixSlot, 2, len);
                }
            }
            else if(1<=lineNum && lineNum <=10){
                
                // line# 1- 10
                // track control (fader, EQ, etc)
                //double m = mag.y;
                //ofxOscMessage msg;
                //msg.setAddress("/track/"+ ofToString(track) + "/fx/2/fxparam/1/value");
                //msg.addFloatArg(m);
            }
            else if( 11<=lineNum && lineNum<=20 ){

                // line# 11 ~ 20
                // FX 1 : synth control
                int prm = lineNum - 10;
                int prmOnDaw = -1;
                switch(prm){
                    case 1:
                        prmOnDaw = 1;
                }
                
                if(prmOnDaw!=-1){
                    float m = mag.y * 0.004;
                    reaper.sendFxParam(track, 1, prmOnDaw, m);
                }

            }
            else if( 21<=lineNum && lineNum<=30 ){
                
                // line# 21 ~ 30
                // FX 2 : effcter reverb?
                int prm = lineNum - 20;
                float m = mag.y * 0.004;
                reaper.sendFxParam(track, 2, prm, m);
            }
            else if( 31<=lineNum && lineNum<=40 ){
                
                // line# 31 ~ 40
                // FX 3 : effcter reverb?
                int prm = lineNum - 30;
                float m = mag.y * 0.004;
                reaper.sendFxParam(track, 3, prm, m);
            }
        }
    }

}

void ofApp::draw(){
   
    ofEnableAntiAliasing();
    ofBackground(255);
    
    cam.begin();
 
    if(cam.getOrtho()){
        ofDisableDepthTest();
        ofScale(3, 3);
    }else{
        ofEnableDepthTest();
    }
    
    ofDrawAxis(100);
    
    if(!bPrmMode){
        for(auto & m : mesh){
            m.draw();
        }
    }else{
        prmLine.draw();
    }
    
    // indicator
    glPointSize(3);
    points.draw();
    
    // collision
    glPointSize(6);
    triggerPoint.draw();
    
    // circle surface
    for(auto & t : surface){
        ofNoFill();
        ofSetColor(100,50);
        t.draw();
    }
    
    cam.end();
    
    ofDisableAntiAliasing();
    ofSetupScreenOrtho();
    for(int i=0; i<triggerPoint.getNumVertices(); i++){
        
        glm::vec3 v = triggerPoint.getVertex(i);
        glm::vec3 s = cam.worldToScreen(v);
        glm::vec3 left = s;
        s.x = 50;
        ofSetLineWidth(1);
        ofSetColor(100, 200);
        ofDrawLine(s, left);
    }
    
}

void ofApp::keyPressed(int key){
    
    switch(key){
        case 'O':
            cam.getOrtho() ? cam.disableOrtho() : cam.enableOrtho();
            break;
        
        case 'P':
            bPrmMode = !bPrmMode;
            break;
            
        case 't':
            reaper.sendNoteOn(0, ofRandom(36,60), ofRandom(80,120), ofRandom(10,100));
            break;
            
        case 'R':
            reaper.sendNoteOffAll();
            break;
            
        default:
            break;
    }
}

void ofApp::printOscIn(){
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // unrecognized message: display on the bottom of the screen
        string msg_string;
        msg_string = m.getAddress();
        msg_string += ":";
        for(size_t i = 0; i < m.getNumArgs(); i++){
            // get the argument type
            msg_string += " ";
            msg_string += m.getArgTypeName(i);
            msg_string += ":";
            // display the argument - make sure we get the right type
            if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                msg_string += ofToString(m.getArgAsInt32(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                msg_string += ofToString(m.getArgAsFloat(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                msg_string += m.getArgAsString(i);
            }
            else{
                msg_string += "unknown";
            }
        }
        cout << msg_string << endl;
    }
}

void ofApp::exit(){
    reaper.sendNoteOffAll();
}
