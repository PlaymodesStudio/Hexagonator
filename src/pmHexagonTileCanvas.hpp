//
//  pmHexagonTileCanvas.hpp
//  Hexagonator
//
//  Created by iClon Media  on 17/03/17.
//
//

#ifndef pmHexagonTileCanvas_hpp
#define pmHexagonTileCanvas_hpp

#include <stdio.h>
#include "pmHexagonTile.hpp"
#include "ofMain.h"

typedef struct
{
    int ring;
    int pathId;
    bool available;
    
} tileInfo;

class pmHexagonTileCanvas
{
    public :
    
    pmHexagonTileCanvas();
    
    static vector<pmHexagonTile> searchForStartingOn(vector<pmHexagonTile> inTiles , int startingAtSide);
    static void searchForAvailableHexagons(int id, int ring);
    
    private :
    
};

#endif /* pmHexagonTileCanvas_hpp */
