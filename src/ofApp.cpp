#include "ofApp.h"


void ofApp::setup(){
    
    ofEnableDepthTest();
    ofEnableAlphaBlending();
    ofSetCircleResolution(120);

    // camera settings
    // default fov = 60
    // cam.setFov(30);
    
    // osc
    sender.setup("localhost", 1234);

    
    mesh.push_back(ofVboMesh());
    mesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
    
    poly.push_back(ofPolyline());
    magnitude.push_back(ofPolyline());
    
    points.setMode(OF_PRIMITIVE_POINTS);
    
    // csv data format (from Paraview)
    //  0 P
    //  1 T
    //  2 PAbs
    //  3 TKE
    //  4 TED
    //  5 TTot
    //  6 vel:0
    //  7 vel:1
    //  8 vel:2
    //  9 IntegrationTime
    // 10 Vorticity:0
    // 11 Vorticity:1
    // 12 Vorticity:2
    // 13 Rotation
    // 14 AngularVelocity
    // 15 Normals:0
    // 16 Normals:1
    // 17 Normals:2
    // 18 Points:0
    // 19 Points:1
    // 20 Points:2
    ifstream file(ofToDataPath("musical_cylinder_04_strm_02.csv"));

    string str;
    getline(file,str);
    {
        str.erase(str.size()-1);    // erase \r
        string prmName;
        istringstream stream(str);
        int i=0;
        while(getline(stream,prmName,'\t')){
            paramId.insert({prmName, i});
            cout << i++ << " " << prmName << " \n";
        }
    }
    
    while(getline(file,str)){
        
        string token;
        istringstream stream(str);
        
        // separate depends on , or \t
        vector<double> entry(paramId.size());
        
        for(int i=0; i<entry.size(); i++){
            getline(stream,token,'\t');
            entry[i] = ofToDouble(token);
        }
        
        glm::vec3 pos(entry[paramId["Points:0"]], entry[paramId["Points:1"]], entry[paramId["Points:2"]]);
        glm::vec3 vel(entry[paramId["vel:0"]], entry[paramId["vel:1"]], entry[paramId["vel:2"]]);
        
        float len = glm::length(vel);
        len *= 0.004;
        ofFloatColor red(1,0,0);
        ofFloatColor blue(0,0,1);
        ofFloatColor col;
        col =  red*len + blue*(1.0-len);
        col.a = 0.2;
        
        double time = entry[paramId["IntegrationTime"]];
        if(time == 0){
            
            int prevNumVertices = mesh.back().getNumVertices();
            if(prevNumVertices < 10){
                mesh.pop_back();
                poly.pop_back();
                magnitude.pop_back();
                cout << "delete data because too few vertices : " << prevNumVertices << endl;
            }
            
            mesh.push_back(ofVboMesh());
            mesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
            
            poly.push_back(ofPolyline());
            magnitude.push_back(ofPolyline());
        }
        
        mesh.back().addVertex(pos);
        mesh.back().addColor(col);
        
        poly.back().addVertex(pos);
        
        int vId = mesh.back().getNumVertices();
        float m = glm::length(vel);
        magnitude.back().addVertex(glm::vec3(vId, m, 0));
    }
    
    int numPoly = poly.size();
    cout << endl << "create " << numPoly << " polylines" << endl;
}


void ofApp::update(){
    int frame = ofGetFrameNum();
    float duration = 25 * 60 * 3;
    float percent = frame/duration;
    
    points.clear();
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
        
        ofSetColor(mag.y, 0, 0);
        
        points.addVertex(v);
        points.addColor(col);
        
        
        // send OSC
        if(1){
            
            
            
            
        }
    }
    

}


void ofApp::draw(){
   
    ofBackground(255);
    
    cam.begin();
 
    //ofDrawAxis(10);
    ofSetColor(255,0,0);
    ofNoFill();
    ofPushMatrix();
//    ofRotateXDeg(90);
    //ofDrawCircle(0, 0, 100);
    ofPopMatrix();
    
    for(auto & m : mesh)
        m.draw();

    glPointSize(1);
    points.draw();
    
    cam.end();
}


void ofApp::keyPressed(int key){

}
