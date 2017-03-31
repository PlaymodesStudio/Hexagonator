
#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup(){
    
    
    // Use GL_TEXTURE_2D Textures (normalized texture coordinates 0..1)
    ofDisableArbTex();
    
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofBackground(32);
    
    /////////////////////////////
    /// VARS
    /////////////////////////////
    
    mode = 0;
    numModes = 1;
    numRings = 35;
    numHexasPerRing = 64;
    useShader = true;
    toggle_useTBOMatrix = true;
    numVertexsOneHexagonWithCenter = 7;
    numVertexsOneHexagon = 6;
    recordedFrame = 0;
    lastTimeWeReceivedOsc = 0.0;
    usingOsc = false;
    lastRandomTime = ofGetElapsedTimeMillis();
    
    /////////////////////////////
    /// GUI
    /////////////////////////////
    
    
    // GRAPHICS
    dropdown_whichTextureSource = HEX_TEXTURE_VIDEO; // image as initial texture source
    dropdown_whichTexCoord = 0 ;
    
    parametersGraphics.setName("Hexagonator");
    parametersGraphics.add(toggle_showLayout.set("Show Layout",true));
    parametersGraphics.add(toggle_showVertices.set("Show Vertices",false));
    parametersGraphics.add(toggle_drawMask.set("Draw Mask",true));
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Texture Coordinates",{"64x35","1200x1200"},dropdown_whichTexCoord);
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Source",{"Texture","Quads","Cucs","Random","Grow"},dropdown_whichSource);
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Texture Source",{"Image","Video","Syphon","Syph.Max","Cubes","Colors AB"},dropdown_whichTextureSource);
    parametersGraphics.add(color_shaderColorA.set("Color A", ofColor::white, ofColor::white, ofColor::black));
    parametersGraphics.add(color_shaderColorB.set("Color B", ofColor::white, ofColor::white, ofColor::black));
    
    // LISTENERS
    dropdown_whichTexCoord.addListener(this,&ofApp::changedTexCoord);
    dropdown_whichTextureSource.addListener(this,&ofApp::changedTexSource);
    dropdown_whichSource.addListener(this,&ofApp::changedSource);
    
    // RANDOM GUI
    slider_howManyRandomHexagons = 1;
    slider_decreaseRate = 0.10;
    slider_randomPeriod = 0.250;
    parametersRandom.setName("Random");
    parametersRandom.add(slider_howManyRandomHexagons.set("Num.Elem.", slider_howManyRandomHexagons,0,500));
    parametersRandom.add(slider_decreaseRate.set("Fade Out", slider_decreaseRate,0.0,0.10));
    
    
    
    parametersRandom.add(slider_randomPeriod.set("Period(ms)", slider_randomPeriod,0.0,1000.0));
    
    
    // RECORDING GUI
    parametersRecording.setName("Recording");
    parametersRecording.add(startStopRecording.set("Recording", false));
    parametersRecording.add(framesToRecord.set("frames to Rec", 100, 1, 99999));
    parametersRecording.add(recFilename.set("Filename", ofGetTimestampString()+".mov"));

    // DRAWING GUI
    guiDrawing = new ofxDatGui();

    drawingSizeX.set("Size X",4,1,64);
    drawingSizeY.set("Size Y",20,1,35);
    drawingSizeX.addListener(this,&ofApp::changedMatrixX);
    drawingSizeX.addListener(this,&ofApp::changedMatrixY);

    ofxDatGuiTheme* theme = new ofxDatGuiThemeCharcoal;
    ofColor randColor =  ofColor::indianRed;
    theme->color.slider.fill = randColor;
    theme->color.textInput.text = randColor;
    theme->color.icons = randColor;
    guiDrawing->setTheme(theme);
    
    guiDrawing->setWidth(290);
    guiDrawing->addHeader("Draw");
    guiDrawing->addFooter();

    drawingMatrix = guiDrawing->addMatrix("Drawing", drawingSizeX*drawingSizeY,true);
    drawingMatrix->setRadioMode(false);
    guiDrawing->setWidth(drawingSizeX*50);

    drawingMatrix->onMatrixEvent(this, &ofApp::onGuiMatrixEvent);

    // 400 .. 10 cols
    // 350 .. 9 cols
    // 300 .. 8
    //
    // 200 .. 5
    // 100 .. 2
//    guiDrawing->addSlider(drawingSizeX, 1, 32);
    guiDrawing->addSlider(drawingSizeX);
    guiDrawing->addSlider(drawingSizeY);
    parametersControl::getInstance().addDatGui(guiDrawing);
    

    
    // CREATE
    parametersControl::getInstance().createGuiFromParams(parametersGraphics, ofColor::orange);
    parametersControl::getInstance().createGuiFromParams(parametersRandom, ofColor::red);
    parametersControl::getInstance().createGuiFromParams(parametersRecording, ofColor::white);
    parametersControl::getInstance().setup();
    parametersControl::getInstance().setSliderPrecision(2,"Fade Out", 6);
    
    parametersControl::getInstance().distributeGuis();
    
    
    /////////////////////////////
    /// VIDEO RECORDING
    /////////////////////////////
    
    isRecording = false;
    
    //    fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    //    desiredFramerate = 60;
    //
    //    // override the default codecs if you like
    //    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    //    // prores codecs info !! https://trac.ffmpeg.org/wiki/Encode/VFX
    //
    ////    videoRecorder.setVideoCodec("prores");
    //    videoRecorder.setVideoCodec("png");
    //    //videoRecorder.setVideoBitrate("800k");
    //    //videoRecorder.setAudioCodec("mp3");
    //    //videoRecorder.setAudioBitrate("192k");
    //    ofAddListener(videoRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    
    /////////////////////////////
    /// SHADER
    /////////////////////////////
    {
#ifdef TARGET_OPENGLES
        shader.load("shadersES2/shader");
#else
        if(ofIsGLProgrammableRenderer())
        {
            cout << "Working on GL Programable Render mode" << endl;
            shader.load("shadersGL3/shader");
        }else
        {
            cout << "Working on a NOT GL Programable Render mode" << endl;
            shader.load("shadersGL2/shader");
        }
#endif
    }
    
    /////////////////////////////
    /// FBO
    /////////////////////////////
    
    {
        fboResolution = ofVec2f(1200,1200);
        
        fboOut.allocate(fboResolution.x, fboResolution.y,GL_RGB,8);
        //fboOut.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        fboOut.begin();
        ofClear(255,0,255, 0);
        fboOut.end();
        
        fboSyphon.allocate(fboResolution.x, fboResolution.y,GL_RGB,8);
        //fboOut.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        fboSyphon.begin();
        ofClear(255,0,255, 0);
        fboSyphon.end();
    }
    
    /////////////////////////////
    /// SYPHON
    /////////////////////////////
    {
        syphon.setup();
        syphonMax.setup();
        
        //using Syphon app Simple Server, found at http://syphon.v002.info/
        //    syphon.setApplicationName("Simple Server");
        //    syphon.setServerName("");
        //    syphon.set("","Simple Server");
        syphon.set("Gen_Grayscale","MIRABCN_Generator");
        syphon.bind();
        syphon.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        syphon.unbind();
        
        syphonMax.set("MIDIFICATOR","midiToHexagonoDebug");
        syphonMax.bind();
        syphonMax.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        syphonMax.unbind();
        
        // SERVER
        //syphonServer.setName("Main");
        
    }
    
    /////////////////////////////
    /// IMAGES AND VIDEOS
    /////////////////////////////
    {
        imageFilename = "./testMedia/mapaPixels64x35_White.png";
        image.load(imageFilename);
        image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        
        // MASK
        mask.load("./testMedia/masks/maskAll.png");
        maskWireframe.load("./testMedia/masks/wireYellow.png");
        
        // VIDEO
        videoFilename = "./testMedia/rings.mov";
        videoPlayer.load(videoFilename);
        videoPlayer.setLoopState(OF_LOOP_NORMAL);
        if(dropdown_whichTextureSource == HEX_TEXTURE_VIDEO) videoPlayer.play();
        
        
        while(!videoPlayer.isLoaded())
        {
            cout << " ... loading video " << endl;
        }
        videoPlayer.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        
    }
    
    ////////////
    /// OSC
    ////////////
    {
        oscReceiver.setup(1234);
        resetVecOscVector();
    }
    
    /////////////////////////////
    // HEXAGONS DATA AND CANVAS
    /////////////////////////////
    
    //svgFilename = "./svg/test_svg_part.svg";
    svgFilename = "./svg/santi3_App.svg";
    //    svgFilename = "./svg/testSVG9Hexagons.svg";
    //        svgFilename = "./svg/testSVG3Hexagons.svg";
    //    svgFilename = "./svg/test_svg_part_nomes2.svg";
    //    svgFilename = "./svg/testOrdreRadial.svg";
    //    svgFilename = "./svg/polarExampleAngle.svg";
    //    svgFilename = "./svg/test_svg_partCENTRAT.svg";
    //    svgFilename = "./svg/vector_complet_v1.svg";
    //    svgFilename = "./svg/vector_complet_v2_flat.svg";
    
    hexagonCanvas.setup(svgFilename);
    
    /////////////////////////////
    /// VBO TEXTURE
    /////////////////////////////
    
    prepareHexagons();
    
    ///////////////////
    /// PM VBO QUADS
    ///////////////////
    
    prepareQuads();
    
    ///////////////////
    // VBO CUCS STUFF
    ///////////////////
    
    prepareCucs();
    
    ///////////////////
    // RECURSIVE CUCS
    ///////////////////
    
    
    
    //    vector<vector<int>> vIndexData = hexagonCanvas.getHexagonsIndexData();
    //    cout << "V Index Data sizes : " << vIndexData.size() << " , " << vIndexData[0].size() << endl;
    //
    //        for(int i=0;i<vIndexData.size();i++)
    //        {
    //            for(int j=0;j<vIndexData[i].size();j++)
    //            {
    //                cout << "index : " << i << " , " << j << " = " << vIndexData[i][j] << endl;
    //            }
    //
    //        }
    
    // CREATE A GROW PATH ...
    ///////////////////////////////////////////
    ///////////////////////////////////////////
    
    usedHexagons.resize(hexagonCanvas.getNumHexagons(),false);
    occupyOneHexagon(ofVec2f(10,0),0);
    
    int howManyHexagonsOccupied = 0;
    for(int i=0;i<usedHexagons.size();i++)
    {
        if(usedHexagons[i]==true)
        {
            howManyHexagonsOccupied = howManyHexagonsOccupied +1 ;
        }
    }
    cout << " Occupation done ... Used : " << howManyHexagonsOccupied << endl;

