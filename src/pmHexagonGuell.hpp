//
//  pmHexagonGuell.h
//  6_guellGeneratorVBO_TBO
//
//  Created by emaduell on 15/12/16.
//
//

#ifndef pmHexagonGuell_hpp
#define pmHexagonGuell_hpp

#include <stdio.h>
#include "ofMain.h"


class pmHexagonGuell
{
    public :
    
    pmHexagonGuell();
    void                    setup(ofPath _path, int _pathId, ofVec2f _compositionResolution);
    void                    setHexagonColor(vector<ofFloatColor> _colors){colors = _colors;};
    void                    setHexagonTexCoord(vector<ofVec2f> _texCoord){texCoordinates = _texCoord;};

    ofPoint                 getCentroid(){return centroid;};
    vector<ofPoint>         getVertexs(){return vertexs;};
    vector<ofFloatColor>    getColors(){return colors;};
    vector<ofVec2f>         getTexCoordinates(){return texCoordinates;};
    vector<ofIndexType>     getFaces(){return faces;};

    private :
    
    int                     pathId;

    vector<ofPoint>         vertexsOriginals;
    
    vector<ofPoint>         vertexs;
    vector<ofIndexType>     faces;
    vector<ofFloatColor>    colors;
    vector<ofVec2f>         texCoordinates;

    ofPoint                 centerOrigin;
    ofVec2f                 compositionResolution;
    ofPoint                 centroid;
    
    
    vector<ofPoint>         reverseVerticesInVector(vector<ofPoint> _v);
    vector<ofPoint>         orderVerticesOfHexagonBasedOnDistanceToOrigin(vector<ofPoint> _v);
    ofPoint                 projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
};

#endif /* pmHexagonGuell_h */