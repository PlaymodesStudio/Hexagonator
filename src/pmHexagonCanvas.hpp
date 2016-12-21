//
//  pmHexagonCanvas.hpp
//  8_GuellGenerator_2
//
//  Created by emaduell on 16/12/16.
//
//

#ifndef pmHexagonCanvas_hpp
#define pmHexagonCanvas_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxSVG.h"
#include "pmHexagonGuell.hpp"

typedef struct
{
    pmHexagonGuell      hexagon;
    int                 whichRing;
    int                 whichIdInRing;
}hexagonData;

class pmHexagonCanvas
{
    public :
    
        pmHexagonCanvas();
        void                        setup(string _filename);
    
        int                         getNumHexagons(){return numHexagons;};
        vector<ofVec3f>             getVertexData();
        vector<ofVec2f>             getTextureCoords();
        vector<ofFloatColor>        getColorData();
        vector<ofIndexType>         getFaceData();
        vector<ofPoint>             getCentroidData();
    
    private :

        void                        orderHexagonOnRingsAndIds(int i);
        ofPoint                     projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
    
        ofxSVG                      svg;
        vector<hexagonData>         hexagonsData;
        string                      svgFilename;
    
        int                         numHexagons;
        int                         numRings;
        int                         numIdsPerRing;

};

#endif /* pmHexagonCanvas_hpp */
