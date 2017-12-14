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
    int nSurface = 21;
    int surfaceWidth = 1000.0f/(nSurface-1);
    for(int i=0; i<nSurface; i++){
        surface.push_back(TriggerSurface());
        surface.back().set(glm::vec3(0,0, i*surfaceWidth-500), 100, 100);
    }
}

void ofApp::update(){
    
    sender.update();
    
    int duration = ofGetTargetFrameRate() * 60 * 1;
    frame = ++frame % duration;
    float percent = (float)frame/duration;
    
    points.clear();
    prmLine.clear();
    
    triggerPoint.clear();
    
    vector<TriggerData>::iterator it = triggerData.begin();
    for(; it!=triggerData.end(); it++){
        it->dispLife -=1;
        if(it->dispLife<=0){
            triggerData.erase(it);
            it--;
        }
    }
    
    int numPoly = poly.size();
    
    
    for( int i=0; i<numPoly; i++){
        ofPolyline & p = poly[i];
        ofPolyline & m = magnitude[i];
        ofPolyline & r = rotation[i];
        
        glm::vec3 v = p.getPointAtPercent(percent);
        glm::vec3 mag = m.getPointAtPercent(percent);
        glm::vec3 rot = r.getPointAtPercent(percent);
        
        ofFloatColor red(1,0,0);
        ofFloatColor blue(0,0,1);
        ofFloatColor col;

        float len = rot.y * 0.004;
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
    }
    
    // sender
    // Midi Ch      : 1 ~ 16
    int maxMidiCh = 16;
    int dataWidth = numPoly/maxMidiCh;

    vector<vector<int>> prms(maxMidiCh, vector<int>(10));

    // check intersect & send noteOn
    for(int i=0; i<maxMidiCh; i++){
        
        vector<int> & prm = prms[i];
        
        int lineId = i * dataWidth;

        // Note On
        ofPolyline & p = poly[lineId];
        ofPolyline & m = magnitude[lineId];
        glm::vec3 v = p.getPointAtPercent(percent);
        glm::vec3 mag = m.getPointAtPercent(percent);
        
        triggerPoint.addVertex(v);
        triggerPoint.addColor(ofFloatColor(0));
        prm[0] = 1;
        
        for(int j=0; j<prm.size(); j++){
            int prmId = j+1;
            //ofPolyline & m = magnitude[lineId];
            //glm::vec3 mag = m.getPointAtPercent(percent);
            //int val = ofMap(mag.y, range["mag"].min, range["mag"].max, 0, 127);

            //ofPolyline & r = rotation[lineId];
            //glm::vec3 rot = r.getPointAtPercent(percent);
            //int val = ofMap(rot.y, range["Rotation"].min, range["Rotation"].max, 0, 127);
            
            glm::vec3 axis(0,1,0);
            glm::vec3 vn = glm::normalize(v);
            float angle = glm::angle(vn, axis);
             float val = ofMap(angle, -PI, PI, 0, 127);
            prm[prmId] = val;
        }
    }
    
    //
    //  send MIDI
    //
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
            int note = prm[1];
            int vel  = prm[2] + 10;
            int dur  = prm[3]*0.5 + 5;
            sender.sendNoteOn(ch, note, vel, dur);
            
            printf("send midi %i ch, %i note,  %i vel, %i dur \n", ch, note, vel, dur);
            // store data
            //triggerData.push_back(TriggerData(midiCh, note, vel, dur));
        }
    }
    
    cout << endl;

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
        glPointSize(10);
        triggerPoint.draw();
        for(int i=0; i<triggerPoint.getNumVertices(); i++){
            glm::vec3 v = triggerPoint.getVertex(i);
            ofFill();
            ofSetColor(255,0,0);
            ofDrawSphere(v, 5);
        }
        
        // circle surface
        for(auto & t : surface){
            ofNoFill();
            ofSetColor(100,50);
            t.draw();
        }
    }cam.end();
    
//    //ofDisableAntiAliasing();
//    ofSetupScreenOrtho();
//    vector<int> nTrigger;
//    nTrigger.assign(16,0);
//    for(int i=0; i<triggerData.size(); i++){
//        TriggerData & d = triggerData[i];
//        glm::vec3 & v = d.pos;
//        glm::vec3 s1 = cam.worldToScreen(v);
//        bool left = d.midiCh<5;
//        int x = left ? 220 : ofGetWidth()-220;
//        int y = 50 + (d.midiCh%5) * 200;
//        glm::vec3 s2(x, y+nTrigger[d.midiCh]*10, 0);
//        
//        //ofSetLineWidth(1);
//        //ofSetColor(0);
//        //ofDrawLine(s1, s2);
//        char c[255];
//        sprintf(c, "noteOn ch%i n%i v%i d%i", d.midiCh, d.note, d.vel, d.dur);
//        ofSetColor(100);
//        int sx = s2.x + (left ? -200:30);
//        ofDrawBitmapString(string(c), sx, s2.y);
//        nTrigger[d.midiCh]++;
//    }
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
