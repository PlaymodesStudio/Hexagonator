//
//  pmHexagonTile.cpp
//  Hexagonator
//
//  Created by Eloi Maduell on 13/03/2017.
//
//

#include "pmHexagonTile.hpp"

//-----------------------------------------------
pmHexagonTile::pmHexagonTile()
{
    connectedSides.resize(6);
    
    connections.resize(3);
    for(int i=0;i<connections.size();i++)
    {
        connections[i].resize(7);
    }
    
}

//-----------------------------------------------
void pmHexagonTile::addConnection(int i, int j)
{
    connections[i][j] = true;
}

//-----------------------------------------------
void pmHexagonTile::resetConnections()
{
    for(int i=0;i<connections.size();i++)
    {
        for(int j=0;j<connections[i].size();j++)
        {
            connections[i][j] = false;
        }
    }
}
//-----------------------------------------------
bool pmHexagonTile::hasConnectionOnSide(int side)
{
    bool hasConnection=false;
    
    // check for connections of level 1 jump
    if(connections[0][6]==false)
    {
        for(int i=0;i<6;i++)
        {
            // check for connection starting from "int side"
            if(connections[0][i]==true)
            {
                if(i==side)
                {
                    hasConnection=true;
                }
            }
            // check for connections finishing at "int side"
            if(i>1)
            {
                if(connections[0][i-1]==true)
                {
                    hasConnection = true;
                }
            }
            else if(i==1)
            {
                // last case is where the connection from side 5 goes to side 0
                if(connections[0][5]==true)
                {
                    hasConnection = true;
                }

            }
        }
    }
    else if(connections[0][6]==true)
    {
        // it's marked as no connections of level 2 jumps
    }
    
    
    
    
}
