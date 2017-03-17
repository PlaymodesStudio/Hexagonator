//
//  pmHexagonTileCanvas.cpp
//  Hexagonator
//
//  Created by iClon Media  on 17/03/17.
//
//

#include "pmHexagonTileCanvas.hpp"


pmHexagonTileCanvas::pmHexagonTileCanvas()
{
    
}


vector<pmHexagonTile> pmHexagonTileCanvas::searchForStartingOn(vector<pmHexagonTile> inTiles , int startingAtSide)
{
    vector<pmHexagonTile> resultTiles;
    
    for(int i=0;i<inTiles.size();i++)
    {
        // by now just process those with just 1 connection
        if(inTiles[i].howManyConnections()==1)
        {
            // if we have the right number of connections ... continue
            vector<tileData> tileData = inTiles[i].getConnections();
            
            if((tileData[0].startsAt==startingAtSide)||((tileData[0].endsAt)==startingAtSide))
            {
                resultTiles.push_back(inTiles[i]);
                cout << "TileCanvas :: adding " << i << " to the list of tiles that start at : " << startingAtSide << endl;
            }
            
        }
    }
    
    return resultTiles;
}

