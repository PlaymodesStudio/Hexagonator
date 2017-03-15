#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
//#include "pmVbo.hpp"
#include "ofxSyphon.h"
#include "pmHexagonCanvas.hpp"
#include "ofxOsc.h"
#include "ofxVideoRecorder.h"
#include "ofxAVScreenCapture.h"
#include "parametersControl.h"
#include "pmHexagonTile.hpp"

enum sourceTextureType
{
    HEX_TEXTURE_IMAGE = 0,
    HEX_TEXTURE_VIDEO = 1,
    HEX_TEXTURE_SYPHON = 2,
    HEX_TEXTURE_SYPHON_MAX = 3,
    HEX_TEXTURE_CUBES = 4,
    HEX_TEXTURE_COLOR = 5
};
enum sourceType
{
    HEX_SOURCE_TEXTURE = 0,
    HEX_SOURCE_QUADS = 1,
    HEX_SOURCE_CUCS = 2,
    HEX_SOURCE_RANDOM = 3
};


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
        void exit();

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
	
    ///
    void prepareQuads();
    void prepareCucs();
    void prepareHexagons();
    void prepareRandom();
    void buildTBOs();
    
    // MATRIX DATA UPDATE
    void                    updateMatrices();
    void                    updateCubeColors();
    void                    updateVertexsForQuad();
    void                    updateCucs();
    void                    updateRandom();

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
    string                  imageFilename;
    ofImage                 imageToSave;
    ofImage                 mask;
    ofImage                 maskWireframe;
    
    // VIDEO
    ofVideoPlayer           videoPlayer;
    string                  videoFilename;
    string                  currentFolderName;
    int                     recordedFrame;
    
    // SYPHON TEXTURE
    ofxSyphonClient         syphon;
    ofxSyphonClient         syphonMax;
    bool                    useSyphon;
    bool                    useTransformMatrix;

    
    /// OSC
    ofxOscReceiver          oscReceiver;
    void                    updateOsc();
    vector<vector<float>>   vecOsc;
    float                   lastTimeWeReceivedOsc;
    bool                    usingOsc;
    void                    resetVecOscVector();
    
    /// VARS
    int                     numVertexsOneHexagonWithCenter;
    int                     numVertexsOneHexagon;
    int                     numRings;
    int                     numHexasPerRing;

    int                     mode;
    int                     numModes;
    string                  modeString;
    bool                    saveNow;

    /// HEXAGON CANVAS
    pmHexagonCanvas         hexagonCanvas;

    /// VBO    
    ofVbo vboTex;
    ofVbo vboQuads;
    ofVbo vboCucs;
    ofVbo vboRandom;
    
    // VBO DATA
    vector<ofVec3f>                 vecVboTex_Verts;
    vector<ofIndexType>             vecVboTex_Faces;
    vector<ofFloatColor>            vecVboTex_Colors;
    vector<vector<ofVec2f>>         vecVboTex_TexCoords;

    vector<ofVec3f>                 vecVboQuads_Verts;
    vector<ofFloatColor>            vecVboQuads_Colors;
    vector<ofIndexType>             vecVboQuads_Indexs;
    
    vector<ofVec3f>                 vecVboCucs_Verts;
    vector<ofIndexType>             vecVboCucs_Faces;
    vector<ofFloatColor>            vecVboCucs_Colors;
    vector<ofVec2f>                 vecVboCucs_TexCoords;

    vector<ofVec3f>                 vecVboRandom_Verts;
    vector<ofIndexType>             vecVboRandom_Faces;
    vector<ofFloatColor>            vecVboRandom_Colors;

    
    // VBO CUCS STUFF
    //////////////////////
    ofPolyline hexagon;
    vector<ofVec3f> hexaPoints;
    vector<ofVec3f> hexaSides;
    
    int startSide;
    int endSide;
    
    ofVec3f startPoint;
    ofVec3f endPoint;
    
    float   cucWidth;
    float   widthStart;
    float   widthEnd;
    ofPolyline bezierLine;
    vector<ofVec3f> sampledPoints;
    // RIBS
    // init ribs vector
    // each "rib" is 2 x ofVec2f with the coordinates of the segment of each rib [Vert.Left , Vert.Right]
    vector<vector<ofVec3f>> ribs;
    vector<pmHexagonTile>   hexagonTiles;
    
    int numSteps;
    int lastFaceAddedToCucs;
    
    vector<ofVec3f>         vecTempVbo_Verts;
    vector<ofFloatColor>    vecTempVbo_Colors;
    vector<ofIndexType>     vecTempVbo_Faces;
    vector<ofVec2f>         vecTempVbo_TexCoords;
    

    
    void                    calculateStartEndPointsAndCurve();
    void                    calculateRibs();
    void                    calculateVboData();
    void                    calculateSides();
    void                    calculateTilePatterns();
    
    /// RANDOM
    float                   lastRandomTime;
    
    
    // DATA VECTORS
    vector<ofVec3f>         vertexsRibbon;

    // SHADER
    ofShader                shader;
    bool                    useShader;
    
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
    
    // VIDEO RECORDING
    ofxVideoRecorder        videoRecorder;
    bool                    isRecording;
    string                  fileName;
    string                  fileExt;
    int                     desiredFramerate;
    void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);

    ofxAVScreenCapture      capture;
    
    //GUI
    //Each gui window is a parameterGroup, each parameter in parameterGroup a element in the gui. The parametersControl class keeps parameterGroup and gui is sync. The translation of parameterGroup to datgui elements is as follows:
    // ofParameter<int>     --->    int slider;
    // ofParameter<float>   --->    float slider;
    // ofParameter<bool>    --->    toggle;
    // ofParameter<string>  --->    textfield (if the name of parameter ends with '_label', it becomes a label)
    // ofParameter<ofColor> --->    colorPicker element;
    // especial case: to use a dropdown you need a ofParameter<int> that will hold the selected index. For creating a dropdown you can use the static function addDropdownToParameterGroupFromParameters, from parametersControl;
    
    // DEMO GUI
    ofParameterGroup    parameters;
    ofParameter<int>    dropdownTest;
    ofParameter<string> labelTest;
    ofParameter<string> saveFilename;
    ofParameter<ofColor>    colorPicker;
    
    // GRAPHICS & OPTIONS
    ofParameterGroup    parametersGraphics;
    //ofParameter<int>    param_whichTexCoord;
    ofParameter<int>    dropdown_whichSource;
    ofParameter<int>    dropdown_whichTexCoord;
    ofParameter<int>    dropdown_whichTextureSource; // 0 : image 1 : video 2 : syphon
    ofParameter<bool>   toggle_drawMask;
    ofParameter<bool>   toggle_showVertices;
    ofParameter<bool>   toggle_showLayout;
    ofParameter<bool>   toggle_useTBOMatrix;
    ofParameter<ofColor> color_shaderColorA;
    ofParameter<ofColor> color_shaderColorB;
    
    // LISTENERS FUNCTIONS
    void                changedTexCoord(int &i);
    void                changedTexSource(int &i);
    void                changedSource(int &i);
    
    // RANDOM GUI
    ofParameterGroup    parametersRandom;
    ofParameter<int>    slider_howManyRandomHexagons;
    ofParameter<float>  slider_decreaseRate;
    ofParameter<float>  slider_randomPeriod;
    
    // RECORDING GUI
    ofParameterGroup    parametersRecording;
    ofParameter<bool>   startStopRecording;
    ofParameter<int>    framesToRecord;
    ofParameter<string> recFilename;
    
};
