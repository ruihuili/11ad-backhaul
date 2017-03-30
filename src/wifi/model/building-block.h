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
#ifndef BUILDING_BLOCK_H
#define BUILDING_BLOCK_H

#include "ns3/object.h"
#include "ns3/vector.h"
#include <ns3/buildings-module.h>

namespace ns3 {

/* 
 * Work with FriisLosPropagationLoss model. This BuildingBlock model helps to determine whether the link between two stations are blocked.
 */
class BuildingBlock : public Object
{
public:
  BuildingBlock ();

  virtual ~BuildingBlock ();
  
  /* Add a building to record*/
  void AddBuilding (Ptr<Building> building);
    
  /* Go through building list, and check for each building, if the link between 2 nodes,
   * with mobility model a and b, respectively, is blocked by the building or not.
   * Then set the corresponding los info (by calling function SetIfLos()) in class FriisLosPropagation.
   * Attributes: Ptr to the mobility models of the 2 stations
   * Return: 1 if the link is LOS, 0 if NLOS
   */
  bool GetIfLos (Ptr<MobilityModel> a, Ptr<MobilityModel> b);
    
  /* Check if a building's boundary (defined as Box) intersects with the link between node a and b,
   * called by GetIfLos()
   * Return: 1 if intersects, 0 if not intersect
   */
  bool CheckifIntersect(Ptr<MobilityModel> a, Ptr<MobilityModel> b, Box box);
  /* Examine if a point (x0, y0) is above a line segment between (x1,y1) and (x2,y2),
   * called by CheckifIntersect()
   * Return: 1 if the point is above line segment, 0 otherwise
   */
  bool IsPointAboveLine(double x0, double y0, double x1, double y1, double x2, double y2);
    
  /*list of buildings*/
  std::vector < Ptr<Building> >  m_buildings;
private:


};

} // namespace ns3

#endif /* Building Block */
