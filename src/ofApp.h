#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofRange.h"

//class CfdData{
//    
//public:
//    double P;
//    double T;
//    double PAbs;
//    double TKE;
//    double TED;
//    double TTot;
//    double IntegrationTime;
//    double Rotation;
//    double AngularVelocity;
//    glm::vec3 vel;
//    glm::vec3 Vorticity;
//    glm::vec3 Normals;
//    glm::vec3 Points;
//};

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

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void loadData();
    void printOscIn();
    
    vector<ofVboMesh> mesh;
    vector<ofPolyline> poly;
    vector<ofPolyline> magnitude;
    
    ofVboMesh points;
    ofVboMesh lines;
    
    ofEasyCam cam;
    
    vector<ofVboMesh> graph;
    

    ofxOscSender   sender;
    ofxOscReceiver receiver;
    
    map<string, ofRange> range;
    
    map<string, int> paramId;
    
    vector<TriggerSurface> surface;

    ofVboMesh triggerPoint;
};
