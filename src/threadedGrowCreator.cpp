//
//  threadedGrowCreator.cpp
//  Hexagonator
//
//  Created by Eduard Frigola on 30/03/2017.
//
//

#include "threadedGrowCreator.hpp"
#include "ofApp.h"


threadedGrowCreator::threadedGrowCreator(ofApp* appPtr)
{
    vIndexData = appPtr->hexagonCanvas.getHexagonsIndexData();
    usedHexagons.resize(appPtr->hexagonCanvas.getNumHexagons(),false);
    ofAppPtr = appPtr;
    startThread();
    thread.setStackSize(1000000000000000) ;
    cout<<thread.getStackSize()<<endl;
}

void threadedGrowCreator::threadedFunction(){
//    test();
    occupyOneHexagon(ofVec2f(0,ofRandom(64)), 0);
    cout<<"torna a cridar"<<endl;
//    pair<ofVec2f, int> pairTest = occupyOneHexagonNonRecursive(pair<ofVec2f, int>(ofVec2f(0,0), 0));
//    bool testCond = pairTest.second == -1 ? false : true;
//    while(testCond){
//        
//    }
}

bool threadedGrowCreator::occupyOneHexagon(ofVec2f startingHexagon, int startingSide){
    while (!isFrameNew);
    isFrameNew = false;
    
    // we set it the origin as "used" on the usedHexagons
    int whichNumberOfHexagon =  vIndexData[startingHexagon.x][startingHexagon.y];
    usedHexagons[whichNumberOfHexagon] = true;
    //cout << "Marked hexagon " << whichNumberOfHexagon << " :: " << startingHexagon.x << " , " << startingHexagon.y << " as used !" <<endl;
    
    // NEIGHBOURS
    vector<ofVec2f> hexagonNeighbours;
    hexagonNeighbours.resize(6,ofVec2f(-1,-1));
    
    // [0] or .x is RING 0 .. 35
    // [1] or .y is INDEX 0 .. 64
    
    if(int(startingHexagon.x)%2==0)
    {
        hexagonNeighbours[0].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[0].x = startingHexagon.x - 1;
        
        hexagonNeighbours[1].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[1].x = startingHexagon.x + 0;
        
        hexagonNeighbours[2].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[2].x = startingHexagon.x + 1;
        
        hexagonNeighbours[3].y = startingHexagon.y + 0;
        hexagonNeighbours[3].x = startingHexagon.x + 1;
        
        hexagonNeighbours[4].y = startingHexagon.y -1;
        if(hexagonNeighbours[4].y<0) hexagonNeighbours[4].y = 64 + hexagonNeighbours[4].y;
        hexagonNeighbours[4].x = startingHexagon.x +0;
        
        hexagonNeighbours[5].y = startingHexagon.y + 0;
        hexagonNeighbours[5].x = startingHexagon.x - 1;
        
    }
    else
    {
        hexagonNeighbours[0].y = (int(startingHexagon.y) +0 )%64;
        hexagonNeighbours[0].x = startingHexagon.x - 1;
        
        hexagonNeighbours[1].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[1].x = startingHexagon.x + 0;
        
        hexagonNeighbours[2].y = (int(startingHexagon.y) + 0)%64;
        hexagonNeighbours[2].x = startingHexagon.x + 1;
        
        hexagonNeighbours[3].y = startingHexagon.y -1;
        if(hexagonNeighbours[3].y<0) hexagonNeighbours[3].y = 64 + hexagonNeighbours[3].y;
        hexagonNeighbours[3].x = startingHexagon.x + 1;
        
        hexagonNeighbours[4].y = startingHexagon.y -1;
        if(hexagonNeighbours[4].y<0) hexagonNeighbours[4].y = 64 + hexagonNeighbours[4].y;
        hexagonNeighbours[4].x = startingHexagon.x +0;
        
        hexagonNeighbours[5].y = startingHexagon.y -1;
        if(hexagonNeighbours[5].y<0) hexagonNeighbours[5].y = 64 + hexagonNeighbours[5].y;
        hexagonNeighbours[5].x = startingHexagon.x - 1;
        
    }
    
    vector<ofVec2f> possibleNextHexagons;
    vector<int> possibleNumNexHexagons;
    
    for(int i=0;i<6;i++)
    {
        if( (hexagonNeighbours[i].x>=0) && (hexagonNeighbours[i].y>=0) && (hexagonNeighbours[i].x<35) )
        {
            int whichHexagonDoYouWantToGo = vIndexData[hexagonNeighbours[i].x][hexagonNeighbours[i].y];
            if(whichHexagonDoYouWantToGo!=-1)
            {
                if(usedHexagons[whichHexagonDoYouWantToGo]==false)
                {
                    //cout << "Seems like you could go to neighbour : " << i << " :: Ring " << hexagonNeighbours[i].x << " Id :: " << hexagonNeighbours[i].y << endl;
                    possibleNextHexagons.push_back(hexagonNeighbours[i]);
                    possibleNumNexHexagons.push_back(i);
                }
            }
        }
    }
    
    if(possibleNextHexagons.size()>0)
    {
        int optionChoosed;
        // choose an option from the possible neighbours.
        //        if(possibleNumNexHexagons.size()>2) optionChoosed = 2 ; //ofRandom(0,possibleNextHexagons.size());
        //        else optionChoosed=0;
        
        bool foundCulDeSac = false;
        bool isFinishingGrow = false;
        
        while(!foundCulDeSac){
            
            optionChoosed = ofRandom(0,possibleNextHexagons.size());
            //cout << "Random choosed to go to : " << optionChoosed << endl;
            
            ofVec2f nextHexagon = ofVec2f(possibleNextHexagons[optionChoosed].x,possibleNextHexagons[optionChoosed].y);
            int whichSideItStarts = (possibleNumNexHexagons[optionChoosed]+3)%6;
            
            // add the next hexagon to the vector of hexagons in order to reconstruct the worm...
            growTileInfo g;
            g._num = ofAppPtr->hexagonCanvas.getHexagonNumberFromIdAndRing(startingHexagon);
            g._id = startingHexagon.x;
            g._ring = startingHexagon.y;
            g._startingAtSide = startingSide;
            g._endingAtSide = possibleNumNexHexagons[optionChoosed];
            
            ofAppPtr->growingHexagons.push_back(g);
            
            hexagonNeighbours.clear();

            
            cout<< "INFO: " << ofAppPtr->growingHexagons.size() << " " << possibleNextHexagons.size()<<endl;
            isFinishingGrow = occupyOneHexagon(nextHexagon, whichSideItStarts);
            
//            while (!isFrameNew);
//            isFrameNew = false;
            
            if(!isFinishingGrow){
                ofAppPtr->growingHexagons.pop_back();
                possibleNextHexagons.erase(possibleNextHexagons.begin()+optionChoosed);
                possibleNumNexHexagons.erase(possibleNumNexHexagons.begin()+optionChoosed);
            }
            
            if(isFinishingGrow || !(possibleNextHexagons.size()>0)){
                foundCulDeSac = true;
            }
            
        }
        if(!isFinishingGrow) usedHexagons[whichNumberOfHexagon] = false;
        return isFinishingGrow;
    }
    else
    {
        if(ofAppPtr->growingHexagons.size()>120) return true;
        else{
            usedHexagons[whichNumberOfHexagon] = false;
            return false;
        }
    }

}

