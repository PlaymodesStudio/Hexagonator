#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
#include "pmVbo.hpp"
#include "ofxSyphon.h"

typedef struct
{
    int _hexaCentroidIndex;
    int _ring;
    int _num;
} hexagonPixel;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    /// SVG
    string              svgFilename;
    ofxSVG              svg;
    vector<ofPolyline>  svgPolys;
    vector<ofPath>      svgPaths;
    

    /// FBO
    ofFbo               fboOut;
    ofVec2f             fboResolution;

    /// VARS
    int                 mode;
    int                 numModes;
    string              modeString;
    bool                saveNow;
    int                 numVertexsOneHexagonWithCenter;
    int                 numVertexsOneHexagon;
    int                 drawPrimitive;
    
    /// VBO
    pmVbo               pmVbo1;
    
    
    vector<ofPoint>         svgVecPoints;
    vector<ofPoint>         polygonCentroids;
    
    ofVboMesh               vboMesh;
    ofVbo                   vbo;
    vector<vector<ofVec3f>> vecVboVerts;
    vector<ofIndexType>     vboFaces;
    vector<ofFloatColor>    vboColor;
    vector<vector<ofVec2f>> vecVboTexCoords;

    // TEXTURE 
    ofImage                 image;
    ofImage                 imageToSave;
    
    ofShader                shader;
    bool                    useShader;
    
    void                    updateMatrices();

    ofTexture               tex;
    ofBufferObject          buffer;
    vector<ofMatrix4x4>     matrices;

//    ofTexture               texB;
//    ofBufferObject          bufferB;
//    vector<ofMatrix4x4>     matricesB;

    ofPoint                 projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
    static bool             sortPointsOnDistanceToOrigin(ofPoint &p1, ofPoint &p2);
    vector<ofPoint>         reverseVerticesInVector(vector<ofPoint> _v);
    vector<ofPoint>         orderVerticesOfHexagonBasedOnDistanceToOrigin(vector<ofPoint> _v);
    void                    orderHexagonOnRings(int _index);
    
    int                     numRings;
    int                     numHexasPerRing;
    vector<vector<hexagonPixel>>    hexaPix;
    
    // SYPHON
    ofxSyphonClient         syphon;
    bool                    useSyphon;
    
};