//    // CREATE A GROW PATH ... MANUAL
//    ///////////////////////////////////////////
//    ///////////////////////////////////////////
//    
//    int howManyHexagonsOccupied = 0;
//    usedHexagons.resize(hexagonCanvas.getNumHexagons(),false);
//    //occupyOneHexagon(ofVec2f(0,0),0);
//    for(int i=0;i<hexagonCanvas.getNumHexagons();i++)
//    {
//        if(hexagonCanvas.getHexagonIdAndRing(i)[0] == 0)
//        {
//            // we are in id = 0 ... select this hexagon !!
//            usedHexagons[i] = true;
//            howManyHexagonsOccupied = howManyHexagonsOccupied +1 ;
//        }
//    }
//    cout << " Occupation done ... Used : " << howManyHexagonsOccupied << endl;
//
    
    
    ///////////////////////////////////////////
    ///////////////////////////////////////////
    
    //buildNumElementsTBO();
    prepareGrow();
    
    
    ///////////////////
    // VBO CUCS RANDOM
    ///////////////////
    
    prepareRandom();
    
    /// TRANSFORM TBO STUFF (Texture Buffer Object)
    ///////////////////////////////////////////////
    
    buildTBOs();
    
    /// SHADER INITS
    ////////////////
    
    {
        shader.begin();
        shader.setUniformTexture("texTransform",texTransform,0);
        shader.setUniformTexture("texCubeColors",texCubeColors,1);
        //shader.setUniformTexture("texNumElements",texNumElements,1);
        
        shader.setUniform1i("u_numHexags",hexagonCanvas.getNumHexagons());
        shader.setUniform4f("u_color", ofFloatColor(0.5,0.5,0.5,1.0));
        shader.setUniform1i("u_source", 0);
        shader.setUniform1i("u_textureSource", 1);
        

        shader.end();
        
        /// 
//
//        shader.begin();
//            shader.bindAttribute(5, "u_vertexHexagonId");
//        shader.linkProgram();
//        shader.end();
//        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
//        shader.linkProgram();

//        shader.setupShaderFromSource(<#GLenum type#>, <#string source#>)
//        ofShader::setupShaderFromSource(...)
//        ofShader::bindAttribute(customLocation, "myattribute");
//        ofShader::linkProgram()
        
    }
    
    updateTransformMatrices();
    
    
}




//--------------------------------------------------------------
void ofApp::updateOsc()
{
    float now = ofGetElapsedTimef();
    
    if(oscReceiver.hasWaitingMessages())
    {
        usingOsc = true;
        
        lastTimeWeReceivedOsc = now;
        
        ofxOscMessage m;
        oscReceiver.getNextMessage(m);
        //cout << m.getAddress() << endl;
        
        int whichId;
        int whichRing;
        
        for(int i=0;i<m.getNumArgs();i++)
        {
            //int                         getHexagonIndex(int x, int y){return hexagonsIndexData[x][y];};
            whichRing = i % 35;
            whichId = int(float(i) / 35.0);
            
            // save the osc value in the vector<vector<>>
            vecOsc[whichId][whichRing] = m.getArgAsFloat(i);
        }
        
    }
    
    if( ((now - lastTimeWeReceivedOsc) > 3.0) && usingOsc )
    {
        usingOsc = false;
        cout << "RESETTING OSC VECTOR :: seems like we're not receiving any more OSC????  "  << (now - lastTimeWeReceivedOsc) << endl;
        
        resetVecOscVector();
    }
}

//--------------------------------------------------------------
void ofApp::updateTransformMatrices()
{
    vector<ofPoint> centr = hexagonCanvas.getCentroidData();
    
    // TRANSFORM MATRIX
    for(size_t i=0;i<matricesTransform.size();i++)
    {
        ofNode node;
        ofVec3f scale(1.0,1.0,1.0);
        
        float factor = sin (ofGetElapsedTimef()*2 + (i*0.5));
        
        ofVec2f myHexagonCoordinates = hexagonCanvas.getHexagonIdAndRing(i);
        factor = vecOsc[myHexagonCoordinates.x][myHexagonCoordinates.y];
        //cout << "scale " << i << " : " << factor << endl;
        
        
        node.setPosition(ofVec3f(centr[i]));
        node.setScale((ofMap(factor,0.0,1.0,0.0,1.0)/1.0));
        
        matricesTransform[i] = node.getLocalTransformMatrix();
    }
    // and upload them to the texture buffer
    bufferTransform.updateData(0,matricesTransform);
    
    
}
//--------------------------------------------------------------
void ofApp::updateNumElementsMatrices()
{
    // and upload them to the texture buffer
    
    bufferNumElements.updateData(0,matricesNumElements);
    
    
}
//--------------------------------------------------------------
void ofApp::updateCubeColorsMatrices()
{
    // TRANSFORM CUBE COLORS
    for(size_t i=0;i<hexagonCanvas.getNumHexagons();i++)
    {
        //        ofFloatColor cube1(fabs(1.0 * sin(ofGetElapsedTimef()/5.0)),0.0,0.0,1.0);
        //        ofFloatColor cube2(0.0,fabs(1.0 * cos(ofGetElapsedTimef())),0.0,1.0);
        //        ofFloatColor cube3(0.0,0.0,fabs(1.0 * sin(ofGetElapsedTimef()/13.0)),1.0);
        
        ofFloatColor cube1(0.0,0.75,0.85,1.0);
        ofFloatColor cube2(0.0,0.5,0.65,1.0);
        ofFloatColor cube3(0.0,0.25,0.55,1.0);
        
        matricesCubeColors[(i*3)+0] = cube1;
        matricesCubeColors[(i*3)+1] = cube2;
        matricesCubeColors[(i*3)+2] = cube3;
    }
    // and upload them to the texture buffer
    bufferCubeColors.updateData(0,matricesCubeColors);
    
}

//--------------------------------------------------------------
void ofApp::updateVertexsForQuad()
{
    vector<ofPoint> vertexsOriginal = hexagonCanvas.getVertexData();
    //    vector<ofPoint> vertexsOriginal = hexagonCanvas.getOriginalVertexData();
    
    float ribbonWidth = 1.0 * (sin(ofGetElapsedTimef()) + 1.0)/8.0 ;
    //ribbonWidth = ofRandomuf();
    
    //ribbonWidth = 1.0;
    
    int howManyHexagonsWeVisited = 0;
    
    for(int i=0;i<hexagonCanvas.getNumHexagons();i++)
    {
        // get id and ring of each hexagon processed ...
        ofVec2f hexaIdRing = hexagonCanvas.getHexagonIdAndRing(i);
        
        // if id != -1 then it's a valid hexagon ...
        if(hexaIdRing[0] != -1 )
        {
            vector<ofPoint>     vertOfHexagon;
            ofPoint p1,p2,p3,p4;
            ofVec2f v01,v34,v23,v50;
            ofVec2f vA,vB;
            vertOfHexagon.resize(numVertexsOneHexagonWithCenter);
            float ribbonWidthDivider = 1.0;
            
            // first get the 7 original vertices of the array
            for(int k=0;k<7;k++)
            {
                //                    int w = hexaPix[j][i]._pathId;
                int w = hexaIdRing[0];
                //                    vertOfHexagon[k] = vertexsOriginal[(w*numVertexsOneHexagonWithCenter)+1 + k ];
                vertOfHexagon[k] = vertexsOriginal[(i*numVertexsOneHexagonWithCenter)+1 + k ];
            }
            
            
            //!!!!!!!!!! !!!
            //ribbonWidthDivider = 4.0 * (float(hexaPix[j][i]._ring + 0)) / float(numRings);
            // !!!!!!!!
            ribbonWidthDivider = 2.0;
            
            /// VERTS RIBBON [2] : 4v for each hexagon -> to draw the ribbon
            ////////////////////////////////////////////////////////
            
            // A
            v01 = ofVec2f(vertOfHexagon[1] - vertOfHexagon[0]);
            v34 = ofVec2f(vertOfHexagon[4] - vertOfHexagon[3]);
            
            // B
            v23 = ofVec2f(vertOfHexagon[3] - vertOfHexagon[2]);
            v50 = ofVec2f(vertOfHexagon[0] - vertOfHexagon[5]);
            
            
            
            //
            //        // VERTS RIBBON [2] : 4v for each hexagon -> to draw the ribbon
            //        ////////////////////////////////////////////////////////
            //        ofPoint p1,p2,p3,p4;
            //        float ribbonWidth = 0.1;
            //
            //        ofVec2f v01 = ofVec2f(vecOrdered[1] - vecOrdered[0]);
            //        ofVec2f v34 = ofVec2f(vecOrdered[4] - vecOrdered[3]);
            //
            //        p1 = vecOrdered[0] + (v01/2.) - (v01 * (ribbonWidth/2.0));
            //        p2 = vecOrdered[0] + (v01/2.) + (v01 * (ribbonWidth/2.0));
            //        p3 = vecOrdered[3] + (v34/2.) - (v34 * (ribbonWidth/2.0));
            //        p4 = vecOrdered[3] + (v34/2.) + (v34 * (ribbonWidth/2.0));
            //        //p2 = vecOrdered[0] + (v01 * 0.66);
            //        //            p3 = vecOrdered[3] + (v34 * 0.33);
            //        //            p4 = vecOrdered[3] + (v34 * 0.66);
            //
            //        vecVboQuads_Verts[(i*4)+0] = p1;
            //        vecVboQuads_Verts[(i*4)+1] = p2;
            //        vecVboQuads_Verts[(i*4)+2] = p3;
            //        vecVboQuads_Verts[(i*4)+3] = p4;
            
            
            
            //           typedef struct
            //        {
            //            int _hexaCentroidIndex;
            //            int _ring;
            //            int _num;
            //        } hexagonPixel;
            
            
            p1 = vertOfHexagon[2] + (v23/2.) - (v23 * (ribbonWidth*ribbonWidthDivider));
            p2 = vertOfHexagon[2] + (v23/2.) + (v23 * (ribbonWidth*ribbonWidthDivider));
            p3 = vertOfHexagon[5] + (v50/2.) - (v50 * (ribbonWidth*ribbonWidthDivider));
            p4 = vertOfHexagon[5] + (v50/2.) + (v50 * (ribbonWidth*ribbonWidthDivider));
            //p2 = vecOrdered[0] + (v01 * 0.66);
            //            p3 = vecOrdered[3] + (v34 * 0.33);
            //            p4 = vecOrdered[3] + (v34 * 0.66);
            
            vecVboQuads_Verts[(howManyHexagonsWeVisited*4)+0] = p1;
            vecVboQuads_Verts[(howManyHexagonsWeVisited*4)+1] = p2;
            vecVboQuads_Verts[(howManyHexagonsWeVisited*4)+2] = p3;
            vecVboQuads_Verts[(howManyHexagonsWeVisited*4)+3] = p4;
            //
            
            
            
            howManyHexagonsWeVisited = howManyHexagonsWeVisited + 1 ;
            
        }
        
    }
    
    //    pmVboRibbon.setVertData(vecVboQuads_Verts,hexagonCanvas.getNumHexagons());
    //vboQuads.setVertexData(vecVboQuads_Verts.data(), vecVboQuads_Verts.size(),GL_DYNAMIC_DRAW);
    vboQuads.updateVertexData(vecVboQuads_Verts.data(), vecVboQuads_Verts.size());
    
    //    cout << "ribbon width : " << ribbonWidth << " HowMany Visited : " << howManyHexagonsWeVisited <<  endl;
    //    cout << vecVboQuads_Verts[0][0] << endl;
    
}

