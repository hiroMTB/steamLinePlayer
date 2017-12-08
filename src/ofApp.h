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

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void loadData();
    
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
};
