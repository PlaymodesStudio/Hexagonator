#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup(){

    ofSetVerticalSync(false);
    ofBackground(32);
    
    /////////////////////////////
    /// VARS
    /////////////////////////////

    mode = 1;
    numModes = 2;
    
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

    
    /////////////////////////////
    /// SVG
    /////////////////////////////
    
    svgFilename = "./svg/test_svg_complet.svg";
    svg.load(svgFilename);
    
    cout << "Setup :: Main SVG opened :: " << svgFilename << endl;
    cout << "SVGpaths has " << svg.getNumPath() << " paths. " << endl;
    
    for (int i = 0; i < svg.getNumPath(); i++)
    {
        ofPath p = svg.getPathAt(i);
        // svg defaults to non zero winding which doesn't look so good as contours
        p.setPolyWindingMode(OF_POLY_WINDING_ODD);
        
        svgPaths.push_back(p);
        
        vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(p.getOutline());
        for(int j=0;j<(int)lines.size();j++)
        {
            svgPolys.push_back(lines[j].getResampledBySpacing(1));
        }
        
    }
    cout << "SVGpolys has " << svgPolys.size()  << " polylines objects. " << endl;
    
    /////////////////////////////
    /// PM VBO
    /////////////////////////////
    
    pmVbo1.setup(svg.getNumPath(),7);
    
    
    /////////////////////////////
    // IMAGE FOR TEXTRE
    ////////////////////
    image.load("./tex/eye.jpg");
    image.getTexture().getWidth();
    pmVbo1.setTextureReference(image.getTexture());

    
    /////////////////////////////
    // PREPARE VERTEXS
    /////////////////////////////

    vector<ofVec3f> vertexsA;
    
    vertexsA.resize(svg.getNumPath()*numVertexsOneHexagonWithCenter, ofVec3f());
    
    // for each path (hexagon)
    for(int i=0;i<svg.getNumPath();i++)
    {
        ofPath p = svg.getPathAt(i);
        // svg defaults to non zero winding which doesn't look so good as contours
        p.setPolyWindingMode(OF_POLY_WINDING_ODD);
        // generate the polyline so we can extract the vertex info and pass it to vboVert ...
        vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(p.getOutline());
        // the lines.size is = 1 always ... just one line for each path.
        for(int j=0;j<(int)lines.size();j++)
        {
            // get hexagon center ...
            polygonCentroids.push_back(lines[j].getCentroid2D());
            // get the vertices of the path (hexagon)
            vector<ofPoint> vecP  = lines[j].getVertices();
            if(vecP.size()!=6) cout << " !!! Some polygons have not 6 vertexs on the path !! " << vecP.size() << " i : " << i << endl;
            
            // create the vboVert as follows = vertex[0] will always be the centroid, and the rest are the triangulation (6 triangles for 1 hexagon)
            //      _
            //     /.\
            //     \_/
            //
            
            // VERTS [0] : centroid + 6 v for each hexagon
            /////////////////////////////////////////////
            vertexsA[(i*numVertexsOneHexagonWithCenter)+0] = ofVec3f(polygonCentroids[i]);
            for(int k=0;k<vecP.size();k++)
            {
                vertexsA[(i*numVertexsOneHexagonWithCenter)+k+1] = vecP[k];
            }
            
//            // VERTS [1] : 6v for each hexagon
//            /////////////////////////////////
//            for(int k=0;k<vecP.size();k++)
//            {
//                vertexsB[(i*numVertexsOneHexagon)+k ] = vecP[k];
//            }

        }
    }
    
