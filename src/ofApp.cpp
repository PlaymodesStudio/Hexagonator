
#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup(){


    // Use GL_TEXTURE_2D Textures (normalized texture coordinates 0..1)
    ofDisableArbTex();

    ofSetVerticalSync(false);
    ofBackground(32);
    
    /////////////////////////////
    /// VARS
    /////////////////////////////

    mode = 1;
    numModes = 2;
    numRings = 35;
    numHexasPerRing = 64;
    useShader = true;
    useTransformMatrix = true;
    useCubeColors = false;
    showVertices = false;
    numVertexsOneHexagonWithCenter = 7;
    numVertexsOneHexagon = 6;
    drawPrimitive = GL_TRIANGLES;
    
    

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
    
    //using Syphon app Simple Server, found at http://syphon.v002.info/
//    syphon.setApplicationName("Simple Server");
//    syphon.setServerName("");
//    syphon.set("","Simple Server");
    syphon.set("Gen_Grayscale","MIRABCN_Generator");
    syphon.bind();
    syphon.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    syphon.unbind();
    useSyphon = false;

    /////////////////////////////
    // IMAGE AS TEXTRE
    /////////////////////////////
    image.load("./tex/mapaPixels64x35_2.png");
    //image.load("./tex/eye.jpg");
    image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    //pmVbo1.setTextureReference(image.getTexture());
    
    // MASK
    mask.load("./masks/testMask.png");

    // VIDEO
    videoPlayer.load("./videos/rings.mov");
    videoPlayer.setLoopState(OF_LOOP_NORMAL);
    videoPlayer.play();
    while(!videoPlayer.isLoaded())
    {
      cout << " ... loading video " << endl;
    }
    videoPlayer.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    // !!!
    pmVbo1.setTextureReference(videoPlayer.getTexture());
    // !!!
    
    
    ////////////
    // OSC
    ////////////
    oscReceiver.setup(1234);
    vecOsc.resize(64);
    for(int i=0;i<vecOsc.size();i++)
    {
        vecOsc[i].resize(35);
        for(int j=0;j<35;j++)
        {
            vecOsc[i][j] = 1.0;
        }
    }
    
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
    pmVbo1.setTexCoordsData(hexagonCanvas.getTextureCoords(),0);
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
void ofApp::updateOsc()
{
    if(oscReceiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        oscReceiver.getNextMessage(m);
//        cout << m.getAddress() << endl;
        
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

    videoPlayer.update();
    videoPlayer.setSpeed(0.125);

}

//--------------------------------------------------------------
void ofApp::draw()
{
    //pmVbo1.updateVertData(vertexsRibbon,0);
    //vbo.updateVertexData(vecVboVerts[currentVboVerts].data(), vecVboVerts[currentVboVerts].size());
    
    /// DRAW SYPHON INTO FBO TO LATER RETRIEVE IT's TEXTURE
    fboSyphon.begin();
        syphon.draw(0,0,fboResolution.x,fboResolution.y);
    fboSyphon.end();
    
    
    string modeString;
    
    
    //    ofEnableSmoothing();
    //    ofEnableAntiAliasing();
    
    /// DRAW INTO FBO
    fboOut.begin();
    {
        ofSetColor(0,30,30);
        ofFill();
        ofDrawRectangle(0,0,1200,1200);
        
        
        if(useShader)
        {
            shader.begin();
            // choose which texture to feed into the shader (image or syphon)
            if(!useSyphon)
            {
                shader.setUniformTexture("uTexture", videoPlayer, 2);
//                image.bind();
//                shader.setUniform1i("uTexture", 2);
//                image.unbind();
                
            }
            else
            {
                shader.setUniformTexture("uTexture", fboSyphon.getTexture(), 2);
            }
       }
        
        if(mode==0)
        {
            modeString = "svg draw";
            
            ofSetColor(255,255,255);
            svg.draw();
        }
        else if (mode == 1)
        {
            modeString = "pmVBO " ;

            ofSetColor(255);
            shader.setUniform4f("u_color", ofFloatColor(1.0,0.5,0.0,1.0));
            
            if(useTransformMatrix) shader.setUniform1i("u_useMatrix", 1);
            else shader.setUniform1i("u_useMatrix", 0);

            if(useCubeColors) shader.setUniform1i("u_useCubeColors", 1);
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
//    ofSetColor(255);
//    mask.draw(0,0,1200,1200);

    // DRAW CENTER AND CIRCLE RADIUS
//    ofSetColor(255);
//    ofDrawLine(0,600,1200,600);
//    ofDrawLine(600,0,600,1200);
//    ofNoFill();
//    ofSetCircleResolution(128);
//    ofDrawCircle(600,600,ofGetMouseX());
//    ofFill();


    ofNoFill();
    ofSetCircleResolution(128);

    for(int i=0;i<hexagonCanvas.ringsRadius.size();i++)
    {
      ofDrawCircle(600,600,hexagonCanvas.ringsRadius[i]);
    }
    ofFill();
    
    
    /// END FBO !! 
    fboOut.end();

    /// DRAW FBO TO SCREEN
    ofPushMatrix();
        ofSetColor(255,255,255);
        fboOut.draw(0,0,ofGetHeight(),ofGetHeight());
    ofPopMatrix();

    // draw texture previews
//    ofSetColor(255);
//    image.draw(0,ofGetHeight()-100,200,200);
//    syphon.draw(200,ofGetHeight()-100,200,200);

    /// DRAW INFO STRING
    ofSetColor(255);
    ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())) + " | Mode : " + ofToString(mode) + " " +modeString + " Shader? : " + ofToString(useShader),10,ofGetHeight()*.90 +  30);
    
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
    else if(key=='t')
    {
        int i = (pmVbo1.currentVboTexCoords + 1)%3;
        pmVbo1.setTexCoordsIndex(i);
        cout << " Set Tex Coords : " << i << endl;
    }
    else if(key=='u')
    {
        pmVbo1.setUseTexture(true);
    }
    else if(key=='U')
    {
        pmVbo1.setUseTexture(false);
    }
    else if(key=='m')
    {
        pmVbo1.setDrawMode(CONTOURS);
    }
    else if(key=='M')
    {
        pmVbo1.setDrawMode(TRIANGLES);
    }
    else if(key=='n')
    {
        pmVbo1.setDrawMode(QUADS);
    }
    else if(key=='p')
    {
        drawPrimitive=GL_LINE_LOOP;
    }
    else if(key=='P')
    {
        drawPrimitive=GL_TRIANGLES;
    }
    else if(key=='o')
    {
        drawPrimitive=GL_QUADS;
    }
    else if(key=='h')
    {
        useShader = !useShader;
    }
    else if(key=='1')
    {
        pmVbo1.setVertices(0);
        cout << "1" << endl;
    }
    else if(key=='2')
    {
        pmVbo1.setVertices(1);
        cout << "2" << endl;
    }
    else if(key=='3')
    {
        pmVbo1.setVertices(2);
        cout << "3" << endl;

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
    else if(key=='y')
    {
        useSyphon = !useSyphon;
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
    useSyphon = false;
    
    if( info.files.size() > 0 )
    {
        image.load(info.files[0]);
        image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        pmVbo1.setTextureReference(image.getTexture());
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
