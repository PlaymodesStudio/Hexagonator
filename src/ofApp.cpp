
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
    useTransformMatrix = true;
    useCubeColors = false;
    showVertices = false;
    numVertexsOneHexagonWithCenter = 7;
    numVertexsOneHexagon = 6;
    drawPrimitive = GL_TRIANGLES;
    recordedFrame = 0;
    lastTimeWeReceivedOsc = 0.0;
    usingOsc = false;
    
    /////////////////////////////
    /// GUI
    /////////////////////////////

    //
//    parameters.setName("Parameter test");
//    parameters.add(labelTest.set("This is a label_label", " "));
//    parametersControl::addDropdownToParameterGroupFromParameters(parameters, "dropdownTest", {"video", "syphon", "image", "svg Sequence"}, dropdownTest);
//    parameters.add(saveFilename.set("Save name", "test.mov"));
//    parameters.add(colorPicker.set("color", ofColor::white, ofColor::white, ofColor::black));
    
    // GUI VARS
    dropdown_whichTextureSource = 1; // image as initial texture source
    dropdown_whichTexCoord = 0 ;
    
    parametersGraphics.setName("Hexagonator");
    parametersGraphics.add(toggle_showLayout.set("Show Layout",true));
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Source",{"Texture","Quads"},dropdown_whichSource);
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Texture Coordinates",{"64x35","1200x1200"},dropdown_whichTexCoord);
    dropdown_whichTexCoord.addListener(this,&ofApp::changedTexCoord);
    parametersControl::addDropdownToParameterGroupFromParameters(parametersGraphics,"Texture Source",{"Image","Video","Syphon","Syph.Max"},dropdown_whichTextureSource);
    
    // LISTENERS
    dropdown_whichTexCoord.addListener(this,&ofApp::changedTexCoord);

    // CREATE
    parametersControl::getInstance().createGuiFromParams(parametersGraphics, ofColor::orange);
    //parametersControl::getInstance().createGuiFromParams(parameters, ofColor::orange);
    parametersControl::getInstance().setup();
    
    

    
    
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

    #ifdef TARGET_OPENGLES
        shader.load("shadersES2/shader");
    #else
        if(ofIsGLProgrammableRenderer()){
            cout << "Working on GL Programable Render mode" << endl;
            shader.load("shadersGL3/shader");
        }else
        {
            cout << "Working on a NOT GL Programable Render mode" << endl;
            shader.load("shadersGL2/shader");
        }
    #endif

    /////////////////////////////
    /// FBO
    /////////////////////////////

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


    /////////////////////////////
    /// SYPHON
    /////////////////////////////
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

    syphonMax.set("midihex","Max");
    syphonMax.bind();
    syphonMax.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    syphonMax.unbind();

    
    /////////////////////////////
    // IMAGE AS TEXTRE
    /////////////////////////////
    imageFilename = "./testMedia/mapaPixels64x35.png";
    image.load(imageFilename);
    //image.load("./tex/eye.jpg");
    image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    //pmVbo1.setTextureReference(image.getTexture());
    
    // MASK
    mask.load("./testMedia/masks/maskAll.png");
    maskWireframe.load("./testMedia/masks/wireYellow.png");
    

    // VIDEO
    videoFilename = "./testMedia/indexs.mov";
    videoPlayer.load(videoFilename);
    videoPlayer.setLoopState(OF_LOOP_NORMAL);
    if(dropdown_whichTextureSource==1) videoPlayer.play();
    
    
    while(!videoPlayer.isLoaded())
    {
      cout << " ... loading video " << endl;
    }
    videoPlayer.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    
    
    
    
    ////////////
    // OSC
    ////////////
    oscReceiver.setup(1234);
    resetVecOscVector();
    
    // HEXAGONS DATA AND CANVAS
    /////////////////////////////
    
    
    svgFilename = "./svg/test_svg_part.svg";
  svgFilename = "./svg/santi2.svg";
//        svgFilename = "./svg/testSVG3Hexagons.svg";
    //    svgFilename = "./svg/test_svg_part_nomes2.svg";
    //    svgFilename = "./svg/testOrdreRadial.svg";
    //    svgFilename = "./svg/polarExampleAngle.svg";
    //    svgFilename = "./svg/test_svg_partCENTRAT.svg";
    //    svgFilename = "./svg/vector_complet_v1.svg";
    //    svgFilename = "./svg/vector_complet_v2_flat.svg";

    
    hexagonCanvas.setup(svgFilename);
    
    /////////////////////////////
    /// PM VBO
    /////////////////////////////
    
    pmVbo1.setup(hexagonCanvas.getNumHexagons(),7);

    
