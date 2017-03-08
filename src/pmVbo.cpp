//
//  pmVBO.cpp
//  guellGenerator
//
//  Created by emaduell on 11/11/16.
//
//

#include "pmVBO.hpp"
int PMVBO_MAX_VECTORS = 8 ;

//--------------------------------------------------------------
pmVbo::pmVbo()
{
    currentVboVerts = 0 ;
    currentVboFaces = 0;
    currentVboColors = 0;
    currentVboTexCoords = 0;
    
    // allocate vectors of vectors . By now we set a maximum of 8 vector positions for each data type
    vecVboVerts.resize(PMVBO_MAX_VECTORS);
    vecVboFaces.resize(PMVBO_MAX_VECTORS);
    vecVboColors.resize(PMVBO_MAX_VECTORS);
    vecVboTexCoords.resize(PMVBO_MAX_VECTORS);
    
    useTexture = true;
    drawMode = TRIANGLES;
}
//--------------------------------------------------------------
void pmVbo::setup(int _numElements,int _vertModulo)
{
    numElements = _numElements;
    vertModulo = _vertModulo;
    
    int numVerts = vertModulo*numElements;
    
    for(int i=0;i<PMVBO_MAX_VECTORS;i++)
    {
        vecVboVerts[i].resize(numVerts,ofVec3f());
        vecVboTexCoords[i].resize(numVerts,ofVec2f());
        vecVboColors[i].resize(numVerts,ofFloatColor());
        vecVboFaces[i].resize(numVerts*3,ofIndexType());

    }
        
    cout << "pmVbo :: setup with " << PMVBO_MAX_VECTORS << " vectors<> on each data buffer. (vertxs,texCoords,colors,faces)" << endl;
    cout << "pmVbo :: numVerts = " << numVerts << " numFaces(v*3) = " << numVerts*3 << " vert.modulo = " << vertModulo  << endl;
}

//--------------------------------------------------------------
void pmVbo::setVertData(vector<ofVec3f> _v, int _index)
{
    vecVboVerts[_index] = _v;
    
    vbo.setVertexData(vecVboVerts[_index].data(),vecVboVerts[_index].size(),GL_DYNAMIC_DRAW);
    currentVboVerts = _index;

    cout << "pmVbo : setting vert data for num vertices : " << _v.size() << " = " << vecVboVerts[_index].size() << endl;
}

//--------------------------------------------------------------
void pmVbo::updateVertData(vector<ofVec3f> _v, int _index)
{
    vecVboVerts[_index] = _v;
    vbo.updateVertexData(vecVboVerts[_index].data(),vecVboVerts[_index].size());
    currentVboVerts = _index;
    
}
//--------------------------------------------------------------
void pmVbo::setColorData(vector<ofFloatColor> _vc, int _index)
{
    vecVboColors[_index] = _vc;
    vbo.setColorData(vecVboColors[_index].data(), vecVboColors[_index].size(), GL_DYNAMIC_DRAW);
}

//--------------------------------------------------------------
void pmVbo::setFacesData(vector<ofIndexType> _vf, int _index)
{
    vecVboFaces[_index] = _vf;
    
    vbo.setIndexData(vecVboFaces[_index].data(),vecVboFaces[_index].size(), GL_STATIC_DRAW);
}

//--------------------------------------------------------------
void pmVbo::setTexCoordsData(vector<ofVec2f> _v, int _index)
{
    vecVboTexCoords[_index] = _v;
    
    vbo.setTexCoordData(vecVboTexCoords[_index].data(), vecVboTexCoords[_index].size(),GL_STATIC_DRAW);
}

//--------------------------------------------------------------
void pmVbo::setTextureReference(ofTexture _tex)
{
    cout << _tex.getWidth() << " ... " << _tex.getHeight() << endl;
    texture.allocate(_tex.getWidth(),_tex.getHeight(),GL_RGB);
    texture = _tex;
}