//    for(int i=0;i<vertexsB.size();i++)
//    {
//        cout << " VertexsB" << i << " :: " << vertexsB[i] << endl;
//    }
    
    /////////////////////////////
    // PREPARE COLOR
    /////////////////////////////
    vector<ofFloatColor> colorsA;
    colorsA.resize(vertexsA.size(),ofFloatColor());
    for(int j=0;j<vertexsA.size();j++)
    {
        float factor = float(j) / float(vertexsA.size());
        ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
        colorsA[j] = color;
    }

    
    /////////////////////////////
    // PREPARE TEXTURE COORDS
    /////////////////////////////

    // TEXCOORDS 0
    // this draws full texture over svg

    vector<ofVec2f> texCoordA;
    texCoordA.resize(vertexsA.size(),ofVec2f());
    
    for(int i=0;i<vertexsA.size();i++)
    {
        // for each vertex we need to create a ofVec2f
        ofVec2f v;
        // UV coordinates are not normalized !! are w,h !!
        v = ofVec2f(vertexsA[i].x, vertexsA[i].y);
        
        texCoordA[i]=v;
        
        //cout << "setup __ TexCoord : " << i << " :: " << texCoordA[i] << endl;
    }
    
    // TEXCOORDS 1
    // this draws the texture quantized to the hexagons based on it's UV coordinates of the centroid
    vector<ofVec2f> texCoordB;
    texCoordB.resize(vertexsA.size(),ofVec2f());

    for(int i=0;i<vertexsA.size();i++)
    {
        // for each vertex we need to create a ofVec2f
        ofVec2f v;
        int whichHexagonAreWe = (i/numVertexsOneHexagonWithCenter);
        
        v = ofVec2f(vertexsA[whichHexagonAreWe*numVertexsOneHexagonWithCenter].x,vertexsA[whichHexagonAreWe*numVertexsOneHexagonWithCenter].y);

        texCoordB[i]=v;
    }
