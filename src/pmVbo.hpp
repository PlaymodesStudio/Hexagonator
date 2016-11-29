//
//  pmVBO.hpp
//  guellGenerator
//
//  Created by emaduell on 11/11/16.
//
//

#ifndef pmVBO_hpp
#define pmVBO_hpp

#include <stdio.h>
#include "ofMain.h"

enum mode { TRIANGLES, CONTOURS};

class pmVbo
{
    public :
    
    pmVbo();
    void        setup(int _numElements,int _vertModulo);
    void        draw(int drawMode);

    /// VBO
    //////////
    ofVbo                           vbo;
    int                             currentVboVerts;
    int                             currentVboFaces;
    int                             currentVboColors;
    int                             currentVboTexCoords;
    
    void drawDebug(int _index);
    void setVertData(vector<ofVec3f> _v, int index);
    void setColorData(vector<ofFloatColor> _vc, int _index);
    void setFacesData(vector<ofIndexType> _vf, int _index);
    void setTexCoordsData(vector<ofVec2f> _v, int _index);
    void setTextureReference(ofTexture _tex);
    
    void setUseTexture(bool _b){ useTexture = _b;};
    void setTexCoordsIndex(int _i);
    void setDrawMode(mode _mode){ drawMode = _mode; cout << "changed draw mode to : " << drawMode << endl;};
    
    vector<ofVec3f>                 getVertices(int _index){return vecVboVerts[_index];};
    
    // POINT LINE CALCULATIONS
    ofPoint                         projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
    ofTexture                       texture;
    
    private :
    
    vector<vector<ofVec3f>>         vecVboVerts;
    vector<vector<ofIndexType>>     vecVboFaces;
    vector<vector<ofFloatColor>>    vecVboColors;
    vector<vector<ofVec2f>>         vecVboTexCoords;

    bool                            useTexture;
    mode                            drawMode;
    int                             vertModulo;
    int                             numElements;
};



#endif /* pmVBO_hpp */
