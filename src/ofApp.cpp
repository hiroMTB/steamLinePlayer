#include "ofApp.h"

void ofApp::setup(){
    
    ofEnableDepthTest();
    ofEnableAlphaBlending();
    ofSetCircleResolution(120);

    // osc
    sender.setup("localhost", 8000);
    receiver.setup(9000);
    
    loadData();
    
    // construct surface
    for(int i=0; i<11; i++){
        surface.push_back(TriggerSurface());
        surface.back().set(glm::vec3(0,0, i*100-500), 100, 100);
    }
}

void ofApp::loadData(){
    mesh.push_back(ofVboMesh());
    mesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
    
    poly.push_back(ofPolyline());
    magnitude.push_back(ofPolyline());
    
    points.setMode(OF_PRIMITIVE_POINTS);
    lines.setMode(OF_PRIMITIVE_LINES);
    
    triggerPoint.setMode(OF_PRIMITIVE_POINTS);
    
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
    
    vector<double> pos0, pos1, pos2;
    vector<double> vel0, vel1, vel2;
    vector<double> mag;
    vector<double> time;
    
    while(getline(file,str)){
        
        string token;
        istringstream stream(str);
        
        vector<double> entry(paramId.size());
        
        for(int i=0; i<entry.size(); i++){
            getline(stream,token,'\t');
            entry[i] = ofToDouble(token);
        }

        double p0 = entry[paramId["Points:0"]];
        double p1 = entry[paramId["Points:1"]];
        double p2 = entry[paramId["Points:2"]];
        double v0 = entry[paramId["vel:0"]];
        double v1 = entry[paramId["vel:1"]];
        double v2 = entry[paramId["vel:2"]];
        double t = entry[paramId["IntegrationTime"]];

        glm::vec3 vel(v0, v1, v2);
        
        float m = glm::length(vel);
        
        pos0.push_back(p0);
        pos1.push_back(p1);
        pos2.push_back(p2);
        vel0.push_back(v0);
        vel1.push_back(v1);
        vel2.push_back(v2);
        mag.push_back(m);
        time.push_back(t);
    }
    
    double pos0Min = *min_element(pos0.begin(), pos0.end());
    double pos1Min = *min_element(pos1.begin(), pos1.end());
    double pos2Min = *min_element(pos2.begin(), pos2.end());
    double vel0Min = *min_element(vel0.begin(), vel0.end());
    double vel1Min = *min_element(vel1.begin(), vel1.end());
    double vel2Min = *min_element(vel2.begin(), vel2.end());
    double magMin  = *min_element(mag.begin(), mag.end());
    double timeMin = *min_element(time.begin(), time.end());

    double pos0Max = *max_element(pos0.begin(), pos0.end());
    double pos1Max = *max_element(pos1.begin(), pos1.end());
    double pos2Max = *max_element(pos2.begin(), pos2.end());
    double vel0Max = *max_element(vel0.begin(), vel0.end());
    double vel1Max = *max_element(vel1.begin(), vel1.end());
    double vel2Max = *max_element(vel2.begin(), vel2.end());
    double magMax  = *max_element(mag.begin(), mag.end());
    double timeMax = *max_element(time.begin(), time.end());
    
    range.insert({"Points:0", ofRange(pos0Min, pos0Max)});
    range.insert({"Points:1", ofRange(pos1Min, pos1Max)});
    range.insert({"Points:2", ofRange(pos2Min, pos2Max)});
    range.insert({"vel:0", ofRange(vel0Min, vel0Max)});
    range.insert({"vel:1", ofRange(vel1Min, vel1Max)});
    range.insert({"vel:2", ofRange(vel2Min, vel2Max)});
    range.insert({"mag", ofRange(magMin, magMax)});
    range.insert({"time", ofRange(timeMin, timeMax)});
    
    range.insert({"Points:123", range["Points:0"] + range["Points:1"] + range["Points:2"]});
    range.insert({"vel:123", range["vel:0"] + range["vel:1"] + range["vel:2"]});
    
    for(int i=0; i<pos0.size(); i++){
    
        if(time[i] == 0){
            
            mesh.push_back(ofVboMesh());
            mesh.back().setMode(OF_PRIMITIVE_LINE_STRIP);
            
            poly.push_back(ofPolyline());
            magnitude.push_back(ofPolyline());
        }
        
        glm::vec3 pos(pos0[i], pos1[i], pos2[i]);
        glm::vec3 vel(vel0[i], vel1[i], vel2[i]);

        pos.x -= range["Points:0"].center();
        pos.y -= range["Points:1"].center();
        pos.z -= range["Points:2"].center();
        
        float biggestSpan = MAX(range["Points:0"].span(), range["Points:1"].span() );
        biggestSpan = MAX(biggestSpan, range["Points:2"].span());
        pos /= biggestSpan;
        pos *= 1000.0f;
        
        float m = mag[i];
        float cRate = m * 0.004;
        ofFloatColor red(1,0,0);
        ofFloatColor blue(0,0,1);
        ofFloatColor col;
        col =  red*cRate + blue*(1.0-cRate);
        col.a = 0.2;
        
        mesh.back().addVertex(pos);
        mesh.back().addColor(col);
        
        poly.back().addVertex(pos);
        
        int vId = mesh.back().getNumVertices();
        magnitude.back().addVertex(glm::vec3(vId, m, 0));
    }
    
    int numPoly = poly.size();
    cout << endl << "create " << numPoly << " polylines" << endl;
    
    
    // delete short one
    // deleted short lines
    vector<ofVboMesh>::iterator  meshit = mesh.begin();
    vector<ofPolyline>::iterator polyit = poly.begin();
    vector<ofPolyline>::iterator magit  = magnitude.begin();
    int numDeleted = 0;
    for(int i=0; meshit!=mesh.end(); i++, meshit++, polyit++, magit++){
    
        int num = meshit->getNumVertices();
        float length = polyit->getLengthAtIndex(polyit->getPerimeter());
        //glm::vec3 dir = meshit->getVertices()[meshit->getNumVertices()] - meshit->getVertices()[0];
        //float length = glm::length(dir);
        if(length < 10 || num<10){
            mesh.erase(meshit--);
            poly.erase(polyit--);
            magnitude.erase(magit--);
            numDeleted++;
        }
    }
    cout << "deleted " << numDeleted << " data because too few vertices : " << endl;

}

