//
//  pmHexagonTile.hpp
//  Hexagonator
//
//  Created by Eloi Maduell on 13/03/2017.
//
//

#ifndef pmHexagonTile_hpp
#define pmHexagonTile_hpp

#include <stdio.h>
#include "ofMain.h"

typedef struct
{
    int startsAt;
    int endsAt;
}tileData;

class pmHexagonTile
{
    public :
    
    pmHexagonTile();
    
    int                     howManyConnections();
    //bool                    hasConnectionOnSide(int s);
    void                    resetConnections();
    void                    addConnection(int i, int j);
    tileData                getConnection(int n);
    pmHexagonTile           mergeTileData(pmHexagonTile t);
    vector<tileData>        getConnections(){return connections;};
    
    private :
    
    vector<tileData>        connections;
    vector<bool>            connectedSides;
};

#endif /* pmHexagonTile_hpp */