//--------------------------------------------------------------
void ofApp::update()
{
    updateOsc();
    updateTransformMatrices();
    
    if(dropdown_whichSource == HEX_SOURCE_QUADS )
    {
        // HEX_SOURCE_QUADS
        updateVertexsForQuad();
    }
    else if(dropdown_whichSource == HEX_SOURCE_TEXTURE)
    {
        // HEX_SOURCE_TEXTURE
        updateCubeColorsMatrices();
    }
    else if(dropdown_whichSource == HEX_SOURCE_RANDOM)
    {
        updateRandom();
    }
    else if(dropdown_whichSource == HEX_SOURCE_GROW)
    {
        updateNumElementsMatrices();
    }
    
    if(dropdown_whichSource==HEX_SOURCE_TEXTURE && dropdown_whichTextureSource==HEX_TEXTURE_VIDEO && videoPlayer.isLoaded())
    {
        videoPlayer.update();
    }
    
    
    //syphonServer.publishTexture(&fboOut.getTexture());
}

//--------------------------------------------------------------
void ofApp::draw()
{
    //pmVboRibbon.updateVertData(vecVboQuads_Verts,0);
    //vbo.updateVertexData(vecVboVerts[currentVboVerts].data(), vecVboVerts[currentVboVerts].size());
    
    /// DRAW SYPHON INTO FBO TO LATER RETRIEVE IT's TEXTURE
    if( dropdown_whichSource==HEX_SOURCE_TEXTURE && (dropdown_whichTextureSource == HEX_TEXTURE_SYPHON || dropdown_whichTextureSource == HEX_TEXTURE_SYPHON_MAX))
    {
        fboSyphon.begin();
        if(dropdown_whichTextureSource == HEX_TEXTURE_SYPHON) syphon.draw(0,0,fboResolution.x,fboResolution.y);
        else if(dropdown_whichTextureSource == HEX_TEXTURE_SYPHON_MAX) syphonMax.draw(0,0,fboResolution.x,fboResolution.y);
        fboSyphon.end();
    }
    
    //////////////////////
    /// DRAW INTO FBO
    //////////////////////
    
    fboOut.begin();
    {
        // draw background
        ofSetColor(0,0,0);
        ofFill();
        ofDrawRectangle(0,0,1200,1200);
        
        if(useShader)
        {
            shader.begin();
            
            // choose which texture to feed into the shader (image or syphon)
            if(dropdown_whichSource == HEX_SOURCE_TEXTURE)
            {
                shader.setUniform1i("u_modulo",7);
                
                switch (dropdown_whichTextureSource)
                {
                    case HEX_TEXTURE_IMAGE:
                    shader.setUniformTexture("uTexture", image, 2);
                    break;
                    case HEX_TEXTURE_VIDEO:
                    shader.setUniformTexture("uTexture", videoPlayer, 2);
                    break;
                    case HEX_TEXTURE_SYPHON:
                    shader.setUniformTexture("uTexture", fboSyphon.getTexture(), 2);
                    break;
                    case HEX_TEXTURE_SYPHON_MAX:
                    shader.setUniformTexture("uTexture", fboSyphon.getTexture(), 2);
                    break;
                    
                    default:
                    break;
                }
            }
            else if(dropdown_whichSource == HEX_SOURCE_QUADS)
            {
                shader.setUniform1i("u_modulo",4);
            }
            else if(dropdown_whichSource == HEX_SOURCE_CUCS)
            {
                shader.setUniform1i("u_modulo",numSteps*2*6);
            }
            else if(dropdown_whichSource == HEX_SOURCE_RANDOM)
            {
                shader.setUniform1i("u_modulo",7);
            }
            else if(dropdown_whichSource == HEX_SOURCE_GROW)
            {
                shader.setUniform1i("u_modulo",numSteps*2*6);
            }
            
            
            //                shader.setUniformTexture("uTexture", videoPlayer, 2);
            //                shader.setUniformTexture("uTexture", image, 2);
            //                image.bind();
            //                shader.setUniform1i("uTexture", 2);
            //                image.unbind();
            
        }
        
        if (mode == 0)
        {
            ofSetColor(255);
            shader.setUniform4f("u_colorA", ofFloatColor(color_shaderColorA->r/255.0,color_shaderColorA->g/255.0,color_shaderColorA->b/255.0,color_shaderColorA->a/255.0));
            shader.setUniform4f("u_colorB", ofFloatColor(color_shaderColorB->r/255.0,color_shaderColorB->g/255.0,color_shaderColorB->b/255.0,color_shaderColorB->a/255.0));
            
            if(toggle_useTBOMatrix) shader.setUniform1i("u_useMatrix", 1);
            else shader.setUniform1i("u_useMatrix", 0);
            
            
            switch(dropdown_whichSource)
            {
                case HEX_SOURCE_TEXTURE :
                {
                    vboTex.bind();
                    int numVertexsPerOneFace = 18;
                    vboTex.drawElements(GL_TRIANGLES,hexagonCanvas.getNumHexagons()*numVertexsPerOneFace );
                    vboTex.unbind();
                    break;
                }
                case HEX_SOURCE_QUADS :
                {
                    //vboQuads.bind();
                    
                    vboQuads.drawElements(GL_TRIANGLES,vecVboQuads_Indexs.size());
                    
                    //vboQuads.unbind();
                    break;
                }
                case HEX_SOURCE_CUCS :
                {
                    vboCucs.drawElements(GL_TRIANGLES, vecVboCucs_Faces.size());
                    
                    break;
                }
                case HEX_SOURCE_RANDOM :
                {
                    int numFacesPerOneHexagon = 18;
                    vboRandom.drawElements(GL_TRIANGLES,hexagonCanvas.getNumHexagons()*numFacesPerOneHexagon );
                    break;
                }
                case HEX_SOURCE_GROW :
                {
                    int numVertexsPerOneHexagon = 14;
                    vboGrow.drawElements(GL_TRIANGLES, vecVboGrow_Faces.size());
                    break;
                }
                
            }
            
            
        }
        else
        {
            if(useShader) shader.end();
            ofSetColor(255,255,0);
            vboQuads.draw(GL_QUADS,0,vecVboQuads_Verts.size());
        }
        
        // ... END SHADING
        if(useShader) shader.end();
        
        if(toggle_showVertices)
        {
            vector<ofVec3f> v;
            int elementSize;
            
            if(dropdown_whichSource==HEX_SOURCE_TEXTURE)
            {
                // DRAW VERTEX COORDINATES
                ofSetColor(255,0,0);
                //v = pmVbo1.getCurrentVertices();
                v = vecVboTex_Verts;
                elementSize = 7;
            }
            else  if (dropdown_whichSource==HEX_SOURCE_QUADS)
            {
                // DRAW VERTEX COORDINATES
                ofSetColor(255,0,0);
                v = vecVboQuads_Verts;
                elementSize = 4;
            }
            else  if (dropdown_whichSource==HEX_SOURCE_CUCS)
            {
                // DRAW VERTEX COORDINATES
                ofSetColor(255,0,0);
                v = vecVboCucs_Verts;
                elementSize = vecTempVbo_Verts.size();
                
            }
            
            int whichHexagon = 0;
            for(int i=0;i<v.size();i++)
            {
                whichHexagon = float(i)/float(elementSize);
                if(toggle_useTBOMatrix)
                {
                    ofDrawBitmapString(ofToString(i)+ " : " +ofToString(v[i])   ,v[i].x + hexagonCanvas.getCentroidData()[whichHexagon].x, v[i].y + hexagonCanvas.getCentroidData()[whichHexagon].y) ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
                }
                else
                {
                    ofDrawBitmapString(ofToString(i)+ " : " +ofToString(v[i])   ,v[i].x /*+ hexagonCanvas.getCentroidData()[whichHexagon].x*/, v[i].y /*+ hexagonCanvas.getCentroidData()[whichHexagon].y*/) ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
                }
                
            }
            
            
            //            // DRAW VERTEX COORDINATES
            //            ofSetColor(255,0,0);
            //            vector<ofVec3f> v;
            //            if(dropdown_whichSource=HEX_SOURCE_TEXTURE)
            //            {
            //                v = pmVbo1.getCurrentVertices();
            //            }
            //            else  if (dropdown_whichSource=HEX_SOURCE_QUADS)
            //            {
            //                v = pmVboRibbon.getCurrentVertices();
            //            }
            //            int whichHexagon = 0;
            //            for(int i=0;i<v.size();i++)
            //            {
            //                cout << "v size " << v.size() << endl;
            //                whichHexagon = float(i)/7.0;
            //                if(true)
            //                {
            //                    if(dropdown_whichSource=HEX_SOURCE_TEXTURE)
            //                    {
            //                        ofDrawBitmapString(ofToString(i)  ,v[i].x + hexagonCanvas.getCentroidData()[whichHexagon].x, v[i].y + hexagonCanvas.getCentroidData()[whichHexagon].y) ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
            //                    }
            //                    else  if (dropdown_whichSource=HEX_SOURCE_QUADS)
            //                    {
            //                        ofDrawBitmapString(ofToString(i)  ,v[i].x , v[i].y)  ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
            //                    }
            //                }
            //            }
        }
    }
    
    /// DRAW THE MASK
    if(toggle_showLayout)
    {
        ofSetColor(128,128,128,128);
        maskWireframe.draw(0,0,1200,1200);
    }
    if(toggle_drawMask)
    {
        ofSetColor(255);
        mask.draw(0,0,1200,1200);
    }
    
    /// DRAW VIDEO FILE INFO
    if(isRecording) ofSetColor(255,0,0);
    else ofSetColor(128);
    
    ofVec2f stringPosition = ofVec2f(530,1100);
    
    if(dropdown_whichSource==HEX_SOURCE_TEXTURE)
    {
        switch (dropdown_whichTextureSource)
        {
            case HEX_TEXTURE_IMAGE:
            ofDrawBitmapString(imageFilename + " : " + ofToString(videoPlayer.getCurrentFrame()),stringPosition);
            ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
            break;
            case HEX_TEXTURE_VIDEO:
            if(videoPlayer.isLoaded())
            {
                ofDrawBitmapString(videoFilename + " // Current Frame :  " + ofToString(videoPlayer.getCurrentFrame()),stringPosition);
            }
            ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
            break;
            case HEX_TEXTURE_SYPHON:
            ofDrawBitmapString("Syhpon",stringPosition);
            ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
            break;
            case HEX_TEXTURE_SYPHON_MAX:
            ofDrawBitmapString("Syhpon Max",stringPosition);
            ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
            break;
            
            default:
            break;
        }
    }
    else if(dropdown_whichSource==HEX_SOURCE_CUCS)
    {
        ofDrawBitmapString("CUCS" ,stringPosition);
        ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
    }
    else if(dropdown_whichSource==HEX_SOURCE_RANDOM)
    {
        ofDrawBitmapString("RANDOM" ,stringPosition);
        ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
    }
    else if(dropdown_whichSource==HEX_SOURCE_GROW)
    {
        ofDrawBitmapString("GROW" ,stringPosition);
        ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), stringPosition + ofVec2f(0,15));
    }
    
    if(isRecording)
    {
        ofDrawBitmapString("REC : Recorded Frame : " + ofToString(recordedFrame) + " // Time : " +ofToString(ofGetElapsedTimef()), stringPosition + ofVec2f(0,30) );
    }
    
    // DRAW CENTER AND CIRCLE RADIUS
    //    ofSetColor(255);
    //    ofDrawLine(0,600,1200,600);
    //    ofDrawLine(600,0,600,1200);
    //    ofNoFill();
    //    ofSetCircleResolution(128);
    //    ofDrawCircle(600,600,ofGetMouseX());
    //    ofFill();
    
    
    // DRAW CIRCLE RADIUS
    //    ofNoFill();
    //    ofSetColor(127);
    //    ofSetCircleResolution(128);
    //    for(int i=0;i<hexagonCanvas.ringsRadius.size();i++)
    //    {
    //      ofDrawCircle(600,600,hexagonCanvas.ringsRadius[i]);
    //    }
    //    ofFill();
    
    
    /// END FBO !!
    fboOut.end();
    
    ///
    // RECORDING !!
    ///
    
    
    //    if(isRecording)
    //    { // FRAME NEW ?????????
    //        ofPixels pixels;
    //        fboOut.readToPixels(pixels);
    //        bool success = videoRecorder.addFrame(pixels);
    //        if (!success) {
    //            ofLogWarning("This frame was not added!");
    //        }
    //    }
    //
    //    // Check if the video recorder encountered any error while writing video frame or audio smaples.
    //    if (videoRecorder.hasVideoError()) {
    //        ofLogWarning("The video recorder failed to write some frames!");
    //    }
    //
    //    if (videoRecorder.hasAudioError()) {
    //        ofLogWarning("The video recorder failed to write some audio samples!");
    //    }
    ///
    ///
    /// END RECORDING !!
    
    
    
    if(isRecording && (dropdown_whichTextureSource == HEX_TEXTURE_VIDEO))
    {
        //currentFolderName
        
        if(recordedFrame>videoPlayer.getTotalNumFrames()-1)
        {
            // stop the recording
            isRecording = false;
            recordedFrame = 0;
            videoPlayer.play();
            videoPlayer.setPaused(false);
            
        }
        else
        {
            cout << "Rendering video : Length " << videoPlayer.getTotalNumFrames() << " // Rec.Frames = " << recordedFrame << endl;
            
            ofImage currentFrame;
            ofPixels pixels;
            fboOut.readToPixels(pixels);
            currentFrame.allocate(fboResolution.x,fboResolution.y, OF_IMAGE_COLOR);
            currentFrame.setFromPixels(pixels);
            currentFrame.save(currentFolderName + "/" +ofToString(recordedFrame) +".png" );
            
            videoPlayer.nextFrame();
            
            
        }
        
        recordedFrame = recordedFrame + 1;
        
    }
    
    
    
    
    /// DRAW FBO TO SCREEN
    ofPushMatrix();
    ofSetColor(255,255,255);
    if(ofGetWidth()<1200.0)
    {
        fboOut.draw(0,0,ofGetWidth(),ofGetWidth());
    }
    else
    {
        fboOut.draw(0,0,ofGetHeight(),ofGetHeight());
    }
    ofPopMatrix();
    
    // draw texture previews
    //    ofSetColor(255);
    //    image.draw(0,ofGetHeight()-100,200,200);
    //    syphon.draw(200,ofGetHeight()-100,200,200);
    
    
    /// SAVE IMAGE
    if(saveNow)
    {
        saveNow = false;
        ofPixels pixels;
        fboOut.readToPixels(pixels);
        imageToSave.allocate(fboResolution.x,fboResolution.y, OF_IMAGE_COLOR);
        imageToSave.setFromPixels(pixels);
        imageToSave.save("./captures/" + ofGetTimestampString() +".png" );
    }
    
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key==' ')
    {
        mode=(mode+1)%numModes;
    }
    else if(key=='s')
    {
        saveNow = true;
    }
    else if(key=='p')
    {
        if(videoPlayer.isPaused()) videoPlayer.setPaused(false);
        else videoPlayer.setPaused(true);
        //videoPlayer.stop();
        cout << " pausing " << endl;
    }
    else if(key=='-')
    {
        videoPlayer.nextFrame();
    }
    else if(key=='.')
    {
        videoPlayer.previousFrame();
    }
    else if(key=='h')
    {
        useShader = !useShader;
    }
    else if(key=='r')
    {
        toggle_useTBOMatrix = !toggle_useTBOMatrix;
    }
    else if(key=='q')
    {
        isRecording=!isRecording;
        
        if(isRecording)
        {
            if(dropdown_whichTextureSource == HEX_TEXTURE_VIDEO) // video
            {
                // extract video filename
                string videoName;
                vector<string> vfs = ofSplitString(videoFilename, "/");
                videoName = vfs[vfs.size()-1];
                
                // create folder
                string path = "./videoCaptures/" + videoName + "_" +ofToString(ofGetTimestampString());
                currentFolderName = path;
                ofDirectory dir(path);
                if(!dir.exists())
                {
                    dir.create(true);
                    //videoPlayer.stop();
                    videoPlayer.setPosition(0.0);
                    videoPlayer.setPaused(true);
                    
                    recordedFrame = 0;
                    
                    cout << "STARTING NEW RENDERING : Video Pos ." << videoPlayer.getPosition() << endl;
                }
            }
            else
            {
                recordedFrame = 0;
                capture.startRecording("capture.mov", 60.0);
            }
        }
        else  // NOT RECORDING
        {
            if(dropdown_whichTextureSource != HEX_TEXTURE_VIDEO)
            {
                capture.stopRecording();
            }
            else if (dropdown_whichTextureSource == HEX_TEXTURE_VIDEO)
            {
                videoPlayer.play();
            }
        }
        //        if(!isRecording)
        //        {
        //            cout << ">>>>> STOP RECORDING" << endl;
        //            videoRecorder.close();
        //        }
        //        else
        //        {
        //            cout << ">>>> START RECORDING !!!!! " << endl;
        //            videoRecorder.setup("./recordings/" +ofGetTimestampString()+fileExt, fboOut.getWidth(), fboOut.getHeight(), desiredFramerate, 0, 0);
        //            videoRecorder.start();
        //        }
    }
    
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}


