#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofRange.h"

#include "MidiSender.h"

class ofApp : public ofBaseApp{
    
public:
    
    static ofApp * get(){
        static ofApp * app;
        if(!app) app = new ofApp();
        return app;
    }
    
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void loadData(string fileName);
    void printOscIn();

    bool bPrmMode = false;
    float frame = 0;
    int triggerResolution = 20;
    int maxMidiCh = 16;

    vector<ofVboMesh> streamMesh;
    vector<ofVboMesh> triggerPoint;
    vector<ofPolyline> streamPoly;
    vector<ofPolyline> magnitude;
    vector<ofPolyline> rotation;
    vector<vector<float>> triggerPointLength;
    map<string, ofRange> range;
    map<string, int> paramId;
    
    ofVboMesh indicator;
    ofVboMesh prmLine;
    
    ofEasyCam cam;
    ofxOscReceiver receiver;

    MidiSender sender;

    
};
