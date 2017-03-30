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
#include "ofxEditableSVG.h"
#include "pmHexagonGuell.hpp"

typedef struct
{
    pmHexagonGuell      hexagon;
    int                 whichRing;
    int                 whichIdInRing;
    float               scale;
}hexagonData;

class pmHexagonCanvas
{
    public :
    
        pmHexagonCanvas();
        void                        setup(string _filename);
    
        int                         getNumHexagons(){return numHexagons;};
        vector<ofVec3f>             getVertexData();
        vector<ofVec3f>             getOriginalVertexData();
        vector<ofVec2f>             getTextureCoords();
        vector<ofFloatColor>        getColorData();
        vector<ofIndexType>         getFaceData();
        vector<ofPoint>             getCentroidData();
        ofVec2f                     getHexagonIdAndRing(int _index){return ofVec2f(hexagonsData[_index].whichIdInRing , hexagonsData[_index].whichRing );};
        vector<vector<int>>         getHexagonsIndexData(){return hexagonsIndexData;};
        int                         getHexagonNumberFromIdAndRing(ofVec2f idRing);
//        void                        setHexagonScale(int _index, float _f){hexagonsData[_index].scale=_f;};
//        float                       getHexagonScale(int _index){return hexagonsData[_index].scale;};
    
        vector<float>               ringsRadius;
    
    private :

        void                        orderHexagonOnRingsAndIds(int i);
        ofPoint                     projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
    
        ofxEditableSVG              svg;
        vector<hexagonData>         hexagonsData;
        vector<vector<int>>         hexagonsIndexData;  // each index gives the position on the hexagonsData vector of the [ring][id] element.
        string                      svgFilename;
    
        int                         numHexagons;
        int                         numRings;
        int                         numIdsPerRing;

};

#endif /* pmHexagonCanvas_hpp */