//
//    // TEXCOORDS 2
//    // this should get a gradient of UV coordinates based on the vector (origin to centroid) ?¿
//    /////////////////////////////
//
//    int numOfHexagons = svg.getNumPath();
//    for(int i=0;i<numOfHexagons;i++)
//        {
//            ofPoint currentCentroid = vecVboVerts[0][i*numVertexsOneHexagonWithCenter];
//            ofPoint center = ofPoint(svg.getWidth()/2 , svg.getHeight()/2);
//            
//            for(int j=0;j<7;j++)
//            {
//                ofPoint projectedPoint = projectPointToLine(vecVboVerts[0][(i*numVertexsOneHexagonWithCenter)+j],center,currentCentroid);
//                ofVec2f v = ofVec2f(projectedPoint.x,projectedPoint.y);
//                vecVboTexCoords[2][(i*numVertexsOneHexagonWithCenter)+j]=v;
//            }
//        }
//
//    // TEXCOORDS 3
//    // this should get a gradient of UV coordinates based on the vector (origin to centroid) ?¿
//    /////////////////////////////
//
//    for(int i=0;i<numOfHexagons;i++)
//    {
//        ofPoint currentCentroid = vecVboVerts[0][i*numVertexsOneHexagonWithCenter];
//        ofPoint center = ofPoint(svg.getWidth()/2 , svg.getHeight()/2);
//        
//        vecVboTexCoords[3][(i*numVertexsOneHexagonWithCenter)]=ofVec2f(currentCentroid.x,currentCentroid.y);
//        for(int j=1;j<6;j++)
//        {
//            ofPoint projectedPoint = projectPointToLine(vecVboVerts[0][(i*numVertexsOneHexagonWithCenter)+j],center,currentCentroid);
//            ofVec2f v = ofVec2f(projectedPoint.x,projectedPoint.y);
//            vecVboTexCoords[3][(i*numVertexsOneHexagonWithCenter)+j]=v;
//        }
//    }
//    
    
    
    /////////////////////////////
    // PREPARE THE FACES
    /////////////////////////////
    // each path (an hexagon) becomes into 6 faces ( allocate x3 i1,i2,i3 3 indeces per face/triangle)

    vector<ofIndexType> faces;
    faces.resize((numVertexsOneHexagon)*3*svg.getNumPath(),ofIndexType());

    int numFaces =svg.getNumPath()*(numVertexsOneHexagonWithCenter-1);
    for(int i=0;i<numFaces;i++)
    {
        int whichHexagonAreWe = (i/numVertexsOneHexagon)+1;
        int whichFaceInHexa = (i%numVertexsOneHexagon);

        faces[(i*3)+0] = (whichHexagonAreWe-1)*7 ;
        faces[(i*3)+1] = (i+1)+(whichHexagonAreWe-1);

        // if it's the last face of an hexagon we need to close the hexagon so that the last vertex of the last triangle = the second vertex (first is center) of the first triangle
        if(whichFaceInHexa==5)
        {
            faces[(i*3)+2] = ((whichHexagonAreWe-1)*7) + 1;
        }
        else
        {
            faces[(i*3)+2] = i+2+(whichHexagonAreWe-1);
        }
        //cout << "setup __ Face : " << i << " : " << faces[(i*3)+0] << " , " << faces[(i*3)+1] << " , " << faces[(i*3)+2] << endl;
    }
    
    
    // SET VBO DATA
    /////////////////////////////

    pmVbo1.setVertData(vertexsA, 0);
    pmVbo1.setColorData(colorsA,0);
    pmVbo1.setFacesData(faces,0);
    pmVbo1.setTexCoordsData(texCoordA,0);
    pmVbo1.setTexCoordsData(texCoordB,1);
    
    pmVbo1.setDrawMode(TRIANGLES);

    
    /// TBO STUFF (Texture Buffer Object)
    //////////////////////////////////////
    
    matrices.resize(svg.getNumPath()*7*4);
    
    // upload the transformation for each box using a
    // texture buffer.
    // for that we need to upload the matrices to the buffer
    // and allocate the texture using it
    buffer.allocate();
    buffer.bind(GL_TEXTURE_BUFFER);
    buffer.setData(matrices,GL_STREAM_DRAW);
    
    // using GL_RGBA32F allows to read each row of each matrix
    // as a float vec4 from the shader.
    // Note that we're allocating the texture as a Buffer Texture:
    // https://www.opengl.org/wiki/Buffer_Texture
    tex.allocateAsBufferTexture(buffer,GL_RGBA32F);

    // now we bind the texture to the shader as a uniform
    // so we can read the texture buffer from it
    shader.begin();
    shader.setUniformTexture("tex",tex,0);
    shader.end();

    updateMatrices();

}
//--------------------------------------------------------------
void ofApp::updateMatrices()
{
    for(size_t i=0;i<matrices.size();i++){
        ofNode node;
        ofVec3f scale(1.0,1.0,1.0);
        node.setScale(scale);

        matrices[i] = node.getLocalTransformMatrix();
    }
    
    // and upload them to the texture buffer
    buffer.updateData(0,matrices);
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw()
{
    
    string modeString;
    
    /// DRAW INTO FBO
    fboOut.begin();
    
//    ofEnableSmoothing();
//    ofEnableAntiAliasing();

    // SHADING ...

    ofSetColor(0,128,255);
    ofFill();

    ofDrawRectangle(0,0,svg.getWidth(),svg.getHeight());
    
    shader.begin();
    
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
        pmVbo1.draw(drawPrimitive);
    }

    // ... END SHADING
    shader.end();

    
    fboOut.end();

    /// DRAW FBO TO SCREEN
    ofPushMatrix();
    ofSetColor(255,255,255);
    fboOut.draw(0,0,ofGetHeight(),ofGetHeight());
    ofPopMatrix();
    
    /// DRAW INFO STRING
    ofSetColor(255);
    ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())) + " | Mode : " + ofToString(mode) + " " +modeString,10,ofGetHeight()*.90 +  30);
    
    /// SAVE IMAGE
    if(saveNow)
    {
        saveNow = false;
        ofPixels pixels;
        fboOut.readToPixels(pixels);
        ofImage image;
        image.allocate(fboResolution.x,fboResolution.y, OF_IMAGE_COLOR);
        image.setFromPixels(pixels);
        image.save("./captures/" + ofGetTimestampString() +".png" );
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
        int i = (pmVbo1.currentVboTexCoords + 1)%2;
        pmVbo1.setTexCoordsIndex(i);
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
    else if(key=='p')
    {
        drawPrimitive=GL_LINE_LOOP;
    }
    else if(key=='P')
    {
        drawPrimitive=GL_TRIANGLES;
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
        image.load(info.files[0]);
    }

}

