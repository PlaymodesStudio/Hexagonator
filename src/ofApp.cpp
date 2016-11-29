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
    numRings = 35;
    numHexasPerRing = 64;
    useShader = true;
    numVertexsOneHexagonWithCenter = 7;
    numVertexsOneHexagon = 6;
    drawPrimitive = GL_TRIANGLES;
    
    
    /////////////////////////////
    /// PM VBO
    /////////////////////////////
    
    pmVbo1.setup(svg.getNumPath(),7);

    
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
    /// SYPHON
    /////////////////////////////
    syphon.setup();
    //using Syphon app Simple Server, found at http://syphon.v002.info/
    //syphon.set("","Simple Server");
    syphon.set("MIRABCN_Generator","MIRAMAD_Generator");
    useSyphon = false;

    /////////////////////////////
    // IMAGE AS TEXTRE
    /////////////////////////////
    image.load("./tex/mapaPixels64x35.png");
    image.load("./tex/eye.jpg");
    image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    pmVbo1.setTextureReference(image.getTexture());
    
    // use syphon texture
    if(useSyphon)
    {
        syphon.bind();
        syphon.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        pmVbo1.setTextureReference(syphon.getTexture());
        syphon.unbind();
    }
    
    /////////////////////////////
    /// SVG
    /////////////////////////////
    
    
    svgFilename = "./svg/test_svg_part.svg";
