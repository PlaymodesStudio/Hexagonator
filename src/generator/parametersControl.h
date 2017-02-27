//
//  parametersControl.h
//  DGTL_Generator
//
//  Created by Eduard Frigola on 03/08/16.
//
//

#ifndef parametersControl_h
#define parametersControl_h

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxDatGui.h"
#include "ofxMidi.h"
#include "ofxTweenzor.h"
#include "bpmControl.h"

static const int NUM_PRESETS = 40;

class nodeConnection{
public:
    nodeConnection(){};
    nodeConnection(ofxDatGuiComponent* c, ofAbstractParameter* p){
        points.resize(2);
        points[0].x = c->getX() + c->getWidth();
        points[0].y = c->getY() + c->getHeight()/2;
        bindedComponents[0] = c;
        bindedParameters[0] = p;
        polyline.addVertex(points[0]);
        polyline.addVertex(points[0]);
    }
    
    void moveLine(ofPoint p){
        points[1] = p;
        polyline.getVertices()[1] = p;
    }
    
    void connectTo(ofxDatGuiComponent* c, ofAbstractParameter* p){
        points[1].x = c->getX();
        points[1].y = c->getY() + c->getHeight()/2;
        bindedComponents[1] = c;
        bindedParameters[1] = p;
        polyline.getVertices()[1] = points[1];
        closedLine = true;
    }
    
    ofPolyline getPolyline(){
        if(closedLine){
            ofPoint p1;
            p1.x = bindedComponents[0]->getX() + bindedComponents[0]->getWidth();
            p1.y = bindedComponents[0]->getY() + bindedComponents[0]->getHeight()/2;
            ofPoint p2;
            p2.x = bindedComponents[1]->getX();
            p2.y = bindedComponents[1]->getY() + bindedComponents[1]->getHeight()/2;
            if(p1 != points[0] || p2 != points[1]){
                polyline.getVertices()[0] = p1;
                polyline.getVertices()[1] = p2;
            }
        }
        
        return polyline;
    }
    
    bool hitTest(ofPoint p){
//        float angleBetweenVectors = (p-points[0]).angle(points[1]-points[0]);
//        float margin = 2;
//        float dotLine = points[0].dot(points[1]);
//        float dotLinePoint = (p-points[0]).dot(points[1]-points[0]);
//        if(angleBetweenVectors < margin && 0 < dotLinePoint && dotLinePoint < dotLine) return true;
//        else return false;
    }
    
    ofAbstractParameter* getSourceParameter(){return bindedParameters[0];};
    ofAbstractParameter* getSinkParameter(){return bindedParameters[1];};
    
    bool closedLine = false;
    
private:
    vector<ofPoint> points;
    ofPolyline polyline;
    ofxDatGuiComponent* bindedComponents[2];
    ofAbstractParameter* bindedParameters[2];
    ofColor color = ofColor::white;
};

template<typename T>
class midiConnection{
public:
    midiConnection(){};
    midiConnection(ofParameter<T>* p){
        bindedParameter = p;
        midiListenMode = true;
        toggle = false;
    }
    
    void assignMidiControl(string device, int channel, int control){
        midiDevice = device;
        channelNumber = channel;
        controlNumber = control;
        midiListenMode = false;
        if(ofGetKeyPressed(OF_KEY_COMMAND))
            toggle = true;
    }
    
    int sendValue(){
        int toMidiVal = 0;
        if(bindedParameter->type() == typeid(ofParameter<int>).name()){
            int range = bindedParameter->getMax()-bindedParameter->getMin();
            //TODO: Review, map is from 0 127 not 0 1;
            if(range < 128)
                toMidiVal = ofMap(*bindedParameter, bindedParameter->getMin(), bindedParameter->getMax(), 0, ((int)(128/(range))*range));
            else
                toMidiVal = ofMap(*bindedParameter, bindedParameter->getMin(), bindedParameter->getMax(), 0, range/ceil((float)range/(float)128));
            
        }
        else if(bindedParameter->type() == typeid(ofParameter<float>).name()){
            toMidiVal = ofMap(*bindedParameter, bindedParameter->getMin(), bindedParameter->getMax(), 0, 127);
        }
        else if(bindedParameter->type() == typeid(ofParameter<bool>).name()){
            if(!toggle)
                toMidiVal = (bindedParameter == 0)? 0 : 127;
        }
        return toMidiVal;
    }
    void setValue(int midiValue){
        if(bindedParameter->type() == typeid(ofParameter<int>).name()){
            int range = bindedParameter->getMax()-bindedParameter->getMin();
            if(range < 128)
                *bindedParameter = ofMap(midiValue, 0, ((int)(128/(range))*range), bindedParameter->getMin(), bindedParameter->getMax(), true);
            else
                *bindedParameter = ofMap(midiValue, 0, range/ceil((float)range/(float)128), bindedParameter->getMin(), bindedParameter->getMax(), true);
            
        }
        else if(bindedParameter->type() == typeid(ofParameter<float>).name()){
            *bindedParameter = (ofMap(midiValue, 0, 127, bindedParameter->getMin(), bindedParameter->getMax(), true));
        }
        else if(bindedParameter->type() == typeid(ofParameter<bool>).name()){
            if(toggle && midiValue == 127)
                *bindedParameter = !*bindedParameter;
            else if(!toggle)
                *bindedParameter = (midiValue == 0)? false : true;
        }
    }
    
