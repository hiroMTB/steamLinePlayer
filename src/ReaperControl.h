#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

class ReaperControl{
    
public:
    
    ReaperControl(){
        sender.setup("localhost", 8000);
    };
    
    void sendMidiNoteOn(unsigned int midiCh, unsigned int noteNum, unsigned int velocity, unsigned int duration){

        ofxOscMessage on;
        char c[255];
        sprintf(c, "/vkb_midi/%i/note/%i", midiCh, noteNum);
        on.setAddress(string(c));
        on.addIntArg(velocity);
        sender.sendMessage(on);
        ofLogVerbose("Note ON", string(c));
        
        std::thread noteOffThread( &ReaperControl::sendNoteOff, this, string(c), duration);
        noteOffThread.detach();
    }
    
    void sendNoteOff(string address, unsigned int duration){
        std::this_thread::sleep_for( chrono::microseconds(duration*1000) );
        ofxOscMessage off;
        off.setAddress(address);
        sender.sendMessage(off);
        
        ofLogVerbose("Note OFF", address);
    }
    
    void sendFxParam(unsigned int track, unsigned int fx, unsigned int prm, float value){
        ofxOscMessage msg;

        char c[255];
        sprintf(c, "/track/%i/fx/%i/fxparam/%i/value", track, fx, prm);
        msg.setAddress(string(c));
        msg.addFloatArg(value);
        //ofLogVerbose("", string(c));
    }
    
    ofxOscSender sender;
};