//    svgFilename = "./svg/testSVG9Hexagons.svg";
//    svgFilename = "./svg/test_svg_part_nomes2.svg";
//    svgFilename = "./svg/testOrdreRadial.svg";
//    svgFilename = "./svg/polarExampleAngle.svg";
//    svgFilename = "./svg/test_svg_partCENTRAT.svg";

    svg.load(svgFilename);

    cout << "Setup :: Main SVG opened :: " << svgFilename << endl;
    cout << "SVGpaths has " << svg.getNumPath() << " paths. " << endl;
    
    /// READ SVG INTO svgPolys vector !
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
    // PREPARE VERTEXS AND VECTORS
    /////////////////////////////

    // allocating "hexaPix" final destination
    // hexaPixel stores information on the relation of each hexagon id and xyz position as an order to generate Texture Coordinates
    // to draw a given data "texture" (syphon). As ex. hexaPix is [m][n].
    // 'm' is the id inside a given ring (0..63) .
    // 'n' is the ring id (0..35)
    // that is why thtat this "pixel mapping" texture coordinates allow us to read a 64x35 texture and give each pixel color to a full hexagon.
    
    hexaPix.resize(numRings);
    for(int i=0;i<numRings;i++)
    {
        hexaPix[i].resize(numHexasPerRing,hexagonPixel());
        for(int j=0;j<numHexasPerRing;j++)
        {
            hexagonPixel h;
            h._num=-1;
            h._hexaCentroidIndex=-1;
            h._ring=-1;
            
            hexaPix[i][j] = h;
        }
    }
    
    // VERTEX ARRAYS for feeding the VBO    
    vertexsTransformed.resize(svg.getNumPath()*numVertexsOneHexagonWithCenter, ofVec3f());
    vertexsOriginal.resize(svg.getNumPath()*numVertexsOneHexagonWithCenter, ofVec3f());
    
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
            hexagonCentroids.push_back(lines[j].getCentroid2D());
            
            // get the vertices of the path (hexagon)
            vector<ofPoint> vec  = lines[j].getVertices();
            
            if(vec.size()!=6) cout << " !!! Some polygons have not 6 vertexs on the path !! " << vec.size() << " i : " << i << endl;
            else cout << ">>>> Current path : " << i << " number of vertexs = " << vec.size() << " __ AREA is : " << lines[j].getArea() << endl;
            
            //////////////////////////////
            // INTERNAL ORDER (CW vs CCW)
            //////////////////////////////

            // http://stackoverflow.com/questions/14505565/detect-if-a-set-of-points-in-an-array-that-are-the-vertices-of-a-complex-polygon?noredirect=1&lq=1
            // http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
            // with the area we can detect is the polyline is CW or CCW
            // area < 0 means that we need to reverse the order of the vertexs, then area will become >0
            
            vector<ofPoint> vecV;
            vecV.resize(vec.size());
            
            if(lines[j].getArea()>0)
            {
                vecV = reverseVerticesInVector(vec);
            }
            else
            {
                vecV = vec;
            }
            
            //////////////////////////////
            // FIRST ELEMENT ORDER (based on minimum distance to origin (600,600))
            //////////////////////////////
            // this ordering of the vertices of each hexagon allows us to have them with it's first vertex as the closest to the image center.
            // this is necessary for drawing "cube" effect over the hexagons.
            
            vector<ofPoint> vecOrdered = orderVerticesOfHexagonBasedOnDistanceToOrigin(vecV);
            
            
            //////////////////////////////
            // HEXAGONs ORDER BASED ON RINGS
            //////////////////////////////

            orderHexagonOnRingsAndIds(i);

            // create the vertexs vector as follows = vertex[0] will always be the centroid, and the rest are the triangulation (6 triangles for 1 hexagon)
            //      _
            //     /.\
            //     \_/
            //

            // VERTS :  TBO model matrix makes the trick with centroid !
            ////////////////////////////////////////////////////////////////////////////

            // vertexsOriginal !! 7 vertexs x hexagon :: EACH HEXAGON SITS in it's initial position
            vertexsOriginal[(i*numVertexsOneHexagonWithCenter)+0] = ofVec3f(hexagonCentroids[i]);

            // vertexsTransformed !! 7 vertexs x hexagon :: EACH HEXAGON IS MOVED TO ORIGIN !
            // the center [+0] need to be transported to Origin (0,0,0).
            // we keep this translation with the hexagonsCentroids array. (that will be passed to the shader as a model matrix)
            vertexsTransformed[(i*numVertexsOneHexagonWithCenter)+0] = ofVec3f(0.0,0.0,0.0);
            
            // for each vertexs on the path ... add it to the vertexs vectors
            for(int k=0;k<vecOrdered.size();k++)
            {
                vertexsTransformed[(i*numVertexsOneHexagonWithCenter)+k+1] = vecOrdered[k] - hexagonCentroids[i];
                vertexsOriginal[(i*numVertexsOneHexagonWithCenter)+k+1] = vecOrdered[k];
                
            }
        }
    }
    
    /////////////////////////////
    // PREPARE COLOR
    /////////////////////////////
    vector<ofFloatColor> colorsA;
    colorsA.resize(vertexsTransformed.size(),ofFloatColor());
    for(int j=0;j<vertexsTransformed.size();j++)
    {
        float factor = float(j) / float(vertexsTransformed.size());
        ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
        colorsA[j] = color;
    }

    
    /////////////////////////////
    // PREPARE TEXTURE COORDS
    /////////////////////////////

    // TEXCOORDS 0 (0...1200 , 0...1200)
    // this draws full texture over svg

    vector<ofVec2f> texCoordA;
    vector<ofVec2f> texCoordB;
    vector<ofVec2f> texCoordC;
    texCoordA.resize(vertexsTransformed.size(),ofVec2f());
    texCoordB.resize(vertexsTransformed.size(),ofVec2f());
    texCoordC.resize(vertexsTransformed.size(),ofVec2f());
    
    for(int i=0;i<vertexsTransformed.size();i++)
    {
        // for each vertex we need to create a ofVec2f
        ofVec2f v;
        ofVec2f vB;
        ofVec2f vC;
        
        int whichHexagonAreWe = (i/numVertexsOneHexagonWithCenter);

        // for a given hexagon "whichHexagonAreWe" ...
        v = ofVec2f(-1,-1);
        // UV coordinates are not normalized !! are w,h !!
        vB = ofVec2f((vertexsOriginal[i].x/float(fboOut.getWidth())) * image.getWidth(), (vertexsOriginal[i].y/float(fboOut.getHeight()))* image.getHeight());
        // this draws the texture quantized to the hexagons based on it's UV coordinates of the centroid
        vC = ofVec2f((vertexsOriginal[whichHexagonAreWe*numVertexsOneHexagonWithCenter].x / float(fboOut.getWidth()))* image.getWidth(),(vertexsOriginal[whichHexagonAreWe*numVertexsOneHexagonWithCenter].y / float(fboOut.getHeight()) ) * image.getHeight());

        for(int n=0;n<hexaPix.size();n++)
        {
            for(int m=0;m<hexaPix[n].size();m++)
            {
                if(hexaPix[n][m]._hexaCentroidIndex==whichHexagonAreWe)
                {
                    v = ofVec2f(hexaPix[n][m]._num,hexaPix[n][m]._ring);
                }
            }
        }

        
        texCoordA[i]=v;
        texCoordB[i]=vB;
        texCoordC[i]=vC;
        //cout << "setup __ TexCoord : " << i << " :: " << texCoordA[i] << endl;
    }
    
