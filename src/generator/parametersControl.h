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

static const int NUM_PRESETS = 40;

class parametersControl{
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
    void setSliderPrecision(int guiId, string sliderName, int p);
    void distributeGuis();
    
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
    
    static void addDropdownToParameterGroupFromParameters(ofParameterGroup &parameterGroup, string name, vector<string> options, ofParameter<int> &dropdownSelector){
        ofParameterGroup tempDropdown;
        tempDropdown.setName(name + " ::");
        string  tempStr;
        ofParameter<string> tempStrParam("Options");
        for(auto opt : options)
            tempStr += opt + "-|-";
        
        tempStr.erase(tempStr.end()-3, tempStr.end());
        tempStrParam.set(tempStr);
        
        tempDropdown.add(tempStrParam);
        tempDropdown.add(dropdownSelector.set(name +" Select", dropdownSelector, 0, options.size()));
        parameterGroup.add(tempDropdown);
        
    }
    
private:
    
    ofxDatGui *datGui;
    ofxDatGuiMatrix* presetMatrix;
    ofxDatGuiDropdown* bankSelect;
    
    ofParameter<int>    framesToRecord;
    
    vector<ofxDatGui*> datGuis;
    vector<ofParameterGroup> parameterGroups;

    
    
    ofxOscReceiver oscReceiver;
    ofxOscSender oscSender;
    
    ofXml xml;
    
    float   presetChangedTimeStamp;
    float   periodTime;
    
    vector<int> randomPresetsArrange;
    vector<pair<int, string>> presetNumbersAndBanks;
    vector<int> presetsTime;
    int presetChangeCounter;
    
    int presetToLoad;
    string bankToLoad;
    
    shared_ptr<ofAppBaseWindow> guiWindow;
    shared_ptr<ofAppBaseWindow> prevWindow;
    
    ofParameterGroup* masterGroupParam;
};


#endif /* parametersControl_h */
