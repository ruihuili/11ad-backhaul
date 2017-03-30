/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NICOLO' FACCHI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Rui Li <lirui628@gmail.com>
 */

#include "building-block.h"

#include "ns3/mobility-model.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BuildingBlock");

NS_OBJECT_ENSURE_REGISTERED (BuildingBlock);

BuildingBlock::BuildingBlock ()
{
}

BuildingBlock::~BuildingBlock ()
{
}

void
BuildingBlock::AddBuilding (Ptr<Building> building)
{
    m_buildings.push_back(building);
}
    
bool
BuildingBlock::GetIfLos (Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
        for (std::vector< Ptr<Building> >::const_iterator it = m_buildings.begin(); it!= m_buildings.end();++it)
        {
            Box box = (*it) ->GetBoundaries();
            if(CheckifIntersect(a, b, box))
            {
                NS_LOG_DEBUG("intersect with building: "<< box.xMin <<" "<< box.xMax <<" "<<box.yMin <<" "<<box.yMax<<" "<<box.zMin <<" "<<box.zMax);
                
                return false;
            }
        }
    return true;

}
    
bool
BuildingBlock::CheckifIntersect(Ptr<MobilityModel> a, Ptr<MobilityModel> b, Box box)
{
    //1.check if all four corners are on the same side of the line
    double ax = a->GetPosition().x;
    double ay = a->GetPosition().y;
    double bx = b->GetPosition().x;
    double by = b->GetPosition().y;
    NS_LOG_DEBUG("line between points ("<< ax <<","<< ay << ") and ("<< bx <<","<< by<<"). Building :"<< box);
    
    //F(x y) = (y2-y1)*x + (x1-x2)*y + (x2*y1-x1*y2)
    //true if above, false if below
    bool xMinyMin = IsPointAboveLine(box.xMin, box.yMin, ax, ay, bx, by);
    bool xMaxyMax = IsPointAboveLine(box.xMax, box.yMax, ax, ay, bx, by);
    bool xMinyMax = IsPointAboveLine(box.xMin, box.yMax, ax, ay, bx, by);
    bool xMaxyMin = IsPointAboveLine(box.xMax, box.yMin, ax, ay, bx, by);

    if (xMinyMin+xMaxyMax+xMinyMax+xMaxyMin == 4 || xMinyMin+xMaxyMax+xMinyMax+xMaxyMin == 0)
    {
        if(xMinyMin+xMaxyMax+xMinyMax+xMaxyMin == 4)
            NS_LOG_DEBUG("Building is above the link");
        else if(xMinyMin+xMaxyMax+xMinyMax+xMaxyMin == 0)
            NS_LOG_DEBUG("Building is below the link");
        return false;
    }
    else
    {
        bool istoRight = (ax > box.xMax) && (bx > box.xMax);
        bool istoLeft = (ax < box.xMin) && (bx < box.xMin);
        bool isAbove = (ay > box.yMax) && (by > box.yMax);
        bool isBelow = (ay < box.yMin) && (by < box.yMin);
        
        if(istoRight||istoLeft||isAbove||isBelow)
        {
            NS_LOG_DEBUG("link is on one side of the building");
            return false;
        }
        else
            return true;
    }
}
  
bool
BuildingBlock::IsPointAboveLine(double x0, double y0, double x1, double y1, double x2, double y2)
{
    //f_xy: line defined by (x1, x2), (y1 ,y2)
    double f_xy = (y1 - y2)/(x1 - x2) * x0 +(x1*y2 - x2*y1)/(x1 - x2);
    
    if (y0 > f_xy)
        return true;
    else
        return false;

}
} // namespace ns3