//    // TEXCOORDS 1
//    // this draws the texture quantized to the hexagons based on it's UV coordinates of the centroid
//    vector<ofVec2f> texCoordB;
//    texCoordB.resize(vertexsOriginal.size(),ofVec2f());
//
//    for(int i=0;i<vertexsOriginal.size();i++)
//    {
//        // for each vertex we need to create a ofVec2f
//        ofVec2f v;
//        int whichHexagonAreWe = (i/numVertexsOneHexagonWithCenter);
//        v = ofVec2f((vertexsOriginal[whichHexagonAreWe*numVertexsOneHexagonWithCenter].x / float(fboOut.getWidth()))* image.getWidth(),(vertexsOriginal[whichHexagonAreWe*numVertexsOneHexagonWithCenter].y / float(fboOut.getHeight()) ) * image.getHeight());
////        v = ofVec2f((vertexsTransformed[i].x/float(fboOut.getWidth())) * image.getWidth(), (vertexsTransformed[i].y/float(fboOut.getHeight()))* image.getHeight());
//
//        texCoordB[i]=v;
//    }
//    
    
    /// UPDATE TEX COORDS
//    {
//        vector<ofVec2f> texCoordA;
//        texCoordA.resize(pmVbo1.getVertices(0).size(),ofVec2f());
//        
//        for(int k=0;k<pmVbo1.getVertices(0).size();k++)
//        {
//            cout << k << " / "  << pmVbo1.getVertices(0).size() << endl;
//            // for each vertex we need to create a ofVec2f
//            ofVec2f v;
//            int whichHexagonAreWe = (k/numVertexsOneHexagonWithCenter);
//            
//            // UV coordinates are not normalized !! are w,h !!
//            //v = ofVec2f((vertexsTransformed[i].x/float(fboOut.getWidth())) * image.getWidth(), (vertexsTransformed[i].y/float(fboOut.getHeight()))* image.getHeight());
//            // for a given hexagon "whichHexagonAreWe" ...
//            v = ofVec2f(-1,-1);
//            float IndexStep = 1.0; //syphon.getWidth() / numHexasPerRing;
//            float RingStep = 1.0; //syphon.getWidth() / numRings;
//            for(int n=0;n<hexaPix.size();n++)
//            {
//                for(int m=0;m<hexaPix[n].size();m++)
//                {
//                    if(hexaPix[n][m]._hexaCentroidIndex==whichHexagonAreWe)
//                    {
//                        v = ofVec2f((hexaPix[n][m]._num * IndexStep) + 0.5,(hexaPix[n][m]._ring * RingStep) + 0.5);
//                    }
//                }
//            }
//            
//            texCoordA[k]=v;
//            
//            //cout << "setup __ TexCoord : " << i << " :: " << texCoordA[i] << endl;
//        }
//        pmVbo1.setTexCoordsData(texCoordA,0);
//    }

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
    
    pmVbo1.setVertData(vertexsOriginal, 0);
    pmVbo1.setColorData(colorsA,0);
    pmVbo1.setFacesData(faces,0);
    pmVbo1.setTexCoordsData(texCoordC,2);
    pmVbo1.setTexCoordsData(texCoordB,1);
    pmVbo1.setTexCoordsData(texCoordA,0);
    pmVbo1.setDrawMode(TRIANGLES);

    
    /// TBO STUFF (Texture Buffer Object)
    //////////////////////////////////////
    
    matrices.resize(svg.getNumPath());
    
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

    
    
    
    shader.begin();
    shader.setUniformTexture("tex",tex,0);
    shader.setUniform1i("u_numHexags",svg.getNumPath());
    shader.setUniform4f("u_color", ofFloatColor(1.0,0.5,0.0,1.0));
    shader.setUniform1i("u_useMatrix", 1);
    shader.end();


}