//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo info)
{
    
    if( info.files.size() > 0 )
    {
        vector<string> dragFileName = ofSplitString(info.files[0], "/");
        string dragFileExtension = ofSplitString(dragFileName[dragFileName.size()-1],".")[1];
        cout << ">> Dragged File ... \n" << info.files[0] << " : ext: " << dragFileExtension << endl;
        if(dragFileExtension=="png" || dragFileExtension == "jpg")
        {
            dropdown_whichSource = HEX_SOURCE_TEXTURE;
            dropdown_whichTextureSource = HEX_TEXTURE_IMAGE;
            cout << "Loading new texture ... " << endl;
            imageFilename = info.files[0];
            image.load(imageFilename);
            image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            //pmVbo1.setTextureReference(image.getTexture());
            
            if(videoPlayer.isLoaded()) videoPlayer.stop();
            
        }
        else if (dragFileExtension=="mov")
        {
            dropdown_whichSource = HEX_SOURCE_TEXTURE;
            dropdown_whichTextureSource = HEX_TEXTURE_VIDEO;
            
            cout << "Loading new video ... " << "./testMedia/" + ofSplitString(dragFileName[dragFileName.size()-1],".")[0] +"." +dragFileExtension <<endl;
            videoFilename=ofToString("./testMedia/" + ofSplitString(dragFileName[dragFileName.size()-1],".")[0] +"." +dragFileExtension);
            videoPlayer.load(videoFilename);
            videoPlayer.setLoopState(OF_LOOP_NORMAL);
            videoPlayer.play();
            while(!videoPlayer.isLoaded())
            {
                cout << " ... loading video " << endl;
            }
            videoPlayer.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            //pmVbo1.setTextureReference(videoPlayer.getTexture());
        }
    }
    
}


//--------------------------------------------------------------
bool ofApp::sortPointsOnDistanceToOrigin(ofPoint &p1, ofPoint &p2)
{
    ofPoint origin = ofPoint(600,600);
    float dist1,dist2;
    dist1  = origin.distance(p1);
    dist2  = origin.distance(p2);
    
    return dist2>dist1;
}



//--------------------------------------------------------------
ofPoint ofApp::projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd)
{
    float LineMag;
    float U;
    ofPoint Intersection;
    
    LineMag = LineEnd.distance(LineStart);
    
    U = ( ( ( Point.x - LineStart.y ) * ( LineEnd.x - LineStart.x ) ) +
         ( ( Point.y - LineStart.y ) * ( LineEnd.y - LineStart.y ) ) +
         ( ( Point.z - LineStart.z ) * ( LineEnd.z - LineStart.z ) ) ) /
    ( LineMag * LineMag );
    
    if( U < 0.0f || U > 1.0f )
    {
        return ofVec3f(1000000,1000000);//cout << " closest point does not fall within the line segment !! " << endl;
        //cout << " closest point does not fall within the line segment !! " << endl;
    }
    
    Intersection.x = LineStart.x + U * ( LineEnd.x - LineStart.x );
    Intersection.y = LineStart.y + U * ( LineEnd.y - LineStart.y );
    Intersection.z = LineStart.z + U * ( LineEnd.z - LineStart.z );
    
    //    *Distance = Magnitude( Point, &Intersection );
    
    
    return Intersection;//.length();
}
//--------------------------------------------------------------
void ofApp::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << endl;
}