//--------------------------------------------------------------
void pmVbo::draw(int _primitive)
{
//    drawDebug(currentVboVerts);
    
    if(useTexture)
    {
        // BIND TEXTURE
        vbo.bind();
        //texture.bind();
    }
    
    switch(drawMode)
    {
        case CONTOURS :
        {
            ofSetLineWidth(5.0);
            vbo.bind();
            glEnable(GL_PRIMITIVE_RESTART);
            glPrimitiveRestartIndex(65535);
            
            int num = vecVboVerts[currentVboVerts].size();
            ofSetLineWidth(5.0);
            vbo.drawElements(_primitive,numElements*7);
            
            glDisable(GL_PRIMITIVE_RESTART);
            vbo.unbind();
            
            break;
        }
        case TRIANGLES :
        {
            int numVertexsPerOneFace = 18;
            vbo.drawElements(_primitive,(vecVboVerts[currentVboVerts].size()/vertModulo)*numVertexsPerOneFace );
            break;
        }
        case QUADS :
        {
            int numVertexPerQuad = 4;
//            vbo.drawElements(_primitive,(vecVboVerts[currentVboVerts].size() * numVertexPerQuad) );
            vbo.drawElements(_primitive,4 * numVertexPerQuad );
            cout << "pmVbo drawing Quads : " << _primitive <<  endl;
            break;
        }

    }
    

    if(useTexture)
    {
        // UNBIND TEXTURE
        //texture.unbind();
        vbo.unbind();
    }

    
//    // DRAW VERTEX COORDINATES
//    ofSetColor(255,255,0);
//    for(int i=0;i<vecVboVerts[currentVboVerts].size();i++)
//    {
//        if(i==0)
//        {
//            ofDrawBitmapString(ofToString(i) +" : " + ofToString(vecVboVerts[currentVboVerts][i]),vecVboVerts[currentVboVerts][i].x, vecVboVerts[currentVboVerts][i].y);
//            //ofDrawBitmapString(ofToString(vboTexCoord[i]), vboVert[i].x, vboVert[i].y+20);
//        }
//    }

}

//--------------------------------------------------------------
ofPoint pmVbo::projectPointToLine(ofPoint Point,ofPoint LineStart,ofPoint LineEnd)
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
        //cout << " closest point does not fall within the line segment !! " << endl;
    }
    
    Intersection.x = LineStart.x + U * ( LineEnd.x - LineStart.x );
    Intersection.y = LineStart.y + U * ( LineEnd.y - LineStart.y );
    Intersection.z = LineStart.z + U * ( LineEnd.z - LineStart.z );
    
    //    *Distance = Magnitude( Point, &Intersection );
    
    return Intersection;
}

//--------------------------------------------------------------
void pmVbo::drawDebug(int _index)
{
    cout << "pmVbo drawing debug for index : " << _index << endl;
    // VERTEXS
    for(int i=0;i<vecVboVerts[_index].size();i++)
    {
        cout << " Vertex [" <<_index <<" , "<<i<<"] >  " << vecVboVerts[_index][i]<< endl;
    }
    // COLORS
    for(int i=0;i<vecVboColors[_index].size();i++)
    {
        cout << " Colors [" <<_index <<" , "<<i<<"] >  " << vecVboColors[_index][i] << endl;
    }
    // FACES
    for(int i=0;i<vecVboFaces[_index].size()/3;i++)
    {
        cout << " Faces [" <<_index <<" , "<<i<<"] >  " << vecVboFaces[_index][i*3] << " " <<  vecVboFaces[_index][(i*3)+1] << " " << vecVboFaces[_index][(i*3)+2] << endl;
    }
    // TEX COORDS
    for(int i=0;i<vecVboTexCoords[_index].size();i++)
    {
        cout << " TexCoords [" <<_index <<" , "<<i<<"] >  " << vecVboTexCoords[_index][i] << endl;
    }
    
}
//--------------------------------------------------------------
void pmVbo::setTexCoordsIndex(int _i)
{
    currentVboTexCoords=_i;
    vbo.updateTexCoordData(vecVboTexCoords[currentVboTexCoords].data(),vecVboVerts[currentVboVerts].size());
}
//--------------------------------------------------------------
void pmVbo::setVertices(int _i)
{
    currentVboVerts = _i;
    vbo.updateVertexData(vecVboVerts[currentVboVerts].data(), vecVboVerts[currentVboVerts].size());
}