//--------------------------------------------------------------
void ofApp::orderHexagonOnRingsAndIds(int i)
{
    float angleStepPerHexa = 360.0 / numHexasPerRing ;
    
    // ORDERS !! IN DRAW ....
    // draw projections
//    for(int i=0;i<hexagonCentroids.size();i++)
//    {
        /// 0..64 ORDER INSIDE A RING ...
        
        ofPoint p;
        ofPoint minP;
        
        float minDiff = 12000;
        ofPoint minPoint;
        int whichIndexInRing = -1;
        
        for(int j=0;j<64;j++)
        {
            
            float specialOffset = 0.0;
            if(j==4) specialOffset = -0.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==5) specialOffset = -0.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==7) specialOffset = -0.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==9) specialOffset = -0.35; //* sin(ofGetElapsedTimef()/8);
            else if(j==10) specialOffset = -0.35; //* sin(ofGetElapsedTimef()/8);
            else if(j==11) specialOffset = -0.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==12) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==13) specialOffset = -0.75; //* sin(ofGetElapsedTimef()/8);
            else if(j==14) specialOffset = -1.0; //* sin(ofGetElapsedTimef()/8);
            else if(j==15) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==16) specialOffset = -1.75; //* sin(ofGetElapsedTimef()/8);
            else if(j==17) specialOffset = -1.75; //* sin(ofGetElapsedTimef()/8);
            else if(j==18) specialOffset = -1.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==19) specialOffset = -1.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==20) specialOffset = -1.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==21) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==22) specialOffset = -1.5; //* sin(ofGetElapsedTimef()/8);
            else if(j==23) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==24) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==25) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==26) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==27) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==28) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==29) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==34) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==35) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==37) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==38) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==39) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==40) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==41) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==42)specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==43) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==44) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==45) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==46) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==47) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==48) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==49) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==50) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==51) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==52) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==53) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==54) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else if(j==55) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
            else specialOffset = 0;
            
            if(i==1) // draw ray lines
            {
                ofSetColor(255,0,255,128);
                ofDrawLine(600, 600,600 + 1000*cos(ofDegToRad(angleStepPerHexa*j + specialOffset)),600 + 1000*sin(ofDegToRad(angleStepPerHexa*j + specialOffset)) );
                ofDrawBitmapString(ofToString(j), 600 + 300*cos(ofDegToRad((angleStepPerHexa*j + specialOffset))), 600 + 300*sin(ofDegToRad((angleStepPerHexa*j + specialOffset))));
            }
            
            
            //calculate the projected distance
            //ofPoint ofApp::projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd)
            
            p = projectPointToLine(hexagonCentroids[i], ofPoint(600,600), ofPoint(600 + 1200*cos(ofDegToRad(j*angleStepPerHexa + specialOffset)),600 + 1200*sin(ofDegToRad(j*angleStepPerHexa + specialOffset))));
            
            if(hexagonCentroids[i].distance(p) < minDiff)
            {
                whichIndexInRing = j;
                minDiff = hexagonCentroids[i].distance(p);
                minP=p;
                
            }
            
            
            
        }
        
        /// RING ORDER
        /////////////////
        
        float x = hexagonCentroids[i].x - svg.getWidth()/2;
        float y = svg.getHeight()/2- hexagonCentroids[i].y;
        
        float radius = ofPoint(x,y).length();
        float angle = ofRadToDeg(atan2(y,x));
        angle = angle<0 ? angle+360 : angle;
        
        int whichRing = -1;
        vector<float> ringsRadius = { 142.224,152.899,163.647,175.084,186.791,198.733,210.743,223.235,235.692,249.293,262.73,276.35,290.542,304.961,318.797,333.73,348.333,363.898,379.651,395.129,410.934,428.658,446.176,463.945,482.558,501.519,521.008,542.217,563.313,584.573,607.385,656.708,630.816,680.376,706.984};
        float minDiffAngle = 10000000;
        
        for(int j=0;j<ringsRadius.size();j++)
        {
            float diff = fabs(radius-ringsRadius[j]);
            if(diff < minDiffAngle)
            {
                minDiffAngle = diff;
                whichRing=j;
            }
        }
        
        //        // draw centroids
        //        float alpha;
        //
        //        if(whichIndexInRing%2==0) alpha = 255;
        //        else alpha = 32;
        //
        //        if(whichRing%8 == 0) ofSetColor(255.0,0,0,alpha);
        //        else if(whichRing%8 == 1) ofSetColor(255.0,255,0,alpha);
        //        else if(whichRing%8 == 2) ofSetColor(0,255,0,alpha);
        //        else if(whichRing%8 == 3) ofSetColor(0,255,255,alpha);
        //        else if(whichRing%8 == 4) ofSetColor(0,0,255,alpha);
        //        else if(whichRing%8 == 5) ofSetColor(255,0,255,alpha);
        //        else if(whichRing%8 == 6) ofSetColor(255,255,0,alpha);
        //        else if(whichRing%8 == 7) ofSetColor(255,255,255,alpha);
        //        else ofSetColor(255.0 * (float(whichRing)/ringsRadius.size()),alpha);
        //
        //        ofDrawCircle(hexagonCentroids[i].x,hexagonCentroids[i].y, 10);
        //
        //        // draw centroid to min point
        //        ofSetColor(255,0,0,255);
        //        ofDrawLine(hexagonCentroids[i],minP);
        //
        //        // draw minP
        //        ofSetColor(0,255,0);
        //        ofDrawCircle(minP.x,minP.y, 1);
        
        // RINGS ORDER
        
        hexaPix[whichRing][whichIndexInRing]._hexaCentroidIndex = i;
        hexaPix[whichRing][whichIndexInRing]._num = whichIndexInRing;
        hexaPix[whichRing][whichIndexInRing]._ring = whichRing;
        
        //        vector<float> ringRadius;
        //        ringRadius.resize(19);
        //        ringRadius = {142,153,165,175,187,198,210,224,236,250,264,277,291,306,320,335,349,366,381}
        