pair<ofVec2f, int>  threadedGrowCreator::occupyOneHexagonNonRecursive(pair<ofVec2f, int> info){
    ofVec2f startingHexagon = info.first;
    int startingSide = info.second;
    while (!isFrameNew);
    isFrameNew = false;
    
    lock();
    vector<vector<int>> vIndexData = ofAppPtr->hexagonCanvas.getHexagonsIndexData();
    unlock();
    
    // we set it the origin as "used" on the usedHexagons
    int whichNumberOfHexagon = vIndexData[startingHexagon.x][startingHexagon.y];
    ofAppPtr->usedHexagons[vIndexData[startingHexagon.x][startingHexagon.y]] = true;
    //cout << "Marked hexagon " << whichNumberOfHexagon << " :: " << startingHexagon.x << " , " << startingHexagon.y << " as used !" <<endl;
    
    // NEIGHBOURS
    vector<ofVec2f> hexagonNeighbours;
    hexagonNeighbours.resize(6,ofVec2f(-1,-1));
    
    // [0] or .x is RING 0 .. 35
    // [1] or .y is INDEX 0 .. 64
    
    if(int(startingHexagon.x)%2==0)
    {
        hexagonNeighbours[0].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[0].x = startingHexagon.x - 1;
        
        hexagonNeighbours[1].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[1].x = startingHexagon.x + 0;
        
        hexagonNeighbours[2].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[2].x = startingHexagon.x + 1;
        
        hexagonNeighbours[3].y = startingHexagon.y + 0;
        hexagonNeighbours[3].x = startingHexagon.x + 1;
        
        hexagonNeighbours[4].y = startingHexagon.y -1;
        if(hexagonNeighbours[4].y<0) hexagonNeighbours[4].y = 64 + hexagonNeighbours[4].y;
        hexagonNeighbours[4].x = startingHexagon.x +0;
        
        hexagonNeighbours[5].y = startingHexagon.y + 0;
        hexagonNeighbours[5].x = startingHexagon.x - 1;
        
    }
    else
    {
        hexagonNeighbours[0].y = (int(startingHexagon.y) +0 )%64;
        hexagonNeighbours[0].x = startingHexagon.x - 1;
        
        hexagonNeighbours[1].y = (int(startingHexagon.y) + 1)%64;
        hexagonNeighbours[1].x = startingHexagon.x + 0;
        
        hexagonNeighbours[2].y = (int(startingHexagon.y) + 0)%64;
        hexagonNeighbours[2].x = startingHexagon.x + 1;
        
        hexagonNeighbours[3].y = startingHexagon.y -1;
        if(hexagonNeighbours[3].y<0) hexagonNeighbours[3].y = 64 + hexagonNeighbours[3].y;
        hexagonNeighbours[3].x = startingHexagon.x + 1;
        
        hexagonNeighbours[4].y = startingHexagon.y -1;
        if(hexagonNeighbours[4].y<0) hexagonNeighbours[4].y = 64 + hexagonNeighbours[4].y;
        hexagonNeighbours[4].x = startingHexagon.x +0;
        
        hexagonNeighbours[5].y = startingHexagon.y -1;
        if(hexagonNeighbours[5].y<0) hexagonNeighbours[5].y = 64 + hexagonNeighbours[5].y;
        hexagonNeighbours[5].x = startingHexagon.x - 1;
        
    }
    
    vector<ofVec2f> possibleNextHexagons;
    vector<int> possibleNumNexHexagons;
    
    for(int i=0;i<6;i++)
    {
        if( (hexagonNeighbours[i].x>=0) && (hexagonNeighbours[i].y>=0) && (hexagonNeighbours[i].x<35) )
        {
            int whichHexagonDoYouWantToGo = vIndexData[hexagonNeighbours[i].x][hexagonNeighbours[i].y];
            if(whichHexagonDoYouWantToGo!=-1)
            {
                if(ofAppPtr->usedHexagons[whichHexagonDoYouWantToGo]==false)
                {
                    //cout << "Seems like you could go to neighbour : " << i << " :: Ring " << hexagonNeighbours[i].x << " Id :: " << hexagonNeighbours[i].y << endl;
                    possibleNextHexagons.push_back(hexagonNeighbours[i]);
                    possibleNumNexHexagons.push_back(i);
                }
            }
        }
    }
    
    if(possibleNextHexagons.size()>0)
    {
        int optionChoosed;
        // choose an option from the possible neighbours.
        //        if(possibleNumNexHexagons.size()>2) optionChoosed = 2 ; //ofRandom(0,possibleNextHexagons.size());
        //        else optionChoosed=0;
        
        bool foundCulDeSac = false;
        bool isFinishingGrow = false;
        
        while(!foundCulDeSac){
            
            optionChoosed = ofRandom(0,possibleNextHexagons.size());
            //cout << "Random choosed to go to : " << optionChoosed << endl;
            
            ofVec2f nextHexagon = ofVec2f(possibleNextHexagons[optionChoosed].x,possibleNextHexagons[optionChoosed].y);
            int whichSideItStarts = (possibleNumNexHexagons[optionChoosed]+3)%6;
            
            // add the next hexagon to the vector of hexagons in order to reconstruct the worm...
            growTileInfo g;
            g._num = ofAppPtr->hexagonCanvas.getHexagonNumberFromIdAndRing(startingHexagon);
            g._id = startingHexagon.x;
            g._ring = startingHexagon.y;
            g._startingAtSide = startingSide;
            g._endingAtSide = possibleNumNexHexagons[optionChoosed];
            
            ofAppPtr->growingHexagons.push_back(g);
            
            
//            if(!isFinishingGrow){
//                ofAppPtr->growingHexagons.pop_back();
//                //cout<<"removed"<<endl;
//            }
//            possibleNextHexagons.erase(possibleNextHexagons.begin()+optionChoosed);
//            possibleNumNexHexagons.erase(possibleNumNexHexagons.begin()+optionChoosed);
//            if(isFinishingGrow || !(possibleNextHexagons.size()>0)){
//                foundCulDeSac = true;
//            }
//            
        }
//        if(!isFinishingGrow) ofAppPtr->usedHexagons[vIndexData[startingHexagon.x][startingHexagon.y]] = false;
//        return isFinishingGrow;
    }
    else
    {
    }
}
