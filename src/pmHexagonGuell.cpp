//
//  pmHexagonGuell.cpp
//  
//
//  Created by emaduell on 15/12/16.
//
//

#include "pmHexagonGuell.hpp"

int numHexasPerRing = 64;
int numRings = 35;
int numVertexsOneHexagonWithCenter = 7;
int numVertexsOneHexagon=6;

//----------------------------------------------------------
pmHexagonGuell::pmHexagonGuell()
{
    
}

//----------------------------------------------------------
void pmHexagonGuell::setup(ofPath _path, int _pathId, ofVec2f _compositionResolution)
{
    // get PathId ... so whichs path from the SVG are we ... is it needed ? not sure now ... ;)
    pathId = _pathId;
    // get composition resolution ... needed to calculate the center or origin (needed to order vertices inside an hexagon based on distance to center)
    compositionResolution = _compositionResolution;
    centerOrigin = ofPoint(compositionResolution.x / 2.0 , compositionResolution.y / 2.0);
    
    _path.setPolyWindingMode(OF_POLY_WINDING_ODD);
    // generate the polyline so we can extract the vertex info and pass it to vboVert ...
    vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(_path.getOutline());
    // we assume the lines.size is = 1 always ... just one line for each path.
    // get hexagon center ...
    centroid = lines[0].getCentroid2D();
        
    // get the vertices of the path (hexagon)
    vector<ofPoint> vec  = lines[0].getVertices();
    
    if(vec.size()!=6) cout << " !!! Some polygons have not 6 vertexs on the path !! " << vec.size() << endl;
    //else cout << ">>>> Current path : " << i << " number of vertexs = " << vec.size() << endl;

    //////////////////////////////
    // INTERNAL ORDER (CW vs CCW)
    //////////////////////////////
    
    // http://stackoverflow.com/questions/14505565/detect-if-a-set-of-points-in-an-array-that-are-the-vertices-of-a-complex-polygon?noredirect=1&lq=1
    // http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    // with the area we can detect is the polyline is CW or CCW
    // area < 0 means that we need to reverse the order of the vertexs, then area will become >0
    
    vector<ofPoint> vecCCW;
    vecCCW.resize(vec.size());
    
    if(lines[0].getArea()>0)
    {
        vecCCW = reverseVerticesInVector(vec);
    }
    else
    {
        vecCCW = vec;
    }
    
    //////////////////////////////
    // FIRST ELEMENT ORDER (based on minimum distance to origin (600,600))
    //////////////////////////////
    // this ordering of the vertices of each hexagon allows us to have them with it's first vertex as the closest to the image center.
    // this is necessary for drawing "cube" effect over the hexagons.
    
    vector<ofPoint> vecOrdered = orderVerticesOfHexagonBasedOnDistanceToOrigin(vecCCW);
    
    // create the vertexs vector as follows = vertex[0] will always be the centroid, and the rest are the triangulation (6 triangles for 1 hexagon)
    //      _
    //     /.\
    //     \_/
    //
    
    vertexsOriginals.resize(numVertexsOneHexagonWithCenter);
    vertexs.resize(numVertexsOneHexagonWithCenter);
    colors.resize(vertexs.size());
    texCoordinates.resize(vertexs.size());
    faces.resize(numVertexsOneHexagon * 3);
    
    // VERTS :  TBO model matrix makes the trick with centroid !
    ////////////////////////////////////////////////////////////////////////////
    
    // vertexsOriginal !! 7 vertexs x hexagon :: EACH HEXAGON SITS in it's initial position
    vertexsOriginals[0] = ofVec3f(centroid);
    
    // vertexsTransformed !! 7 vertexs x hexagon :: EACH HEXAGON IS MOVED TO ORIGIN !
    // the center [+0] need to be transported to Origin (0,0,0).
    // we keep this translation with the hexagonsCentroids array. (that will be passed to the shader as a model matrix)
    vertexs[0] = ofVec3f(0.0,0.0,0.0);
    vertexsOriginals[0] = centroid;
    
    // for each vertexs on the path ... add it to the vertexs vectors
    for(int k=0;k<vecOrdered.size();k++)
    {
        vertexs[k+1] = vecOrdered[k] - centroid;
        vertexsOriginals[k+1] = vecOrdered[k];
        
    }
    

    /////////////////////////////
    // PREPARE COLOR
    /////////////////////////////
    
    for(int j=0;j<vertexs.size();j++)
    {
        float factor = float(j) / float(vertexs.size());
        ofFloatColor color = ofFloatColor(1.0,0.5,0.0,1.0);
        colors[j] = color;
    }

    /////////////////////////////
    // PREPARE TEXTURE COORDS
    /////////////////////////////
    
    // TEXCOORDS 0 (0...1200 , 0...1200)
    // this draws full texture over svg
    
    for(int i=0;i<vertexs.size();i++)
    {
        // for each vertex we need to create a ofVec2f
        // normal
        texCoordinates[i] = ofVec2f(vertexsOriginals[i].x/compositionResolution.x,vertexsOriginals[i].y/compositionResolution.y);
        // all hexagon vertices get color from the centroid
        texCoordinates[i] = ofVec2f(centroid.x/compositionResolution.x,centroid.y/compositionResolution.y);
    }

    ////////////////////
    /// PREPARE FACES
    ////////////////////
    
    int numFacesInHexagon =numVertexsOneHexagon;
    for(int i=0;i<numFacesInHexagon;i++)
    {
        faces[(i*3)+0] = 0;
        faces[(i*3)+1] = (i+1);
        
        // if it's the last face of an hexagon we need to close the hexagon so that the last vertex of the last triangle = the second vertex (first is center) of the first triangle
        if(i==5)
        {
            faces[(i*3)+2] = 1;
        }
        else
        {
            faces[(i*3)+2] = (i)+2;
        }
        
    }
    
}

//--------------------------------------------------------------
vector<ofPoint> pmHexagonGuell::reverseVerticesInVector(vector<ofPoint> _v)
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
vector<ofPoint> pmHexagonGuell::orderVerticesOfHexagonBasedOnDistanceToOrigin(vector<ofPoint> _v)
{
    /// INNER ORDERING : the first vertex of each hexagon has to be the closest to the origin (origin = resolution / 2)
    //get the closest vertex to origin and it's index;
    float minDistToOrigin = 100000;
    int minDistToOrigin_index = -1;
    for (int k=0;k<_v.size();k++)
    {
        if((centerOrigin.distance(_v[k])) < minDistToOrigin)
        {
            minDistToOrigin = centerOrigin.distance(_v[k]);
            minDistToOrigin_index = k;
        }
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

