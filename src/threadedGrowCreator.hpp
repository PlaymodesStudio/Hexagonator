//
//  threadedGrowCreator.hpp
//  Hexagonator
//
//  Created by Eduard Frigola on 30/03/2017.
//
//

#ifndef threadedGrowCreator_hpp
#define threadedGrowCreator_hpp

#include "ofMain.h"

class ofApp;

class threadedGrowCreator: public ofThread{
public:
    threadedGrowCreator(ofApp* appPtr);
    ~threadedGrowCreator(){stopThread();};
    
    void newFrame(){isFrameNew = true;};
    
    void test(){test();}
    
private:
    bool occupyOneHexagon(ofVec2f startingHexagon, int startingSide);
    pair<ofVec2f, int> occupyOneHexagonNonRecursive(pair<ofVec2f, int> info);
    
    void threadedFunction();
    bool isFrameNew = false;
    ofApp* ofAppPtr;
    
    vector<bool>        usedHexagons;
    vector<vector<int>> vIndexData;
};

#endif /* threadedGrowCreator_hpp */