//    }
    
}


//--------------------------------------------------------------
vector<ofPoint> ofApp::reverseVerticesInVector(vector<ofPoint> _v)
{
    vector<ofPoint> vecAux;
    vecAux.resize(_v.size());
    
    for(int i=0;i<_v.size();i++)
    {
        vecAux[i] = _v[(_v.size()-1)-i];
    }
    return vecAux;
}
//--------------------------------------------------------------
vector<ofPoint> ofApp::orderVerticesOfHexagonBasedOnDistanceToOrigin(vector<ofPoint> _v)
{
    
    /// INNER ORDERING : the first vertex of each hexagon has to be the closest to the origin
    //get the closest vertex to origin and it's index;
    ofPoint origin = ofPoint(svg.getWidth()/2,svg.getHeight()/2);
    float minDistToOrigin = 100000;
    int minDistToOrigin_index = -1;
    for (int k=0;k<_v.size();k++)
    {
        if((origin.distance(_v[k])) < minDistToOrigin)
        {
            minDistToOrigin = origin.distance(_v[k]);
            minDistToOrigin_index = k;
        }
        //                else if (origin.distance(vecV[k]) == minDistToOrigin)
        //                {
        //                    if(vecV[k].x > vecV[minDistToOrigin_index].x)
        //                    {
        //                        minDistToOrigin = origin.distance(vecV[k]);
        //                        minDistToOrigin_index = k;
        //                        cout << "OOOOOOOO" << endl;
        //                    }
        //                }
        //                else{
        //                    cout << ">>>>>>>> " << k << " : dist " << origin.distance(vecV[k])  << " min is : " << minDistToOrigin << endl;
        //
        //                }
    }
    
    // sort vertices based on distance to origin and recreate a vector with good order
    vector<ofPoint> vecOrdered;
    vecOrdered.resize(_v.size());
    
    for(int k=0;k<_v.size();k++)
    {
        //cout << " reordering [" <<k<<"]" << (minDistToOrigin_index + k ) % 6 << endl;
        vecOrdered[k]=_v[(minDistToOrigin_index + k ) % 6];
        
    }
    // END OF INNER ORDERING
    return vecOrdered;
}

//--------------------------------------------------------------
void ofApp::updateMatrices()
{
    for(size_t i=0;i<matrices.size();i++){
        ofNode node;
        ofVec3f scale(1.0,1.0,1.0);
        
        float factor = sin (ofGetElapsedTimef()*2 + (i*0.5));
        
        node.setPosition(hexagonCentroids[i]);
        node.setScale((ofMap(factor,-1.0,1.0,0.2,0.8)/1.0));

        matrices[i] = node.getLocalTransformMatrix();
    }
    
    
    // and upload them to the texture buffer
    buffer.updateData(0,matrices);
//    bufferB.updateData(0,matricesB);
}