//    // SET VBO DATA
//    /////////////////////////////
//
    pmVbo1.setVertData(hexagonCanvas.getVertexData(), 0);

    //////
    //////
    // vector<ofVec2f> vecTexCoord
    // try to get centroid data to convert it to texCoord of centroids (we need 2 way of texCoord ... based on Rings and based on Centroids)
    // we got 1 centroid per path = 2735 paths
    vector<ofPoint> vCentroidPoints = hexagonCanvas.getCentroidData();
    cout << "Getting texCoord Data ... size " << hexagonCanvas.getTextureCoords().size() << endl;
    cout << "Getting Centroid Data ... size " << vCentroidPoints.size() << endl;
    vector<ofVec2f> vecCentroidTexCoord;
    vecCentroidTexCoord.resize(hexagonCanvas.getNumHexagons()*7);
    cout << "MY Centroid Data ... size " << vecCentroidTexCoord.size() << endl;
    
    for(int i=0;i<hexagonCanvas.getNumHexagons();i++)
    {
        for(int j=0;j<7;j++)
        {
            vecCentroidTexCoord[(i*7)+j] = vCentroidPoints[i] / 1200.0 ;
        }
    }
    // index 1 we store the texcoords "normal" ... flat texture mapping
    pmVbo1.setTexCoordsData(vecCentroidTexCoord,1);
    // index 0 we store the texcoords "ring and id" mode 
    pmVbo1.setTexCoordsData(hexagonCanvas.getTextureCoords(),0);

    //pmVbo1.setTexCoordsData(, 1);
    //////
    //////

    pmVbo1.setColorData(hexagonCanvas.getColorData(),0);
    pmVbo1.setFacesData(hexagonCanvas.getFaceData(),0);
    pmVbo1.setDrawMode(TRIANGLES);
    
    /// TRANSFORM TBO STUFF (Texture Buffer Object)
    //////////////////////////////////////
    
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
    //////////////////////////////////////
    
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
    
     
    // SHADER INITS
    ////////////////
    
    shader.begin();
    shader.setUniformTexture("texTransform",texTransform,0);
    shader.setUniformTexture("texCubeColors",texCubeColors,1);
    shader.setUniform1i("u_numHexags",hexagonCanvas.getNumHexagons());
    shader.setUniform4f("u_color", ofFloatColor(1.0,0.5,0.0,1.0));
    shader.setUniform1i("u_useMatrix", 1);
    shader.end();

    updateMatrices();
    
    
 
    

}


//--------------------------------------------------------------
void ofApp::changedTexCoord(int &i)
{
    pmVbo1.setTexCoordsIndex(i);
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
void ofApp::updateMatrices()
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
    float ribbonWidth = 1.0 * (sin(ofGetElapsedTimef()) + 1.0)/8.0 ;
    //ribbonWidth = ofRandomuf();

    int howManyHexagonsWeVisited = 0;
    for(int i=0;i<numHexasPerRing;i++)
    {
        for(int j=0;j<numRings;j++)
        {
            if(hexaPix[j][i]._pathId != -1 )
            {
                vector<ofPoint>     vertOfHexagon;
                ofPoint p1,p2,p3,p4;
                ofVec2f v01,v34;
                vertOfHexagon.resize(numVertexsOneHexagonWithCenter);
                float ribbonWidthDivider = 1.0;
                
                // first get the 7 original vertices of the array
                for(int k=0;k<7;k++)
                {
                    int w = hexaPix[j][i]._pathId;
                    vertOfHexagon[k] = vertexsOriginal[(w*numVertexsOneHexagonWithCenter)+1 + k ];
                }
//                cout << " HEXA " << hexaPix[j][i]._ring << " , " << hexaPix[j][i]._num << " :: " << (float(hexaPix[j][i]._ring + 1)) / float(numRings) << endl;

                ribbonWidthDivider = 4.0 * (float(hexaPix[j][i]._ring + 0)) / float(numRings);
                
                // VERTS RIBBON [2] : 4v for each hexagon -> to draw the ribbon
                ////////////////////////////////////////////////////////
                
                v01 = ofVec2f(vertOfHexagon[1] - vertOfHexagon[0]);
                v34 = ofVec2f(vertOfHexagon[4] - vertOfHexagon[3]);
                
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
                //        vertexsRibbon[(i*4)+0] = p1;
                //        vertexsRibbon[(i*4)+1] = p2;
                //        vertexsRibbon[(i*4)+2] = p3;
                //        vertexsRibbon[(i*4)+3] = p4;
                
                
                
                //           typedef struct
                //        {
                //            int _hexaCentroidIndex;
                //            int _ring;
                //            int _num;
                //        } hexagonPixel;
                
                
                p1 = vertOfHexagon[0] + (v01/2.) - (v01 * (ribbonWidth*ribbonWidthDivider));
                p2 = vertOfHexagon[0] + (v01/2.) + (v01 * (ribbonWidth*ribbonWidthDivider));
                p3 = vertOfHexagon[3] + (v34/2.) - (v34 * (ribbonWidth*ribbonWidthDivider));
                p4 = vertOfHexagon[3] + (v34/2.) + (v34 * (ribbonWidth*ribbonWidthDivider));
                //p2 = vecOrdered[0] + (v01 * 0.66);
                //            p3 = vecOrdered[3] + (v34 * 0.33);
                //            p4 = vecOrdered[3] + (v34 * 0.66);
                
                vertexsRibbon[(howManyHexagonsWeVisited*4)+0] = p1;
                vertexsRibbon[(howManyHexagonsWeVisited*4)+1] = p2;
                vertexsRibbon[(howManyHexagonsWeVisited*4)+2] = p3;
                vertexsRibbon[(howManyHexagonsWeVisited*4)+3] = p4;
                //
                
                howManyHexagonsWeVisited = howManyHexagonsWeVisited + 1 ;
                
            }
        }
    }
    
//    cout << "ribbon width : " << ribbonWidth << " HowMany Visited : " << howManyHexagonsWeVisited <<  endl;
//    cout << vertexsRibbon[0][0] << endl;

}