//--------------------------------------------------------------
void ofApp::exit(){
    ofRemoveListener(videoRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    videoRecorder.close();
}

//--------------------------------------------------------------
void ofApp::resetVecOscVector()
{
    vecOsc.resize(64);
    for(int i=0;i<vecOsc.size();i++)
    {
        vecOsc[i].resize(35);
        for(int j=0;j<35;j++)
        {
            vecOsc[i][j] = 1.0;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::changedTexCoord(int &i)
{
    //    pmVbo1.setTexCoordsIndex(i);
    vboTex.setTexCoordData(vecVboTex_TexCoords[i].data(),vecVboTex_TexCoords[i].size() , GL_DYNAMIC_DRAW);
}
//--------------------------------------------------------------
void ofApp::changedTexSource(int &i)
{
    if(i==HEX_TEXTURE_VIDEO) videoPlayer.play();
    
    shader.begin();
    shader.setUniform1i("u_textureSource", dropdown_whichTextureSource);
    shader.end();
    
    cout <<"Changed Texture source to " << dropdown_whichTextureSource <<endl;
}

//--------------------------------------------------------------
void ofApp::changedSource(int &i)
{
    int source;
    
    switch (i) {
        case HEX_SOURCE_TEXTURE:
        source = 0;
        break;
        case HEX_SOURCE_QUADS:
        source = 1;
        break;
        case HEX_SOURCE_CUCS:
        source = 2;
        break;
        case HEX_SOURCE_RANDOM:
        source = 3;
        break;
        case HEX_SOURCE_GROW:
        source = 4;
        break;
        
        default:
        break;
    }
    shader.begin();
    shader.setUniform1i("u_source", source);
    shader.end();
    
    cout <<"Changed source to " << dropdown_whichSource<<endl;
}

//--------------------------------------------------------------
void ofApp::calculateStartEndPointsAndCurve()
{
    // START AND END POINTS
    startPoint = (hexaSides[startSide]/2.0)+hexaPoints[startSide];
    endPoint = (hexaSides[endSide]/2.0)+hexaPoints[endSide];
    
    // BEZIER CURVE BASED ON START AND END POINTS
    // try to calculate the bezier control points automatically ...
    ofVec2f controlStart = ofVec2f(startPoint.x + hexaSides[startSide].y/4, startPoint.y -hexaSides[startSide].x/4 );
    ofVec2f controlEnd = ofVec2f(endPoint.x + hexaSides[endSide].y/4   , endPoint.y - hexaSides[endSide].x/4 );
    
    bezierLine.clear();
    bezierLine.addVertex(startPoint);
    bezierLine.bezierTo(controlStart.x, controlStart.y, controlEnd.x, controlEnd.y, endPoint.x, endPoint.y);
    
    
    
    // calculate sampled line
    for(int i=0;i<numSteps;i++)
    {
        sampledPoints[i] = bezierLine.getPointAtPercent((1.0/float(numSteps-1))*i);
    }
    
    
}

//--------------------------------------------------------------
void ofApp::calculateRibs()
{
    
    widthStart = hexaSides[startSide].length();
    widthEnd = hexaSides[endSide].length();
    
    ofSetColor(0,255,255);
    for(int i=0;i<numSteps;i++)
    {
        if(i==0)
        {
            // first rib
            ofVec3f v = startPoint + (hexaSides[startSide] * cucWidth);
            ofVec3f w = startPoint - (hexaSides[startSide] * cucWidth);
            ribs[i][0] = v;
            ribs[i][1] = w;
            
            //            cout << "first rib dist = " << ofVec3f(w-v).length() << endl;
        }
        else if(i==numSteps-1)
        {
            // last rib
            ofVec3f v = endPoint + (hexaSides[endSide] * cucWidth);
            ofVec3f w = endPoint - (hexaSides[endSide] * cucWidth);
            ribs[i][0] = w;
            ribs[i][1] = v;
            
            //            cout << "last rib dist = " << ofVec3f(w-v).length()<< endl;
            
        }
        else
        {
            // middle ribs
            ofVec3f vS = startPoint + (hexaSides[startSide] * cucWidth);
            ofVec3f wS = startPoint - (hexaSides[startSide] * cucWidth);
            ofVec3f vE = endPoint + (hexaSides[endSide] * cucWidth);
            ofVec3f wE = endPoint - (hexaSides[endSide] * cucWidth);
            
            
            //            cout << "start rib dist = " << ofVec3f(wS-vS).length() << endl;
            //            cout << "end rib dist = " << ofVec3f(wE-vE).length()<< endl;
            
            float middleWidth = ofMap(i,0,numSteps-1,ofVec3f(wS-vS).length(),ofVec3f(wE-vE).length());
            middleWidth = middleWidth ;
            
            //            cout << "middle width = " << middleWidth << endl;
            
            //            // middle ribs
            //            float middleWidth = ofMap(i,0,numSteps-1,widthStart,widthEnd);
            
            ofVec3f vMiddle = sampledPoints[i-1]-sampledPoints[i+1];
            vMiddle = ofVec3f(vMiddle.y,-vMiddle.x,0);
            ofVec3f v = sampledPoints[i] + (vMiddle.normalize() * middleWidth)/2;
            ofVec3f w = sampledPoints[i] - (vMiddle.normalize() * middleWidth)/2;
            
            ribs[i][0] = v;
            ribs[i][1] = w;
            
        }
        //ofDrawLine(ribs[i][0],ribs[i][1]);
        //        cout << i << " _ Drawing from : " << ribs[i][0] << " to : " << ribs[i][1] << endl;
    }
    
}

//--------------------------------------------------------------
void ofApp::calculateVboData()
{
    
    // DEFINE VBO DATA
    
    // VERTEXS
    for(int i=0;i<vecTempVbo_Verts.size();i++)
    {
        int half = (vecTempVbo_Verts.size()/2)-1;
        if(i<=half)
        {
            vecTempVbo_Verts[i] = ribs[i][0];
        }
        else
        {
            vecTempVbo_Verts[i] = ribs[(numSteps-1)-(i-numSteps)][1];
        }
    }
    // FACES
    int totalVertexs = numSteps*2;
    for(int i=0;i<(numSteps-1);i++)
    {
        vecTempVbo_Faces[(i*6)+0] = i + lastFaceAddedToCucs;
        vecTempVbo_Faces[(i*6)+1] = i+1 + lastFaceAddedToCucs;
        vecTempVbo_Faces[(i*6)+2] = (totalVertexs-1)-i + lastFaceAddedToCucs;
        
        vecTempVbo_Faces[(i*6)+3] = i+1 + lastFaceAddedToCucs;
        vecTempVbo_Faces[(i*6)+4] = (totalVertexs-1)-i-1 + lastFaceAddedToCucs;
        vecTempVbo_Faces[(i*6)+5] = (totalVertexs-1)-i + lastFaceAddedToCucs;
        
    }
    lastFaceAddedToCucs=lastFaceAddedToCucs + ((numSteps)*2);
    
    
    //    for(int i=0;i<vecTempVbo_Faces.size();i++)
    //    {
    //        //                cout << "Face : " << i << " :: " << vecVboCucs_faces[(i*3)] << " , " << vecVboCucs_faces[(i*3)+1] << " , " << vecVboCucs_faces[(i*3)+2] << endl;
    //        //        cout << "FaceTemp " << i << ": " << vecTempVbo_Faces[i] << endl;
    //    }
    
    // COLORS and TEXCOORDS
    float factor;
    for(int i=0;i<vecTempVbo_Verts.size();i++)
    {
        factor = ofMap(i,0,numSteps*2,0.0,1.0);
        //            if(i<8)
        //            {
        //                vecVboCucs_colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
        //            }
        //            else
        //            {
        //                vecVboCucs_colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
        //            }
        
        //        vecTempVbo_Colors[i] = ofFloatColor(1.0,1.0*factor,0.0,1.0);
        if(int(i/numSteps)%2)
        {
            vecTempVbo_Colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
        }
        else
        {
            vecTempVbo_Colors[i] = ofFloatColor(0.0,0.0,0.0,1.0);
        }
        vecTempVbo_TexCoords[i] = ofVec2f(vecTempVbo_Verts[i].x,vecTempVbo_Verts[i].y);
    }
    
}

//--------------------------------------------------------------
void ofApp::calculateSides()
{
    
    // HEXAGON SIDES VECTORS represent the vectors of each "side". From V01, V12, ... to V50 total = 6
    for(int i=0;i<6;i++)
    {
        if(i!=5)
        {
            // from 0 to 4th vertex it's i+1 - i
            hexaSides[i] = hexaPoints[i+1] - hexaPoints[i];
        }
        else if(i==5)
        {
            // last vertex, special case 5 - 0
            hexaSides[i] = hexaPoints[0] - hexaPoints[i];
        }
    }
}

//--------------------------------------------------------------
void ofApp::calculateTilePatterns()
{
    
    // 1 CONNECTION JUMP 1
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 1 +1
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 1 +2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 1 +3
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 1
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 1'
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 1''
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 1'''
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+4)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 4 CONNECTION JUMP 1
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 4 CONNECTION JUMP 1'
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        t.addConnection((i+4)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 4 CONNECTION JUMP 1''
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        t.addConnection((i+4)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 5 CONNECTION JUMP 1
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        t.addConnection((i+4)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 6 CONNECTION JUMP 1'
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+4)%6);
        t.addConnection((i+4)%6, (i+5)%6);
        t.addConnection((i+5)%6, (i+6)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 1 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 2'
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+3)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 2''
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+1)%6, (i+3)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+2)%6, (i+4)%6);
        t.addConnection((i+4)%6, (i+6)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+3)%6, (i+5)%6);
        t.addConnection((i+5)%6, (i+7)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+1)%6, (i+3)%6);
        t.addConnection((i+3)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 2
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+1)%6, (i+3)%6);
        t.addConnection((i+2)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 1 CONNECTION JUMP 3
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+3)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 2 CONNECTION JUMP 3
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+3)%6);
        t.addConnection((i+1)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // 3 CONNECTION JUMP 3
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+3)%6);
        t.addConnection((i+1)%6, (i+4)%6);
        t.addConnection((i+2)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    
    // COMBOS
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i)%6, (i+2)%6);
        t.addConnection((i)%6, (i+3)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // COMBOS '
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection(i%6, (i+2)%6);
        t.addConnection((i+1)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // COMBOS '
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+1)%6, (i+2)%6);
        t.addConnection((i+1)%6, (i+4)%6);
        hexagonTilesDictionary.push_back(t);
    }
    // COMBOS '
    for(int i=0;i<6;i++)
    {
        pmHexagonTile t;
        t.addConnection(i%6, (i+1)%6);
        t.addConnection((i+0)%6, (i+1)%6);
        t.addConnection((i+0)%6, (i+2)%6);
        t.addConnection((i+0)%6, (i+3)%6);
        t.addConnection((i+0)%6, (i+4)%6);
        t.addConnection((i+0)%6, (i+5)%6);
        hexagonTilesDictionary.push_back(t);
    }
    
    // DR KING HEXAGONAL STUDY
    // 0,0
    {
        pmHexagonTile t;
        t.addConnection(0,2);
        t.addConnection(1,3);
        t.addConnection(4,5);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,1
    {
        pmHexagonTile t;
        t.addConnection(0,2);
        t.addConnection(5,1);
        t.addConnection(3,4);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,2
    {
        pmHexagonTile t;
        t.addConnection(5,1);
        t.addConnection(0,4);
        t.addConnection(2,3);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,3
    {
        pmHexagonTile t;
        t.addConnection(3,5);
        t.addConnection(0,4);
        t.addConnection(1,2);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,4
    {
        pmHexagonTile t;
        t.addConnection(2,4);
        t.addConnection(3,5);
        t.addConnection(0,1);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,5
    {
        pmHexagonTile t;
        t.addConnection(1,3);
        t.addConnection(2,4);
        t.addConnection(5,0);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 0,5
    {
        pmHexagonTile t;
        t.addConnection(1,3);
        t.addConnection(2,4);
        t.addConnection(5,0);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 1,0
    {
        pmHexagonTile t;
        t.addConnection(2,3);
        t.addConnection(4,5);
        t.addConnection(0,1);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 1,1
    {
        pmHexagonTile t;
        t.addConnection(1,2);
        t.addConnection(3,4);
        t.addConnection(5,0);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 2,0
    {
        pmHexagonTile t;
        t.addConnection(2,5);
        t.addConnection(1,3);
        t.addConnection(0,4);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 2,1
    {
        pmHexagonTile t;
        t.addConnection(1,4);
        t.addConnection(0,2);
        t.addConnection(3,5);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 2,2
    {
        pmHexagonTile t;
        t.addConnection(0,3);
        t.addConnection(1,5);
        t.addConnection(2,4);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 3,0
    {
        pmHexagonTile t;
        t.addConnection(2,5);
        t.addConnection(3,4);
        t.addConnection(0,1);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 3,1
    {
        pmHexagonTile t;
        t.addConnection(1,4);
        t.addConnection(2,3);
        t.addConnection(5,0);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 3,2
    {
        pmHexagonTile t;
        t.addConnection(0,3);
        t.addConnection(1,2);
        t.addConnection(4,5);
        
        hexagonTilesDictionary.push_back(t);
    }
    // 4,0
    {
        pmHexagonTile t;
        t.addConnection(2,5);
        t.addConnection(1,4);
        t.addConnection(0,3);
        
        hexagonTilesDictionary.push_back(t);
    }
    // fill 2 void
    {
        pmHexagonTile t;
        
        hexagonTilesDictionary.push_back(t);
    }
    // fill 2 void
    {
        pmHexagonTile t;
        
        hexagonTilesDictionary.push_back(t);
    }
    
    
    // MERGING sample
    hexagonTilesDictionary.push_back(hexagonTilesDictionary[0].mergeTileData(hexagonTilesDictionary[1]));
}

//--------------------------------------------------------------
void ofApp::prepareQuads()
{
    
    int numRibbons = hexagonCanvas.getNumHexagons();
    int numRibbonVertexs = hexagonCanvas.getNumHexagons()*4;
    
    // VERTEXS
    vecVboQuads_Verts.resize(numRibbonVertexs);
    vboQuads.setVertexData(vecVboQuads_Verts.data(),vecVboQuads_Verts.size(),GL_DYNAMIC_DRAW);
    
    // TEXTURE COORDS
    //    vector<ofVec2f> ribbonTexCoords;
    //    ribbonTexCoords.resize(numRibbons*4);
    //    for(int i=0;i<numRibbons;i++)
    //    {
    //        for(int j=0;j<4;j++)
    //        {
    //            ribbonTexCoords[(i*4)+j] = vCentroidPoints[i] / 1200.0 ;
    //        }
    //    }
    //    pmVboRibbon.setTexCoordsData(vecCentroidTexCoord,1);
    
    // COLORS
    vecVboQuads_Colors.resize(numRibbonVertexs);
    for(int i=0;i<vecVboQuads_Colors.size();i++)
    {
        vecVboQuads_Colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
    }
    vboQuads.setColorData(vecVboQuads_Colors.data(), vecVboQuads_Colors.size(), GL_DYNAMIC_DRAW);
    
    // INDEXS
    vecVboQuads_Indexs.resize(numRibbons*6);
    for(int i=0;i<numRibbons;i++)
    {
        vecVboQuads_Indexs[(i*6)+0] = (i*4)+0;
        vecVboQuads_Indexs[(i*6)+1] = (i*4)+1;
        vecVboQuads_Indexs[(i*6)+2] = (i*4)+2;
        vecVboQuads_Indexs[(i*6)+3] = (i*4)+0;
        vecVboQuads_Indexs[(i*6)+4] = (i*4)+2;
        vecVboQuads_Indexs[(i*6)+5] = (i*4)+3;
    }
    vboQuads.setIndexData(vecVboQuads_Indexs.data(), vecVboQuads_Indexs.size(), GL_DYNAMIC_DRAW);
    
    
    
}

//---------------------------------------------------------------------------
void ofApp::buildTBOs()
{
    
    matricesTransform.resize(hexagonCanvas.getNumHexagons());
    // upload the transformation for each box using a
    // texture buffer.
    // for that we need to upload the matrices to the buffer
    // and allocate the texture using it
    bufferTransform.allocate();
    bufferTransform.bind(GL_TEXTURE_BUFFER);
    bufferTransform.setData(matricesTransform,GL_STREAM_DRAW);
    
    // using GL_RGBA32F allows to read each row of each matrix
    // as a float vec4 from the shader.
    // Note that we're allocating the texture as a Buffer Texture:
    // https://www.opengl.org/wiki/Buffer_Texture
    texTransform.allocateAsBufferTexture(bufferTransform,GL_RGBA32F);
    
    
    
    /// CUBIC COLORS TBO STUFF (Texture Buffer Object)
    //////////////////////////////////////////////////
    
    matricesCubeColors.resize(hexagonCanvas.getNumHexagons() * 3);
    // upload the
    // texture buffer.
    // for that we need to upload the matrices to the buffer
    // and allocate the texture using it
    bufferCubeColors.allocate();
    bufferCubeColors.bind(GL_TEXTURE_BUFFER);
    bufferCubeColors.setData(matricesCubeColors,GL_STREAM_DRAW);
    
    // using GL_RGBA32F allows to read each row of each matrix
    // as a float vec4 from the shader.
    // Note that we're allocating the texture as a Buffer Texture:
    // https://www.opengl.org/wiki/Buffer_Texture
    texCubeColors.allocateAsBufferTexture(bufferCubeColors,GL_RGBA32F);
    
}

//---------------------------------------------------------------------------
void ofApp::buildNumElementsTBO()
{
    matricesNumElements.resize(hexagonCanvas.getNumHexagons()*1000);
    cout << "BuildingNumElem.TBO : size of matrices : " << hexagonCanvas.getNumHexagons()*256 << endl;
    // upload the
    // texture buffer.
    // for that we need to upload the matrices to the buffer
    // and allocate the texture using it
    bufferNumElements.allocate();
    bufferNumElements.bind(GL_TEXTURE_BUFFER);
    bufferNumElements.setData(matricesNumElements,GL_STREAM_DRAW);
    
    // using GL_RGBA32F allows to read each row of each matrix
    // as a float vec4 from the shader.
    // Note that we're allocating the texture as a Buffer Texture:
    // https://www.opengl.org/wiki/Buffer_Texture
    texNumElements.allocateAsBufferTexture(bufferNumElements,GL_R32UI);
    
}


//---------------------------------------------------------------------------
void ofApp::prepareHexagons()
{
    // prepare vectors of data for vboTex. * 7 as we'll have 7 vertexs per each hexagon ... center + 6 sides.
    int nVerts = hexagonCanvas.getNumHexagons()*7;
    vecVboTex_Verts.resize(nVerts);
    vecVboTex_Colors.resize(nVerts);
    vecVboTex_Faces.resize(nVerts*3);
    vecVboTex_TexCoords.resize(2);
    for(int i=0;i<vecVboTex_TexCoords.size();i++)
    {
        vecVboTex_TexCoords[i].resize(nVerts);
    }
    
    vecVboTex_Verts = hexagonCanvas.getVertexData();
    
    ////////////////////////////////////////
    /// GENERATE TEXTURE COORDINATES DATA !!
    ////////////////////////////////////////
    
    // vector<ofVec2f> vecTexCoord
    // try to get centroid data to convert it to texCoord of centroids (we need 2 way of texCoord ... based on Rings and based on Centroids)
    // we got 1 centroid per path = 2735 paths
    vector<ofPoint> vCentroidPoints = hexagonCanvas.getCentroidData();
    vector<ofVec2f> vecCentroidTexCoord;
    vecCentroidTexCoord.resize(hexagonCanvas.getNumHexagons()*7);
    
    for(int i=0;i<hexagonCanvas.getNumHexagons();i++)
    {
        for(int j=0;j<7;j++)
        {
            vecCentroidTexCoord[(i*7)+j] = vCentroidPoints[i] / 1200.0 ;
        }
    }
    
    /// STORE DATA INTO VECTORS FOR VBO_TEX
    //////////////////////////////////////////
    
    // index 1 we store the texcoords "normal" ... flat texture mapping
    vecVboTex_TexCoords[1] = vecCentroidTexCoord;
    // index 0 we store the texcoords "ring and id" mode
    vecVboTex_TexCoords[0] = hexagonCanvas.getTextureCoords();
    vecVboTex_Colors = hexagonCanvas.getColorData();
    vecVboTex_Faces = hexagonCanvas.getFaceData();
    
    /// FILL VBO TEX DATA
    //////////////////////
    vboTex.setVertexData(vecVboTex_Verts.data(),vecVboTex_Verts.size(),GL_DYNAMIC_DRAW);
    vboTex.setTexCoordData(vecVboTex_TexCoords[0].data(), vecVboTex_TexCoords[0].size(), GL_DYNAMIC_DRAW);
    vboTex.setColorData(vecVboTex_Colors.data(), vecVboTex_Colors.size(), GL_DYNAMIC_DRAW);
    vboTex.setIndexData(vecVboTex_Faces.data(), vecVboTex_Faces.size(), GL_DYNAMIC_DRAW);
    
}
//---------------------------------------------------------------------------
void ofApp::prepareCucs()
{
    
    vecVboCucs_Verts.clear();
    vecVboCucs_Faces.clear();
    vecVboCucs_Colors.clear();
    vecVboCucs_TexCoords.clear();
    
    // inits
    startSide = 0;
    endSide = 1;
    numSteps = 8;
    cucWidth = 0.125;
    lastFaceAddedToCucs=0;
    
    // TILES
    calculateTilePatterns();
    
    // we put the vbo data of each hexagon cuc into a temp vector that will be inserted(appended) on the vecVboCucs_Verts
    // resize temp vectors that are recalculated for each hexaogn cuc
    int numMaxOfCucsInOneHexagon = 6;
    
    
    
    for(int i=0;i<hexagonCanvas.getNumHexagons();i++)
    {
        // for each hexagon ....
        // clear and resizing data
        hexaPoints.clear();
        hexaSides.clear();
        hexaPoints.resize(6,ofVec3f(0,0,0));
        hexaSides.resize(6);
        
        // define HEXAGON POINTS
        // add them to the hexagon
        for(int j=0;j<6;j++)
        {
            hexaPoints[j] = vecVboTex_Verts[(i*7)+j+1]; //+1 because vecVbo[0s] are centroids?
        }
        // CALCULATE SIDES (SAME FOR EVERY HEXAGON)
        calculateSides();
        ofVec2f idRing = hexagonCanvas.getHexagonIdAndRing(i);
        
        int choosedPattern = int(ofRandom(168,183));
        
        //        if(int(idRing.y)%3==0)
        //        {
        //            if(int(idRing.x)%2==0)
        //            {
        //                choosedPattern = 134;
        //            }
        //            else if(int(idRing.x)%2==1)
        //            {
        //                choosedPattern = 126;
        //            }
        //        }
        //        else
        //        {
        //            choosedPattern = 90;
        //        }
        
        pmHexagonTile actualTilePattern = hexagonTilesDictionary[choosedPattern];
        int numCucsActualHexagon = actualTilePattern.getConnections().size(); //int(ofRandom(0, 6));
        //cout << "Get number of cucs in pattern : " << choosedPattern << " :: " << actualTilePattern.getConnections().size() << endl;
        
        int maxCucsInOneHexagon = 6;
        for(int k=0;k<maxCucsInOneHexagon;k++)
        {
            vecTempVbo_Verts.resize(numSteps*2,ofVec3f(0,0,0));
            vecTempVbo_Colors.resize(vecTempVbo_Verts.size()),ofFloatColor(0.0,1.0,1.0);
            vecTempVbo_TexCoords.resize(vecTempVbo_Verts.size(),ofVec2f(0,0));
            vecTempVbo_Faces.resize((numSteps-1)*2*3,0);
            
            if(k<numCucsActualHexagon)
            {
                // this is a user defined cuc ... take it normally
                sampledPoints.clear();
                ribs.clear();
                sampledPoints.resize(numSteps);
                ribs.resize(numSteps);
                for(int i=0;i<numSteps;i++)
                {
                    ribs[i].resize(2);
                }
                
                startSide = actualTilePattern.getConnections()[k].startsAt;
                endSide = actualTilePattern.getConnections()[k].endsAt;
                
                // STEPS
                calculateStartEndPointsAndCurve();
//                toSVGPath.newSubPath();
//                toSVGPath.moveTo(bezierLine.getVertices()[0]);
//                for(int jj= 1; jj < bezierLine.getVertices().size() ; jj++)
//                    toSVGPath.lineTo(bezierLine.getVertices()[jj]);
                calculateRibs();
                calculateVboData();
                
                
                
            }
            else
            {
                // fill with "void" cucs
                lastFaceAddedToCucs=lastFaceAddedToCucs + ((numSteps)*2);
                
            }
            // inserting calculated hexagon cuc into vectors that will feed the vboCucs
            vecVboCucs_Verts.insert(vecVboCucs_Verts.end(), vecTempVbo_Verts.begin(), vecTempVbo_Verts.end());
            vecVboCucs_Faces.insert(vecVboCucs_Faces.end(), vecTempVbo_Faces.begin(), vecTempVbo_Faces.end());
            vecVboCucs_Colors.insert(vecVboCucs_Colors.end(), vecTempVbo_Colors.begin(), vecTempVbo_Colors.end());
            vecVboCucs_TexCoords.insert(vecVboCucs_TexCoords.end(), vecTempVbo_TexCoords.begin(), vecTempVbo_TexCoords.end());
            
        }
        
        
    }
    
    //save svg;
//    svgSaver.addPath(toSVGPath);
//    svgSaver.save("HolaTest.svg");
    
    // FILL DATA INTO VBO CUCS
    vboCucs.setVertexData(vecVboCucs_Verts.data(), vecVboCucs_Verts.size(),GL_DYNAMIC_DRAW);
    vboCucs.setIndexData(vecVboCucs_Faces.data(), vecVboCucs_Faces.size(),GL_DYNAMIC_DRAW);
    vboCucs.setColorData(vecVboCucs_Colors.data(), vecVboCucs_Colors.size(),GL_DYNAMIC_DRAW);
    vboCucs.setTexCoordData(vecVboCucs_TexCoords.data(), vecVboCucs_TexCoords.size(),GL_DYNAMIC_DRAW);
    
    cout << "vboCucs sizes _ Verts = " << vecVboCucs_Verts.size() << " Faces = " << vecVboCucs_Faces.size() << endl;
    
    
}



//---------------------------------------------------------------------------
void ofApp::prepareRandom()
{
    vecVboRandom_Verts.clear();
    vecVboRandom_Faces.clear();
    
    // in Random mode we use the same ammount of data ... we just change the colors to hide/show hexagons
    vecVboRandom_Verts.resize(vecVboTex_Verts.size());
    vecVboRandom_Faces.resize(vecVboTex_Faces.size());
    vecVboRandom_Colors.resize(vecVboTex_Colors.size(),ofFloatColor(0.0,0.0,0.0,1.0));
    
    // initialize arrays of verts and indeces
    vecVboRandom_Verts = vecVboTex_Verts;
    vecVboRandom_Faces = vecVboTex_Faces;
    
    // put some triangles to white
    for(int i=0;i<slider_howManyRandomHexagons;i++)
    {
        int whichHexagonToShow = int(ofRandom(0,hexagonCanvas.getNumHexagons()));
        int whereIsIt = (whichHexagonToShow*7);
        for(int j=0;j<7;j++)
        {
            vecVboRandom_Colors[whereIsIt + j] = ofFloatColor(1.0,0.0,0.0,1.0);
        }
    }
    vboRandom.setVertexData(vecVboTex_Verts.data(), vecVboTex_Verts.size(), GL_DYNAMIC_DRAW);
    vboRandom.setIndexData(vecVboTex_Faces.data(), vecVboTex_Faces.size(), GL_DYNAMIC_DRAW);
    vboRandom.setColorData(vecVboRandom_Colors.data(),vecVboRandom_Colors.size(),GL_DYNAMIC_DRAW);
    
    //pmHexagonTileCanvas::searchForStartingOn(hexagonTilesDictionary, 0);
}


//---------------------------------------------------------------------------
void ofApp::updateRandom()
{
    // decrease all vertices colors
    float decrease=0.75;
    for(int i=0;i<vecVboRandom_Verts.size();i++)
    {
        vecVboRandom_Colors[i] = vecVboRandom_Colors[i] - slider_decreaseRate; //*((1-(vecVboRandom_Colors[i].r))*(1-(vecVboRandom_Colors[i].r)));
        //        if(vecVboRandom_Colors[i].r!=0.0) cout << slider_decreaseRate*((1-(vecVboRandom_Colors[i].r))*(1-(vecVboRandom_Colors[i].r))) << endl;
        
    }
    
    if(ofGetElapsedTimeMillis()-lastRandomTime >=slider_randomPeriod)
    {
        // put some triangles to white
        for(int i=0;i<slider_howManyRandomHexagons;i++)
        {
            int whichHexagonToShow = int(ofRandom(0,hexagonCanvas.getNumHexagons()));
            //ofVec2f idRing = hexagonCanvas.getHexagonIdAndRing(whichHexagonToShow);
            //if(int(idRing.x)%4==0)//
            {
                int whereIsIt = (whichHexagonToShow*7);
                for(int j=0;j<7;j++)
                {
                    vecVboRandom_Colors[whereIsIt + j] = vecVboRandom_Colors[whereIsIt + j] + ofFloatColor(color_shaderColorA->r/255.0,color_shaderColorA->g/255.0,color_shaderColorA->b/255.0,color_shaderColorA->a/255.0);
                }
                lastRandomTime = ofGetElapsedTimeMillis();
            }
        }
    }
    
    vboRandom.updateColorData(vecVboRandom_Colors.data(),vecVboRandom_Colors.size());
    
    
    
}

//---------------------------------------------------------------------------
void ofApp::prepareGrow()
{
    
    vecVboGrow_Verts.clear();
    vecVboGrow_Faces.clear();
    vecVboGrow_Colors.clear();
    vecVboGrow_HexagonId.clear();
    
    // inits
    int numCucs = hexagonCanvas.getNumHexagons();
    startSide = 0;
    endSide = 1;
    numSteps = 8;
    cucWidth = 0.125;
    lastFaceAddedToGrow=0;
    
    
    // TODO: already done ?
    // TILES
    //calculateTilePatterns();
    
    // we put the vbo data of each hexagon cuc into a temp vector that will be inserted(appended) on the vecVboGrow_Verts
    // resize temp vectors that are recalculated for each hexaogn cuc
    int numMaxOfCucsInOneHexagon = 6;

    for(int j=0;j<growingHexagons.size();j++)
    {
        int i = growingHexagons[j]._num;
        ofVec2f vIdRing = ofVec2f(growingHexagons[j]._id,growingHexagons[j]._ring);
        int startingAt = growingHexagons[j]._startingAtSide;
        int endingAt = growingHexagons[j]._endingAtSide;
    
        // for each hexagon if it's used on the grow path ....
        // clear and resizing data
        hexaPoints.clear();
        hexaSides.clear();
        
        hexaPoints.resize(6,ofVec3f(0,0,0));
        hexaSides.resize(6);
        
        // define HEXAGON POINTS
        // add them to the hexagon
        for(int m=0;m<6;m++)
        {
            hexaPoints[m] = vecVboTex_Verts[(i*7)+m+1]; //+1 because vecVbo[0s] are centroids?
        }
        // CALCULATE SIDES (SAME FOR EVERY HEXAGON)
        // TODO : make this function return the vector ...
        calculateSides();
        ofVec2f idRing = hexagonCanvas.getHexagonIdAndRing(i);

        int choosedPattern;// = int(ofRandom(105,105));
    
        if(startingAt == 0)
        {
            if(endingAt == 1) choosedPattern = 0;
            if(endingAt == 2) choosedPattern = 78;
            if(endingAt == 3) choosedPattern = 126;
            if(endingAt == 4) choosedPattern = 82;
            if(endingAt == 5) choosedPattern = 5;
        }
        else if(startingAt == 1)
        {
            if(endingAt == 0) choosedPattern = 0;
            if(endingAt == 2) choosedPattern = 1;
            if(endingAt == 3) choosedPattern = 79;
            if(endingAt == 4) choosedPattern = 127;
            if(endingAt == 5) choosedPattern = 83;
        }
        else if(startingAt == 2)
        {
            if(endingAt == 0) choosedPattern = 78;
            if(endingAt == 1) choosedPattern = 1;
            if(endingAt == 3) choosedPattern = 2;
            if(endingAt == 4) choosedPattern = 80;
            if(endingAt == 5) choosedPattern = 128;
        }
        else if(startingAt == 3)
        {
            if(endingAt == 0) choosedPattern = 126;
            if(endingAt == 1) choosedPattern = 79;
            if(endingAt == 2) choosedPattern = 2;
            if(endingAt == 4) choosedPattern = 3;
            if(endingAt == 5) choosedPattern = 81;
        }
        else if(startingAt == 4)
        {
            if(endingAt == 0) choosedPattern = 82;
            if(endingAt == 1) choosedPattern = 127;
            if(endingAt == 2) choosedPattern = 80;
            if(endingAt == 3) choosedPattern = 3;
            if(endingAt == 5) choosedPattern = 4;
        }
        else if(startingAt == 5)
        {
            if(endingAt == 0) choosedPattern = 5;
            if(endingAt == 1) choosedPattern = 83;
            if(endingAt == 2) choosedPattern = 128;
            if(endingAt == 3) choosedPattern = 81;
            if(endingAt == 4) choosedPattern = 4;
        }
        else
        {
            choosedPattern = int(ofRandom(105,105));
            cout << " !!!!!! RANDOM ???? WHYYY ????? !!!! " << endl;
        }
    
        pmHexagonTile actualTilePattern = hexagonTilesDictionary[choosedPattern];
        int numCucsActualHexagon = actualTilePattern.getConnections().size(); //int(ofRandom(0, 6));
        //cout << "Get number of cucs in this pattern id : " << choosedPattern << " :: Number of Connections = " << numCucsActualHexagon << endl;
        //int totalVerticesAddedToThisHexagon = 0;
        for(int k=0;k<numCucsActualHexagon;k++)
        {
            
            vecTempVboGrow_Verts.resize(numSteps*2,ofVec3f(0,0,0));
            vecTempVboGrow_Colors.resize(vecTempVboGrow_Verts.size(),ofFloatColor(0.0,1.0,1.0));
            //                vecTempVboGrow_TexCoords.resize(vecTempVbo_Verts.size(),ofVec2f(0,0));
            vecTempVboGrow_Faces.resize((numSteps-1)*2*3,0);
            //vecTempVboGrow_HexagonId.resize(vecTempVboGrow_Verts.size());
            
            if(k<numCucsActualHexagon)
            {
                // this is a user defined cuc ... take it normally
                sampledPoints.clear();
                ribs.clear();
                sampledPoints.resize(numSteps);
                ribs.resize(numSteps);
                for(int i=0;i<numSteps;i++)
                {
                    ribs[i].resize(2);
                }
                
                startSide = actualTilePattern.getConnections()[k].startsAt;
                if(startSide == startingAt)
                    endSide = actualTilePattern.getConnections()[k].endsAt;
                else{
                    endSide = startSide;
                    startSide = actualTilePattern.getConnections()[k].endsAt;
                }
                
                // STEPS
                calculateStartEndPointsAndCurve();
                
                toSVGPath.newSubPath();
                toSVGPath.moveTo(bezierLine.getVertices()[0]);

                for(int jj= 1; jj < bezierLine.getVertices().size() ; jj++)
                    toSVGPath.lineTo(bezierLine.getVertices()[jj] +  hexagonCanvas.getCentroidData()[i]);
                calculateRibs();
                calculateVboDataGrow();
                
            }
            else
            {
                // fill with "void" cucs
                lastFaceAddedToGrow=lastFaceAddedToGrow + ((numSteps)*2);
                
            }
            // inserting calculated hexagon cuc into vectors that will feed the vboCucs
            vecVboGrow_Verts.insert(vecVboGrow_Verts.end(), vecTempVboGrow_Verts.begin(), vecTempVboGrow_Verts.end());
            vecVboGrow_Faces.insert(vecVboGrow_Faces.end(), vecTempVboGrow_Faces.begin(), vecTempVboGrow_Faces.end());
            vecVboGrow_Colors.insert(vecVboGrow_Colors.end(), vecTempVboGrow_Colors.begin(), vecTempVboGrow_Colors.end());
            vecVboGrow_HexagonId.insert(vecVboGrow_HexagonId.end(),numSteps*2,float(i));
            //cout << "Hexagon id : " << i << " we've added " << numSteps << " vertexs to the HexagonId" << endl;
            
            //totalVerticesAddedToThisHexagon = totalVerticesAddedToThisHexagon + ((numSteps-1)*2*3) ;
            
            //matricesNumElements.insert(matricesNumElements.end(),totalVerticesAddedToThisHexagon,100);
            
            //                for(int m=0;m<totalVerticesAddedToThisHexagon;m++)
            //                {
            //                    matricesNumElements.insert(matricesNumElements.end(),totalVerticesAddedToThisHexagon,ofFloatColor(1,1,1,1));
            //                    matricesNumElements.push_back(ofFloatColor(0,0,0,0));
            //                }
        }
        //cout << "Hexagon num = " << i << " _ Total vertices added to hexagon : " << vecTempVboGrow_Verts.size() <<  endl;
        //cout << " -.-.-.-.-.- " << endl;
    }
    
//        svgSaver.addPath(toSVGPath);
//        svgSaver.save("HolaTest.svg");
    
    // FILL DATA INTO VBO CUCS
    vboGrow.setVertexData(vecVboGrow_Verts.data(), vecVboGrow_Verts.size(),GL_DYNAMIC_DRAW);
    vboGrow.setIndexData(vecVboGrow_Faces.data(), vecVboGrow_Faces.size(),GL_DYNAMIC_DRAW);
    vboGrow.setColorData(vecVboGrow_Colors.data(), vecVboGrow_Colors.size(),GL_DYNAMIC_DRAW);
    hexagonIdCustomLocation = 6;
    while(vboGrow.hasAttribute(hexagonIdCustomLocation))
    {
        hexagonIdCustomLocation++;
    }
    //cout << "Hexagon Id Custom Location : " << hexagonIdCustomLocation << endl;
    //cout << "Hexagon Id Custom data size : " << vecVboGrow_HexagonId.size() << endl;
    //cout << "While we had verts size  : " << vecVboGrow_Verts.size() << endl;
    
    vboGrow.setAttributeData( hexagonIdCustomLocation, reinterpret_cast<float *>( vecVboGrow_HexagonId.data() ),  1, vecVboGrow_HexagonId.size(), GL_STATIC_DRAW, sizeof( vecVboGrow_HexagonId[0] ) );

//    vboGrow.setAttributeData(hexagonIdCustomLocation, vecVboGrow_HexagonId.data(), 1, vecVboGrow_HexagonId.size(), GL_STATIC_DRAW);
    //void ofVbo::setAttributeBuffer(int location, ofBufferObject & buffer, int numCoords, int stride, int offset){Á
    //int customLocation = 5; //
    //while(hasAttribute(customLocation++));
    //vboGrow.setAttributeData(<#int location#>, <#const float *vert0x#>, <#int numCoords#>, <#int total#>, <#int usage#>)
    
    cout << "!!! vboGrow sizes _ Verts = " << vecVboGrow_Verts.size() << " Faces = " << vecVboGrow_Faces.size() << endl;
    cout << "!!! matricesNumElements size  = " << matricesNumElements.size() << endl;
    
}

//--------------------------------------------------------------
void ofApp::calculateVboDataGrow()
{
    
    // DEFINE VBO DATA
    
    // VERTEXS
    for(int i=0;i<vecTempVboGrow_Verts.size();i++)
    {
        int half = (vecTempVboGrow_Verts.size()/2)-1;
        if(i<=half)
        {
            vecTempVboGrow_Verts[i] = ribs[i][0];
        }
        else
        {
            vecTempVboGrow_Verts[i] = ribs[(numSteps-1)-(i-numSteps)][1];
        }
    }
    // FACES
    int totalVertexs = numSteps*2;
    for(int i=0;i<(numSteps-1);i++)
    {
        vecTempVboGrow_Faces[(i*6)+0] = i + lastFaceAddedToGrow;
        vecTempVboGrow_Faces[(i*6)+1] = i+1 + lastFaceAddedToGrow;
        vecTempVboGrow_Faces[(i*6)+2] = (totalVertexs-1)-i + lastFaceAddedToGrow;
        
        vecTempVboGrow_Faces[(i*6)+3] = i+1 + lastFaceAddedToGrow;
        vecTempVboGrow_Faces[(i*6)+4] = (totalVertexs-1)-i-1 + lastFaceAddedToGrow;
        vecTempVboGrow_Faces[(i*6)+5] = (totalVertexs-1)-i + lastFaceAddedToGrow;
        
    }
    lastFaceAddedToGrow=lastFaceAddedToGrow + ((numSteps)*2);
    //    single element (1)
    //    iterator insert (iterator position, const value_type& val);
    //    fill (2)
    //    void insert (iterator position, size_type n, const value_type& val);
    //    range (3)
    //    template <class InputIterator>
    //    void insert (iterator position, InputIterator first, InputIterator last);
    
    //    for(int i=0;i<vecTempVbo_Faces.size();i++)
    //    {
    //        //                cout << "Face : " << i << " :: " << vecVboCucs_faces[(i*3)] << " , " << vecVboCucs_faces[(i*3)+1] << " , " << vecVboCucs_faces[(i*3)+2] << endl;
    //        //        cout << "FaceTemp " << i << ": " << vecTempVbo_Faces[i] << endl;
    //    }
    
    // COLORS and TEXCOORDS
    float factor;
    for(int i=0;i<vecTempVboGrow_Verts.size();i++)
    {
        factor = ofMap(i,0,numSteps*2,0.0,1.0);
        //            if(i<8)
        //            {
        //                vecVboCucs_colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
        //            }
        //            else
        //            {
        //                vecVboCucs_colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
        //            }
        
        //        vecTempVbo_Colors[i] = ofFloatColor(1.0,1.0*factor,0.0,1.0);
//        if(int(i/numSteps)%2)
//        {
//            vecTempVboGrow_Colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);
//        }
//        else
//        {
//            vecTempVboGrow_Colors[i] = ofFloatColor(0.0,0.0,0.0,1.0);
//        }
        //        vecTempVboGrow_TexCoords[i] = ofVec2f(vecTempVboGrow_Verts[i].x,vecTempVboGrow_Verts[i].y);
                    vecTempVboGrow_Colors[i] = ofFloatColor(1.0,1.0,1.0,1.0);

    }
    
}

//---------------------------------------------------------------------------
bool ofApp::occupyOneHexagon(ofVec2f startingHexagon, int startingSide)
{
    
    vector<vector<int>> vIndexData = hexagonCanvas.getHexagonsIndexData();
    
    // we set it the origin as "used" on the usedHexagons
    int whichNumberOfHexagon = vIndexData[startingHexagon.x][startingHexagon.y];
    usedHexagons[vIndexData[startingHexagon.x][startingHexagon.y]] = true;
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
            cout << "Random choosed to go to : " << optionChoosed << endl;
            
            ofVec2f nextHexagon = ofVec2f(possibleNextHexagons[optionChoosed].x,possibleNextHexagons[optionChoosed].y);
            int whichSideItStarts = (possibleNumNexHexagons[optionChoosed]+3)%6;
            
            // add the next hexagon to the vector of hexagons in order to reconstruct the worm...
            growTileInfo g;
            g._num = hexagonCanvas.getHexagonNumberFromIdAndRing(startingHexagon);
            g._id = startingHexagon.x;
            g._ring = startingHexagon.y;
            g._startingAtSide = startingSide;
            g._endingAtSide = possibleNumNexHexagons[optionChoosed];
            
            growingHexagons.push_back(g);
            
            cout<< "INFO: " << g._num << " " << possibleNextHexagons.size()<<endl;
            isFinishingGrow = occupyOneHexagon(nextHexagon, whichSideItStarts);
  
            if(!isFinishingGrow){
                growingHexagons.pop_back();
                cout<<"removed"<<endl;
            }
            possibleNextHexagons.erase(possibleNextHexagons.begin()+optionChoosed);
            possibleNumNexHexagons.erase(possibleNumNexHexagons.begin()+optionChoosed);
            if(isFinishingGrow || !(possibleNextHexagons.size()>0)){
                foundCulDeSac = true;
            }
            
        }
        if(!isFinishingGrow) usedHexagons[vIndexData[startingHexagon.x][startingHexagon.y]] = false;
        return isFinishingGrow;
    }
    else
    {
        if(growingHexagons.size()>100) return true;
        else{
            usedHexagons[vIndexData[startingHexagon.x][startingHexagon.y]] = false;
            return false;
        }
//        return;
    }
    
}

/*
 // print hexagon index data
 // looking for the closer hexagons of hexagons with id = 0 ring = 0 ;
 vector<vector<int>> vIndexData = hexagonCanvas.getHexagonsIndexData();
 cout << "V Index Data sizes : " << vIndexData.size() << " , " << vIndexData[0].size() << endl;
 //
 //    for(int i=0;i<vIndexData.size();i++)
 //    {
 //        for(int j=0;j<vIndexData[i].size();j++)
 //        {
 //            cout << "index : " << i << " , " << j << " = " << vIndexData[i][j] << endl;
 //        }
 //
 //    }
 //
 
 
 
 
 */
void ofApp::onGuiMatrixEvent(ofxDatGuiMatrixEvent e)
{
    cout << "Matrix : " << e.child << endl;
//    if(ofGetKeyPressed(OF_KEY_SHIFT))
//        savePreset(e.child+1, bankSelect->getSelected()->getName());
//    else{
//        loadPreset(e.child+1, bankSelect->getSelected()->getName());
//    }
}

void ofApp::changedMatrixX(int &i)
{
    cout << "changedMatrixX :  " << i << endl;
    drawingSizeX = i;
    drawingMatrix = guiDrawing->getMatrix("Drawing");
    drawingMatrix = new ofxDatGuiMatrix("Drawing", drawingSizeX*drawingSizeY, false);
    guiDrawing->setWidth(drawingSizeX*50);
}
void ofApp::changedMatrixY(int &i)
{
    delete guiDrawing;
    
    cout << "changedMatrixY :  " << i << endl;
    drawingSizeY = i;
    drawingMatrix = guiDrawing->getMatrix("Drawing");
    drawingMatrix = new ofxDatGuiMatrix("Drawing", drawingSizeX*drawingSizeY, false);
    
    guiDrawing->setWidth(drawingSizeX*50);
}
