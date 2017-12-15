#include "ofApp.h"
#include "ofApp_loadData.impl"

void ofApp::setup(){

    sender.sendNoteOffAll();
    
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
    streamMesh.push_back(ofVboMesh());
    streamMesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
        
    streamPoly.push_back(ofPolyline());
    magnitude.push_back(ofPolyline());
    rotation.push_back(ofPolyline());
    indicator.setMode(OF_PRIMITIVE_POINTS);
    prmLine.setMode(OF_PRIMITIVE_LINES);
    
    loadData("musical_cylinder_04_strm_02.csv");
    
}

void ofApp::update(){
    
    sender.update();
    float speed = 0.3;
    int duration = ofGetTargetFrameRate() * 60 * 10;
    
#pragma mark Check TriggerPoint
    
    indicator.clear();
    prmLine.clear();
    
    int numStream = streamPoly.size();
    int dataWidth = numStream/maxMidiCh;

    vector<vector<int>> prms(maxMidiCh, vector<int>(10));

    for(int i=0; i<maxMidiCh; i++){
        
        int lineId = i * dataWidth;

        ofPolyline & p = streamPoly[lineId];
        ofPolyline & m = magnitude[lineId];
        
        glm::vec3 v = p.getPointAtLength(frame);
        glm::vec3 mag = m.getPointAtLength(frame);
        
        ofFloatColor red(1,0,0);
        ofFloatColor blue(0,0,1);
        ofFloatColor col;
        
        float len = mag.y * 0.004;
        col =  red*len + blue*(1.0-len);
        col.a = 0.9;
        
        indicator.addVertex(v);
        indicator.addColor(col);
        
        glm::vec3 zerovec(0,0,v.z);
        prmLine.addVertex(zerovec);
        prmLine.addVertex(v);
        col.a = 0.4;
        prmLine.addColor(ofFloatColor(col));
        prmLine.addColor(ofFloatColor(col));
        
        
#pragma mark Check if hits
    
        float currentLen = frame;
        float nextLen = currentLen + speed;
        vector<int> & prm = prms[i];
        
        for(int j=0; j<triggerPointLength[i].size(); j++){
            float tlen = triggerPointLength[i][j];
            prm[0] = (currentLen <= tlen && tlen <nextLen);
            if(prm[0]) break;
        }
        
#pragma mark send CC anyway
        for(int j=0; j<prm.size(); j++){
            int prmId = j+1;

            float val = 0;
            if(prmId==-1){
                // noteOn
                glm::vec2 axis(0,1);
                glm::vec2 vn(v.x, v.y);
                vn = glm::normalize(vn);
                float angle = glm::orientedAngle(vn, axis);
                cout << angle << endl;
                val = ofMap(angle, -TWO_PI, TWO_PI, 36, 36+12*4);
            }else{
                // Other Prm
                ofPolyline & m = magnitude[lineId];
                glm::vec3 mag = m.getPointAtLength(frame);
                val = ofMap(mag.y, range["mag"].min, range["mag"].max, 0, 127);
            }
            prm[prmId] = val;
        }
    }

#pragma mark Send Midi at once
    for(int i=0; i<prms.size(); i++){

        vector<int> & prm = prms[i];
        int ch = i+1;
        
        int pan     = prm[4];
        int cc13    = prm[5];
        int cc14    = prm[6];
        int cc15    = prm[7];
        int cc16    = prm[8];
        int cc17    = prm[9];
        sender.sendCC(ch, 9, pan);
        sender.sendCC(ch, 13, cc13);
        sender.sendCC(ch, 14, cc14);
        sender.sendCC(ch, 15, cc15);
        sender.sendCC(ch, 16, cc16);
        sender.sendCC(ch, 17, cc17);
        
        if(prm[0]==1){
            int note = prm[1]*0.5 + 36;
            int vel  = prm[2] + 30;
            int dur  = prm[3] + 30;
            sender.sendNoteOn(ch, note, vel, dur);
            
            printf("send midi %0.1f, %ich, %i note,  %i vel, %i dur \n", frame, ch, note, vel, dur);
            // store data
            //triggerData.push_back(TriggerData(midiCh, note, vel, dur));
        }
    }    

    frame += speed;
    frame = fmod(frame, duration);

}

void ofApp::draw(){
   
    ofEnableAntiAliasing();
    ofBackground(255);
    
    cam.begin(); {
 
        if(cam.getOrtho()){
            ofDisableDepthTest();
            ofScale(3, 3);
        }else{
            ofEnableDepthTest();
        }
        
        ofDrawAxis(100);
        
        if(!bPrmMode){
            for(auto & m : streamMesh){
                m.draw();
            }
        }else{
            prmLine.draw();
        }
        
        // indicator
        glPointSize(8);
        indicator.draw();
        
        // trigger point
        glPointSize(4);
        for(auto & t : triggerPoint){
            t.draw();
        }

    }cam.end();

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
            sender.sendNoteOn(0, ofRandom(36,60), ofRandom(80,120), ofRandom(10,100));
            break;
            
        case 'R':
            sender.sendNoteOffAll();
            break;
            
        case '0':
            frame = 0;
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
    sender.sendNoteOffAll();
}