//--------------------------------------------------------------
void ofApp::update()
{
    //updateVertexsForQuad();
    updateOsc();
    updateMatrices();

    if(dropdown_whichSource==HEX_SOURCE_TEXTURE && dropdown_whichTextureSource==HEX_TEXTURE_VIDEO && videoPlayer.isLoaded())
    {
        videoPlayer.update();
    }

   
}

//--------------------------------------------------------------
void ofApp::draw()
{
    //pmVbo1.updateVertData(vertexsRibbon,0);
    //vbo.updateVertexData(vecVboVerts[currentVboVerts].data(), vecVboVerts[currentVboVerts].size());
    
    /// DRAW SYPHON INTO FBO TO LATER RETRIEVE IT's TEXTURE
    if((dropdown_whichTextureSource == HEX_TEXTURE_SYPHON || dropdown_whichTextureSource == HEX_TEXTURE_SYPHON_MAX) || dropdown_whichSource==HEX_SOURCE_TEXTURE)
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
//                shader.setUniformTexture("uTexture", videoPlayer, 2);
//                shader.setUniformTexture("uTexture", image, 2);
//                image.bind();
//                shader.setUniform1i("uTexture", 2);
//                image.unbind();
                
       }
        
        if (mode == 0)
        {
            ofSetColor(255);
            shader.setUniform4f("u_color", ofFloatColor(1.0,0.5,0.0,1.0));
            
            if(useTransformMatrix) shader.setUniform1i("u_useMatrix", 1);
            else shader.setUniform1i("u_useMatrix", 0);

            if(useCubeColors)
            {
                shader.setUniform1i("u_useCubeColors", 1);
            }
            else shader.setUniform1i("u_useCubeColors", 0);

            pmVbo1.draw(drawPrimitive);

        }

        // ... END SHADING
        if(useShader) shader.end();

        if(showVertices)
        {
            // DRAW VERTEX COORDINATES
            ofSetColor(255,255,0);
            vector<ofVec3f> v= pmVbo1.getCurrentVertices();
            int whichHexagon = 0;
            for(int i=0;i<v.size();i++)
            {
                whichHexagon = float(i)/7.0;
                if(true)
                {
                    ofDrawBitmapString(ofToString(i)  ,v[i].x + hexagonCanvas.getCentroidData()[whichHexagon].x, v[i].y + hexagonCanvas.getCentroidData()[whichHexagon].y) ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
                }
            }
        }
    }
    
    /// DRAW THE MASK
    if(toggle_showLayout)
    {
        ofSetColor(128,128,128,128);
        maskWireframe.draw(0,0,1200,1200);
    }
    ofSetColor(255);
    mask.draw(0,0,1200,1200);

    /// DRAW VIDEO FILE INFO
    if(isRecording) ofSetColor(255,0,0);
    else ofSetColor(128);
    
    if(dropdown_whichSource==HEX_SOURCE_TEXTURE)
    {
        switch (dropdown_whichTextureSource)
        {
            case HEX_TEXTURE_IMAGE:
                ofDrawBitmapString(imageFilename + " : " + ofToString(videoPlayer.getCurrentFrame()),550,30);
                ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), 550,45);
                break;
            case HEX_TEXTURE_VIDEO:
                if(videoPlayer.isLoaded())
                {
                    ofDrawBitmapString(videoFilename + " // Current Frame :  " + ofToString(videoPlayer.getCurrentFrame()),550,30);
                }
                ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), 550,45);
                break;
            case HEX_TEXTURE_SYPHON:
                ofDrawBitmapString("Syhpon",550,30);
                ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), 550,45);
                break;
            case HEX_TEXTURE_SYPHON_MAX:
                ofDrawBitmapString("Syhpon Max",550,30);
                ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())), 550,45);
                break;
                
            default:
                break;
        }
    }
    if(isRecording)
    {
        ofDrawBitmapString("REC : Recorded Frame : " + ofToString(recordedFrame) + " // Time : " +ofToString(ofGetElapsedTimef()), 550,60 );
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

    
    
    if(isRecording && (dropdown_whichTextureSource==1))
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
        fboOut.draw(0,0,ofGetWidth(),ofGetWidth());
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
    else if(key=='u')
    {
        pmVbo1.setUseTexture(true);
    }
    else if(key=='U')
    {
        pmVbo1.setUseTexture(false);
    }
    else if(key=='n')
    {
        pmVbo1.setDrawMode(QUADS);
    }
    else if(key=='o')
    {
        drawPrimitive=GL_QUADS;
    }
    else if(key=='p')
    {
        if(videoPlayer.isPaused()) videoPlayer.setPaused(false);
        else videoPlayer.setPaused(true);
        //videoPlayer.stop();
        cout << " pausing " << endl;
//        drawPrimitive=GL_LINE_LOOP;
    }
    else if(key=='-')
    {
        videoPlayer.nextFrame();
        //        drawPrimitive=GL_TRIANGLES;
    }
    else if(key=='.')
    {
        videoPlayer.previousFrame();
        //        drawPrimitive=GL_TRIANGLES;
    }
    else if(key=='h')
    {
        useShader = !useShader;
    }
    else if(key=='i')
    {
        showVertices = !showVertices;
    }
    else if(key=='r')
    {
        useTransformMatrix = !useTransformMatrix;
    }
    else if(key=='c')
    {
        useCubeColors = !useCubeColors;
    }
    else if(key=='q')
    {
        isRecording=!isRecording;
        
        if(isRecording)
        {
            if(dropdown_whichTextureSource==1) // video
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
            if(dropdown_whichTextureSource!=1)
            {
                capture.stopRecording();
            }
            else if (dropdown_whichTextureSource==1)
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
    dropdown_whichSource = HEX_SOURCE_TEXTURE;
    
    if( info.files.size() > 0 )
    {
        vector<string> dragFileName = ofSplitString(info.files[0], "/");
        string dragFileExtension = ofSplitString(dragFileName[dragFileName.size()-1],".")[1];
        cout << ">> Dragged File ... \n" << info.files[0] << " : ext: " << dragFileExtension << endl;
        if(dragFileExtension=="png" || dragFileExtension == "jpg")
        {
            dropdown_whichTextureSource = 0;
            cout << "Loading new texture ... " << endl;
            imageFilename = info.files[0];
            image.load(imageFilename);
            image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            pmVbo1.setTextureReference(image.getTexture());
            
            if(videoPlayer.isLoaded()) videoPlayer.stop();
            
        }
        else if (dragFileExtension=="mov")
        {
            dropdown_whichTextureSource = 1;
            cout << "Loading new video ... " << endl;
            videoFilename=ofToString("./videos/" + ofSplitString(dragFileName[dragFileName.size()-1],".")[0] +"." +dragFileExtension);
            videoPlayer.load(videoFilename);
            videoPlayer.setLoopState(OF_LOOP_NORMAL);
            videoPlayer.play();
            while(!videoPlayer.isLoaded())
            {
                cout << " ... loading video " << endl;
            }
            videoPlayer.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            pmVbo1.setTextureReference(videoPlayer.getTexture());
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
                      
