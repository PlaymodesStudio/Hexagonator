//
//  pmHexagonCanvas.cpp
//  8_GuellGenerator_2
//
//  Created by emaduell on 16/12/16.
//
//

#include "pmHexagonCanvas.hpp"


//----------------------------------------------------------
pmHexagonCanvas::pmHexagonCanvas()
{
    numRings=35;
    numIdsPerRing=64;

    //    vector<float> ringsRadius = { 142.224,153.599,163.647,175.084,186.791,198.733,210.743,223.235,235.692,249.293,262.73,276.35,290.542,304.961,318.797,333.73,348.333,363.898,379.651,395.129,410.934,428.658,446.176,463.945,482.558,501.519,521.008,542.217,563.313,584.573,607.385,656.708,630.816,680.376,706.984};
    ringsRadius = { 142.5,153.5,165.5,168.5,188.791,198.733,210.743,223.235,235.692,249.293,262.73,276.35,290.542,304.961,318.797,333.73,348.333,363.898,379.651,395.129,410.934,432.658,450.176,468.945,488.558, 505.519,521.008,548.217,569.313,588.573,612.385,636,664.816,680.376,706.984};

}

//----------------------------------------------------------
void pmHexagonCanvas::setup(string _filename)
{
    svgFilename = _filename;
    
    svg.load(svgFilename);
    
    cout << "Setup :: Main SVG opened :: " << svgFilename << endl;
    cout << "SVGpaths has " << svg.getNumPath() << " paths. " << endl;

    numHexagons = svg.getNumPath();

    // resize index data to size of pixel texture
    hexagonsIndexData.resize(numRings);
    for(int i=0;i<numRings;i++)
    {
        hexagonsIndexData[i].resize(numIdsPerRing,-1);
    }
    // resize hexagonsData to num hexagons
    hexagonsData.resize(numHexagons);

    for(int i=0;i<numHexagons;i++)
    {
        // for each path/hexagon ... create the hexagon data
        ofPath p = svg.getPathAt(i);
        hexagonsData[i].hexagon.setup(p, i, ofVec2f(1200,1200));
        
        // determine which Ring and which IDInRing
        orderHexagonOnRingsAndIds(i);

        // remake texture coordinates based on rings and ids for a 64 x 35 texture ...
        vector<ofVec2f> newTexCoord;
        newTexCoord.resize(7);
        for(int j=0;j<newTexCoord.size();j++)
        {
            //newTexCoord[j] = ofVec2f(0.5,0.5);
            float halfPixelWidth = (1.0/64.0)/2;
            float halfPixelHeight = (1.0/35.0)/2;
            newTexCoord[j] = ofVec2f((float(hexagonsData[i].whichIdInRing)/64.0) + halfPixelWidth , (float(hexagonsData[i].whichRing)/35.0) + halfPixelHeight	);
        }
        hexagonsData[i].hexagon.setHexagonTexCoord(newTexCoord);
    }
    
    
    
}



//----------------------------------------------------------
vector<ofVec3f> pmHexagonCanvas::getVertexData()
{
    vector<ofVec3f> vec;
    vec.resize(numHexagons*7);
    
    for(int i=0;i<numHexagons;i++)
    {
        vector<ofVec3f> points;
        points = hexagonsData[i].hexagon.getVertexs();
        
        
        for(int j=0;j<7;j++)
        {
            vec[(i*7)+j] = points[j];
            
        }
    }
    
    return vec;
}
//----------------------------------------------------------
vector<ofVec3f> pmHexagonCanvas::getOriginalVertexData()
{
    vector<ofVec3f> vec;
    vec.resize(numHexagons*7);
    
    for(int i=0;i<numHexagons;i++)
    {
        vector<ofVec3f> points;
        points = hexagonsData[i].hexagon.getOriginalVertexs();
        
        
        for(int j=0;j<7;j++)
        {
            vec[(i*7)+j] = points[j];
            
        }
    }
    
    return vec;
}


//----------------------------------------------------------
vector<ofVec2f> pmHexagonCanvas::getTextureCoords()
{
    vector<ofVec2f> vec;
    vec.resize(numHexagons*7);
    
    for(int i=0;i<numHexagons;i++)
    {
        vector<ofVec2f> texCoords;
        texCoords = hexagonsData[i].hexagon.getTexCoordinates();
        
        for(int j=0;j<7;j++)
        {
            vec[(i*7)+j] = texCoords[j];
            
        }
    }
    
    return vec;
}


//----------------------------------------------------------
vector<ofFloatColor> pmHexagonCanvas::getColorData()
{
    vector<ofFloatColor> vec;
    vec.resize(numHexagons*7);
    
    for(int i=0;i<numHexagons;i++)
    {
        vector<ofFloatColor> colors;
        colors = hexagonsData[i].hexagon.getColors();
        

        for(int j=0;j<7;j++)
        {
            vec[(i*7)+j] = colors[j];
        }
    }
    
    return vec;
}

