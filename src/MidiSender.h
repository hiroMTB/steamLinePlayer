#pragma once
//#include "JuceLibraryCode/JuceHeader.h"
//#include "juce_audio_devices/juce_audio_devices.h"

#include "ofMain.h"
#include "ofxMidi.h"

class MidiSender{
    
    
public:
    
    MidiSender(){
        
        mout.openVirtualPort("StreamlinePlayer");
        
        noteOnState.assign(16, vector<int>());
        for(vector<int> & v : noteOnState){
            v.assign(128, 0);
        }
    }
    
    ~MidiSender(){
        mout.closePort();
    }
    
    void update(){
        for(vector<int> & v : noteOnState){
            for(int i=0; i<v.size(); i++){
                int wait = v[i];
                v[i] = MAX(0, wait-1);
            }
        }
    }
    
    void sendNoteOn(int midiCh, int noteNum, int velocity, int duration){
        if(noteOnState[midiCh-1][noteNum] != 0 ) return;
        noteOnState[midiCh-1][noteNum] = resetValue;

        mout.sendNoteOn(midiCh, noteNum, velocity);
        
        std::thread noteOffThread( &MidiSender::sendNoteOff, this, midiCh, noteNum, duration);
        noteOffThread.detach();
    }
    
    void sendNoteOff(int midiCh, int noteNum, int duration){
        if(duration>0)
            std::this_thread::sleep_for( chrono::microseconds(duration*1000) );

        vector<unsigned char> noteOff;
        noteOff.push_back(MIDI_NOTE_OFF+(midiCh-1));
        noteOff.push_back(noteNum);
        noteOff.push_back(0);
        ofPtr<ofxBaseMidiOut> ofOut = mout.midiOut;
        ofxRtMidiOut * rtOut = static_cast<ofxRtMidiOut*>(ofOut.get());
        rtOut->midiOut.sendMessage(&noteOff);
    }
    
    void sendCC(int midiCh, int cc, int value){
        mout.sendControlChange(midiCh, cc, value);
    }
    
    void sendNoteOffAll(){
        for(int i=0; i<16; i++){
            for(int j=0; j<127; j++){
                sendNoteOff(i, j, 0);
            }
        }
    }
  
    ofxMidiOut mout;
    vector<vector<int>> noteOnState;
    int resetValue;

};
