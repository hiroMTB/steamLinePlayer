#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofRange.h"

#include "MidiSender.h"

class TriggerSurface{
    
public:
    
    TriggerSurface(){};
    
    TriggerSurface(const glm::vec3 & pos, float width, float height){
        set(pos, width, height);
    };
                   
    void set(const glm::vec3 & _pos, float width, float height){
        pos = _pos;
        size = width;
    }
    
    bool intersect(const glm::vec3 & v, float w){
        float z = pos.z;
        return (z-w<v.z && v.z<z+w);
    }
    
    void draw(){        
        ofDrawCircle(pos, size);
    }
    
    glm::vec3 pos;
    float size;
    
};

class TriggerData{

public:
    TriggerData(glm::vec3 _pos, int _midiCh, int _note, int _vel, int _dur)
    :pos(_pos), midiCh(_midiCh), note(_note), vel(_vel), dur(_dur), dispLife(10){};

    glm::vec3 pos;

    int midiCh;
    int note;
    int vel;
    int dur;
    int dispLife;
};

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

    vector<ofVboMesh> mesh;
    vector<ofPolyline> poly;
    vector<ofPolyline> magnitude;
    vector<ofPolyline> rotation;
    
    vector<ofVboMesh> graph;
    vector<TriggerSurface> surface;
    
    map<string, ofRange> range;
    map<string, int> paramId;
    
    ofVboMesh points;
    ofVboMesh triggerPoint;
    ofVboMesh prmLine;
    
    ofEasyCam cam;
    ofxOscReceiver receiver;

    vector<TriggerData> triggerData;
    
    MidiSender sender;

    int frame = 0;
};