//--------------------------------------------------------------
void ofApp::update()
{
    updateMatrices();
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

    ofSetColor(0,16,32);
    ofFill();

    ofDrawRectangle(0,0,svg.getWidth(),svg.getHeight());
    
    if(useShader) shader.begin();
    
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
//        shader.setUniform4f("u_color", ofFloatColor(0.0,0.0,0.0,1.0));
//        shader.setUniform1i("u_useMatrix", 0);
        shader.setUniform4f("u_color", ofFloatColor(1.0,0.5,0.0,1.0));
        shader.setUniform1i("u_useMatrix", 0);
        pmVbo1.draw(drawPrimitive);

    }

    // ... END SHADING
    if(useShader) shader.end();

//    // DRAW VERTEX COORDINATES
//    ofSetColor(255,255,0);
//    vector<ofVec3f> v= pmVbo1.getVertices(0);
//    for(int i=0;i<v.size();i++)
//    {
//        if(i%7==0)
////            if(true)
//        {
//            ofDrawBitmapString(ofToString(i/7)  ,v[i].x, v[i].y) ; //+" : " + ofToString(v[i]),v[i].x, v[i].y);
//            //ofDrawBitmapString(".",v[i].x,v[i].y);
//            //ofDrawBitmapString(ofToString(vboTexCoord[i]), vboVert[i].x, vboVert[i].y+20);
//        }
//
//    }

    // draw helpers
    ///////////////////
    
    // draw center
    ofSetColor(255,0,255);
    ofDrawCircle(600,600, 5);

    ofSetColor(255,0,255,128);

    
//    // draw rays of 64ths
//    for(int i=0;i<64;i++)
//    {
//        float specialOffset = 0.0;
//        if(i==4) specialOffset = -0.5 ;//* sin(ofGetElapsedTimef()/8);
//        else specialOffset = 0;
//        
//        ofDrawLine(600, 600,600 + 1000*cos(ofDegToRad(angleStepPerHexa*i + specialOffset)),600 + 1000*sin(ofDegToRad(angleStepPerHexa*i + specialOffset)) );
//        
//        ofDrawBitmapString(ofToString(i), 600 + 300*cos(ofDegToRad((angleStepPerHexa*i + specialOffset))), 600 + 300*sin(ofDegToRad((angleStepPerHexa*i + specialOffset))));
//    }
    
//    // draw centroids
//    for(int i=0;i<hexagonCentroids.size();i++)
//    {
//        //ofDrawLine(600, 600,600 + 1000*cos(ofDegToRad((angleStep*i)+ (angleStep/2.0))),600 + 1000*sin(ofDegToRad((angleStep*i)+ (angleStep/2.0))) );
//        ofSetColor(255,255,0);
//        ofDrawCircle(hexagonCentroids[i].x,hexagonCentroids[i].y, 3);
//    }

    
    
    fboOut.end();

    /// DRAW FBO TO SCREEN
    ofPushMatrix();
    ofSetColor(255,255,255);
    fboOut.draw(0,0,ofGetHeight(),ofGetHeight());


    /// THIS IS THE WAY I FIGURED OUT HOW TO MAKE SYHPON WORK ...
    
    if(useSyphon && ofGetFrameNum()==300)
    {
        syphon.bind();
        pmVbo1.setTextureReference(syphon.getTexture());
        pmVbo1.texture.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        syphon.unbind();
    }
    
    ofPopMatrix();
    
    /// DRAW INFO STRING
    ofSetColor(255);
    ofDrawBitmapString("FPS : " + ofToString(int(ofGetFrameRate())) + " | Mode : " + ofToString(mode) + " " +modeString + " Shader? : " + ofToString(useShader),10,ofGetHeight()*.90 +  30);
    
    /// SAVE IMAGE
    if(saveNow)
    {
        saveNow = false;
        ofPixels pixels;
        fboOut.readToPixels(pixels);
        ofImage image;
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
    else if(key=='p')
    {
        drawPrimitive=GL_LINE_LOOP;
    }
    else if(key=='P')
    {
        drawPrimitive=GL_TRIANGLES;
    }
    else if(key=='h')
    {
        useShader = !useShader;
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
