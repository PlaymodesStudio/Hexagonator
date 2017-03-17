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


class pmHexagonTileCanvas
{
    public :
    
    pmHexagonTileCanvas();
    
    static vector<pmHexagonTile> searchForStartingOn(vector<pmHexagonTile> inTiles , int startingAtSide);
    
    private :
    
};

#endif /* pmHexagonTileCanvas_hpp */
