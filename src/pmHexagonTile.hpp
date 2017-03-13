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


class pmHexagonTile
{
    public :
    
    pmHexagonTile();
    
    bool                    hasConnectionOnSide(int s);
    void                    resetConnections();
    void                    addConnection(int i, int j);
    
    private :
    
    vector<vector<bool>>    connections;
    vector<bool>            connectedSides;
};

#endif /* pmHexagonTile_hpp */