//----------------------------------------------------------
vector<ofIndexType> pmHexagonCanvas::getFaceData()
{
    vector<ofIndexType> vec;
    vec.resize(numHexagons*6*3);
    
    for(int i=0;i<numHexagons;i++)
    {
        vector<ofIndexType> faces;
        faces = hexagonsData[i].hexagon.getFaces();

        for(int j=0;j<18;j++)
        {
            vec[(i*18)+j] = (i*7) + faces[j];

        }
    }
    
    return vec;
}

//----------------------------------------------------------
vector<ofPoint> pmHexagonCanvas::getCentroidData()
{
    vector<ofPoint> vec;
    vec.resize(numHexagons);
    for(int i=0;i<numHexagons;i++)
    {
       
        ofPoint point;
        point = hexagonsData[i].hexagon.getCentroid();
        vec[i] = point;
    }
    
    return vec;
}


//--------------------------------------------------------------
void pmHexagonCanvas::orderHexagonOnRingsAndIds(int _index)
{
    float angleStepPerHexa = 360.0 / numIdsPerRing ;
    
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
        else if(j==8) specialOffset = -0.35; //* sin(ofGetElapsedTimef()/8);
        else if(j==9) specialOffset = -3.0; //* sin(ofGetElapsedTimef()/8);
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
        else if(j==24) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==25) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==26) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==27) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==28) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==29) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==34) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==35) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==37) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==38) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==39) specialOffset = -0.15; //* sin(ofGetElapsedTimef()/8);
        else if(j==40) specialOffset = -1.35; //* sin(ofGetElapsedTimef()/8);
        else if(j==41) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==42) specialOffset = -1.55; //* sin(ofGetElapsedTimef()/8);
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
        else if(j==54) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
        else if(j==55) specialOffset = -0.55; //* sin(ofGetElapsedTimef()/8);
        else specialOffset = -1.5;
        
//        if(i==1) // draw ray lines
//        {
//            ofSetColor(255,0,255,128);
//            ofDrawLine(600, 600,600 + 1000*cos(ofDegToRad(angleStepPerHexa*j + specialOffset)),600 + 1000*sin(ofDegToRad(angleStepPerHexa*j + specialOffset)) );
//            ofDrawBitmapString(ofToString(j), 600 + 300*cos(ofDegToRad((angleStepPerHexa*j + specialOffset))), 600 + 300*sin(ofDegToRad((angleStepPerHexa*j + specialOffset))));
//        }
        
        
        //calculate the projected distance
        //ofPoint ofApp::projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd)
        
//        p = projectPointToLine(hexagonsData[_index].hexagon.getCentroid(), ofPoint(600,600), ofPoint(600 + 1200*cos(ofDegToRad(j*angleStepPerHexa + specialOffset)),600 + 1200*sin(ofDegToRad(j*angleStepPerHexa + specialOffset))));
        p = projectPointToLine(hexagonsData[_index].hexagon.getCentroid(), ofPoint(600,600), ofPoint(600 + 1200*cos(ofDegToRad(j*angleStepPerHexa + specialOffset)),600 + 1200*sin(ofDegToRad(j*angleStepPerHexa + specialOffset))));
        
        if(hexagonsData[_index].hexagon.getCentroid().distance(p) < minDiff)
        {
            whichIndexInRing = j;
            minDiff = hexagonsData[_index].hexagon.getCentroid().distance(p);
            minP=p;
            
        }
        
        
        
    }
    
    /// RING ORDER
    /////////////////
    
    float x = hexagonsData[_index].hexagon.getCentroid().x - svg.getWidth()/2;
    float y = svg.getHeight()/2- hexagonsData[_index].hexagon.getCentroid().y;
    
    float radius = ofPoint(x,y).length();
    float angle = ofRadToDeg(atan2(y,x));
    angle = angle<0 ? angle+360 : angle;
    
    int whichRing = -1;
    float minDiffAngle = 1000000.0f;
    
    for(int j=0;j<ringsRadius.size();j++)
    {
        float diff = fabs(radius-ringsRadius[j]);
        
        if(diff < minDiffAngle)
        {
            minDiffAngle = diff;
            whichRing=j;
        }
        
    }
    
//    if(i%2) whichRing=0;
//    else whichRing = 706.984;
    
    // RINGS ORDER put data 
    hexagonsData[_index].whichRing = whichRing;
    hexagonsData[_index].whichIdInRing = whichIndexInRing;
    
    // set index Data into the right position
    hexagonsIndexData[whichRing][whichIndexInRing] = _index;
}

//--------------------------------------------------------------
ofPoint pmHexagonCanvas::projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd)
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