void ofApp::update(){
    int duration = 25 * 60 * 3;
    int frame = ofGetFrameNum() % duration;
    float percent = (float)frame/duration;
    
    points.clear();
    lines.clear();
    
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
        
        ofSetColor(mag.y, 0, 0);
        
        points.addVertex(v);
        col.a = 0.7;
        points.addColor(col);
        
        glm::vec3 zerovec(0,0,0);
        glm::vec3 v1 = v;
        v1.z  = 0;
        lines.addVertex(zerovec);
        lines.addVertex(v1);
        lines.addColor(ofFloatColor(col));
        lines.addColor(ofFloatColor(col));
        
        // check intersect
        float intersectWidth = 0.5;
        for(int j=0; j<surface.size(); j++){
            bool on = surface[j].intersect(v, intersectWidth);
            if(on){
                triggerPoint.addVertex(v);
                triggerPoint.addColor(ofFloatColor(0));
            }
        }
        
        // send OSC
        if(1){
            glm::vec3 yAxis(0,1,0);
            glm::vec3 v1n = glm::normalize(v1);
            float len = glm::length(v1);
            float angle = glm::angle(yAxis, v1n);
            
            len = ofMap(len, 0, 100.0, 0, 1.0);
            angle = ofMap(angle, -PI, PI, 0, 1.0);
            
            ofxOscBundle bundle;
            
            int track = i/100;
            int lineNum = i%100;
            
            bool bAmbix = false;
            
            if(lineNum == 0){
                if(bAmbix){
                    // azimuth -180 ~ 180
                    ofxOscMessage azi;
                    azi.setAddress("/track/5/fx/1/fxparam/1/value");
                    azi.addFloatArg(angle);
                    sender.sendMessage(azi);
                    cout << "azi : " << angle << endl;
                    
                    // elevation -180 ~ 180
                    ofxOscMessage elev;
                    elev.setAddress("/track/5/fx/1/fxparam/2/value");
                    elev.addFloatArg(len);
                    sender.sendMessage(elev);
                    cout << "elev : " << len << endl;
                }
            }
            else if(1<=lineNum && lineNum <=10){
                
                // line# 1- 10
                // track control (fader, EQ, etc)
                //double m = mag.y;
                //ofxOscMessage msg;
                //msg.setAddress("/track/"+ ofToString(track) + "/fx/2/fxparam/1/value");
                //msg.addFloatArg(m);
                //sender.sendMessage(msg);
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
                    ofxOscMessage msg;
                    msg.setAddress("/track/" + ofToString(track) + "/fx/1/fxparam/" +ofToString(prmOnDaw)+ "/value");
                    msg.addFloatArg(m);
                    sender.sendMessage(msg);
                }

            }
            else if( 21<=lineNum && lineNum<=30 ){
                
                // line# 21 ~ 30
                // FX 2 : effcter reverb?
                int prm = lineNum - 20;
                float m = mag.y * 0.004;
                ofxOscMessage msg;
                msg.setAddress("/track/" + ofToString(track) + "/fx/2/fxparam/" +ofToString(prm)+ "/value");
                msg.addFloatArg(m);
                sender.sendMessage(msg);
            }
            else if( 31<=lineNum && lineNum<=40 ){
                
                // line# 31 ~ 40
                // FX 3 : effcter reverb?
                int prm = lineNum - 30;
                float m = mag.y * 0.004;
                ofxOscMessage msg;
                msg.setAddress("/track/" +ofToString(track)+ "/fx/3/fxparam/"+ofToString(prm)+"/value");
                msg.addFloatArg(m);
                sender.sendMessage(msg);
            }
        }
    }

}

