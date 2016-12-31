#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
#include "pmVbo.hpp"
#include "ofxSyphon.h"
#include "pmHexagonCanvas.hpp"
#include "ofxOsc.h"

typedef struct
{
    int _ring;
    int _num;
    int _pathId;
    float _scale;
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
    ofFbo               fboSyphon;
    ofVec2f             fboResolution;

    // IMAGE TEXTURE
    ofImage                 image;
    ofImage                 imageToSave;
    ofImage                 mask;
    
    // VIDEO
    ofVideoPlayer           videoPlayer;

    // SYPHON TEXTURE
    ofxSyphonClient         syphon;
    bool                    useSyphon;
    bool                    showVertices;
    bool                    useTransformMatrix;
    bool                    useCubeColors;

    /// HEXAGON CANVAS
    pmHexagonCanvas         hexagonCanvas;
    
    /// OSC
    ofxOscReceiver          oscReceiver;
    void                    updateOsc();
    vector<vector<float>>   vecOsc;
    
    /// VARS
    int                     numVertexsOneHexagonWithCenter;
    int                     numVertexsOneHexagon;
    int                     numRings;
    int                     numHexasPerRing;

    int                     mode;
    int                     numModes;
    string                  modeString;
    bool                    saveNow;
    int                     drawPrimitive;

    /// VBO
    pmVbo pmVbo1;

    // DATA VECTORS
    vector<ofVec3f>         vertexsTransformed;
    vector<ofVec3f>         vertexsOriginal;
    vector<ofVec3f>         vertexsRibbon;
    vector<ofPoint>         hexagonCentroids;
    
    vector<vector<hexagonPixel>>    hexaPix;

    // SHADER
    ofShader                shader;
    bool                    useShader;
    
    // MATRIX DATA UPDATE
    void                    updateMatrices();
    // TBO : Texture Buffer Object used to give encoded data to Shader as a texture
    ofTexture               texTransform;
    ofBufferObject          bufferTransform;
    vector<ofMatrix4x4>     matricesTransform;
    // TBO : Texture Buffer Object used to give colors to faces of "cubic" effect
    ofTexture               texCubeColors;
    ofBufferObject          bufferCubeColors;
    vector<ofFloatColor>    matricesCubeColors;
    
    
    /// ORDERS AND LOAD OPERATIONS
    ofPoint                 projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd);
    static bool             sortPointsOnDistanceToOrigin(ofPoint &p1, ofPoint &p2);
    vector<ofPoint>         reverseVerticesInVector(vector<ofPoint> _v);
    vector<ofPoint>         orderVerticesOfHexagonBasedOnDistanceToOrigin(vector<ofPoint> _v);
    void                    orderHexagonOnRingsAndIds(int _index);
    void                    updateVertexsForQuad();
    
    
};
