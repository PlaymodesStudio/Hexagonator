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
    connections.clear();
}

//-----------------------------------------------
void pmHexagonTile::addConnection(int i, int j)
{
    tileData t;
    t.startsAt = i;
    t.endsAt = j;
    connections.push_back(t);
}

//-----------------------------------------------
void pmHexagonTile::resetConnections()
{
    connections.clear();
}

////-----------------------------------------------
int pmHexagonTile::howManyConnections()
{
    return connections.size();
}

////-----------------------------------------------
tileData pmHexagonTile::getConnection(int n)
{
    if(n>(connections.size()-1))
    {
        //
    }
    else if(connections.size()!=0)
    {
        cout << "Connections Size : " << connections.size() << endl;
        return connections[n];
    }
}
////-----------------------------------------------
pmHexagonTile pmHexagonTile::mergeTileData(pmHexagonTile t)
{
    pmHexagonTile tileResult;
    
    tileResult = t;
    for(int i=0;i<this->howManyConnections();i++)
    {
        cout <<" merging..." << this->howManyConnections() << endl;
        int m=connections[i].startsAt;
        int n=connections[i].endsAt;
        tileResult.addConnection(m, n);
    }
    return tileResult;
}

////-----------------------------------------------
//bool pmHexagonTile::hasConnectionOnSide(int side)
//{
//    bool hasConnection=false;
//    
//    // check for connections of level 1 jump
//    if(connections[0][6]==false)
//    {
//        for(int i=0;i<6;i++)
//        {
//            // check for connection starting from "int side"
//            if(connections[0][i]==true)
//            {
//                if(i==side)
//                {
//                    hasConnection=true;
//                }
//            }
//            // check for connections finishing at "int side"
//            if(i>1)
//            {
//                if(connections[0][i-1]==true)
//                {
//                    hasConnection = true;
//                }
//            }
//            else if(i==1)
//            {
//                // last case is where the connection from side 5 goes to side 0
//                if(connections[0][5]==true)
//                {
//                    hasConnection = true;
//                }
//
//            }
//        }
//    }
//    else if(connections[0][6]==true)
//    {
//        // it's marked as no connections of level 2 jumps
//    }
//    
//    
//    
//    
//}