void ofApp::draw(){
   
    ofBackground(255);
    
    cam.begin();
 
    ofDrawAxis(100);
    ofSetColor(255,0,0);
    ofNoFill();
    ofPushMatrix();
    // ofRotateXDeg(90);
    // ofDrawCircle(0, 0, 100);
    ofPopMatrix();
    
    for(auto & m : mesh){
        m.draw();
    }
    
    glPointSize(2);
    points.draw();

    glPointSize(4);
    triggerPoint.draw();
    
    // circle surface
    for(auto & t : surface){
        ofNoFill();
        ofSetColor(100,50);
        t.draw();
    }
    
    cam.end();
    
#pragma mark Ortho
    ofSetupScreenOrtho();
    
    ofPushMatrix();
    ofTranslate(250, 250, 0);
    
    ofSetColor(100);
    ofNoFill();
    ofDrawCircle(0, 0, 50);
    ofDrawCircle(0, 0, 100);
    lines.draw();
    ofPopMatrix();
}

void ofApp::keyPressed(int key){

    ofxOscMessage msg;
    msg.setAddress("/track/5/fx/1/fxparam/1/value");
    msg.addFloatArg(ofRandom(0, 1));
    sender.sendMessage(msg);
    
//    {
//        ofxOscMessage req;
//        req.setAddress("/device/track/select/5");
//        req.addIntArg(1);
//        sender.sendMessage(req);
//    }
//    
//    {
//        ofxOscMessage req;
//        req.setAddress("/device/fx/select/1");
//        req.addIntArg(1);
//        sender.sendMessage(req);
//    }
// 
//    {
//        ofxOscMessage req;
//        req.setAddress("/device/fxparam/bank/select");
//        req.addIntArg(1);
//        sender.sendMessage(req);
//    }

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