    ofParameter<T>* getParameter(){return bindedParameter;};
    bool isListening(){return midiListenMode;};
    string getDevice(){return midiDevice;};
    int getChannel(){return channelNumber;};
    int getControl(){return controlNumber;};
    bool isToggle(){return toggle;};
    void setToggle(bool b){toggle = b;};
    
private:
    ofParameter<T>* bindedParameter;
    bool midiListenMode;
    string midiDevice;
    int channelNumber;
    int controlNumber;
    bool toggle;
};


class parametersControl: public ofxMidiListener{
public:
    
    parametersControl(){};
    ~parametersControl(){};
    
    static parametersControl &getInstance()
    {
        static parametersControl instance;
        return instance;
    }
    
    void createGuiFromParams(ofParameterGroup paramGroup, ofColor guiColor = ofColor(ofRandom(255), ofRandom(255), ofRandom(255)));
    
    void setup();
    void update();
    void draw();
    
    int getGuiWidth(){return datGui->getWidth();};
    
    void onGuiButtonEvent(ofxDatGuiButtonEvent e);
    void onGuiToggleEvent(ofxDatGuiToggleEvent e);
    void onGuiDropdownEvent(ofxDatGuiDropdownEvent e);
    void onGuiMatrixEvent(ofxDatGuiMatrixEvent e);
    void onGuiSliderEvent(ofxDatGuiSliderEvent e);
    void onGuiTextInputEvent(ofxDatGuiTextInputEvent e);
    void onGuiColorPickerEvent(ofxDatGuiColorPickerEvent e);
    void onGuiRightClickEvent(ofxDatGuiRightClickEvent e);
    
    void keyPressed(ofKeyEventArgs &e);
    void keyReleased(ofKeyEventArgs &e);
    void mouseMoved(ofMouseEventArgs &e);
    void mouseDragged(ofMouseEventArgs &e);
    void mousePressed(ofMouseEventArgs &e);
    void mouseReleased(ofMouseEventArgs &e);
    void mouseScrolled(ofMouseEventArgs &e);
    void mouseEntered(ofMouseEventArgs &e);
    void mouseExited(ofMouseEventArgs &e);
    
    void bpmChangedListener(float &bpm);
    
    void listenerFunction(ofAbstractParameter& e);
    
    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    void savePreset(int presetNum, string bank);
    void loadPreset(int presetNum, string bank);
    void loadPresetWithFade(int presetNum, string bank);
    
    void saveMidiMapping();
    void loadMidiMapping();
    
    void saveGuiArrangement();
    void loadGuiArrangement();
    
    bool loadPresetsSequence();
    
    void loadPresetWhenFadeOutCompletes(float *arg);
    
    void setWindows(shared_ptr<ofAppBaseWindow> guiWindow, shared_ptr<ofAppBaseWindow> prevWindow){this->guiWindow = guiWindow; this->prevWindow = prevWindow;};
    
private:
    
    void setFromNormalizedValue(ofAbstractParameter* p, float v);
    void setFromSameTypeValue(ofAbstractParameter* source, ofAbstractParameter* sink);
    
    ofxDatGui *datGui;
    ofxDatGuiMatrix* presetMatrix;
    ofxDatGuiDropdown* bankSelect;
    
    vector<ofxDatGui*> datGuis;
    vector<ofParameterGroup> parameterGroups;
    
    ofParameter<bool> autoPreset;
    ofParameter<float> presetChangeBeatsPeriod;
    ofParameter<float>  fadeTime;
    
    ofxOscReceiver oscReceiver;
    ofxOscSender oscSender;
    
    ofXml xml;
    
    vector<ofxMidiOut> midiOut;
    vector<ofxMidiIn>  midiIn;
    
    deque<ofxMidiMessage> midiMessages;
    vector<ofxMidiMessage>  midiMessagesFiller;
    int newMessages = 0;
    
    float   presetChangedTimeStamp;
    float   periodTime;
    
    vector<int> randomPresetsArrange;
    vector<pair<int, string>> presetNumbersAndBanks;
    vector<int> presetsTime;
    int presetChangeCounter;
    
    int presetToLoad;
    string bankToLoad;
    
    bool isFading;
    
    shared_ptr<ofAppBaseWindow> guiWindow;
    shared_ptr<ofAppBaseWindow> prevWindow;
    
    ofParameterGroup* masterGroupParam;
    
    
    //BPM Detect
    bpmControl *beatTracker;
    float       newBpm;
    
    
    //node
    vector<nodeConnection>  connections;
    vector<midiConnection<int>>  midiIntConnections;
    vector<midiConnection<float>>  midiFloatConnections;
    vector<midiConnection<bool>>  midiBoolConnections;
    
    int lastMidiPressetPitch = 0;
    vector<int> pitchMapper = {36, 38, 40, 41, 43, 45, 47, 24, 26, 28, 29, 31, 33, 35, 12, 14, 16, 17, 19, 21, 23, 0, 2, 4, 5, 7, 9, 11};
};


#endif /* parametersControl_h */
