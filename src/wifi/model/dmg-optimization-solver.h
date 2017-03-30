/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NICOLO FACCHI
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
 * Author: Nicolo' Facchi <nicolo.facchi@gmail.com>
 */
#ifndef DMG_OPTIMIZATION_SOLVER_H
#define DMG_OPTIMIZATION_SOLVER_H

#include "ns3/object.h"

namespace ns3 {

/* Dummy and unused class in the current implementation.
 * In future implementation this calss will be used by the DmgAlmightyController
 * to solve optimization problems for network resources optimization */
class DmgOptimizationSolver : public Object
{
public:
  DmgOptimizationSolver ();

  virtual ~DmgOptimizationSolver ();


 };

} // namespace ns3

#endif /* DMG_OPTIMIZATION_SOLVER_H */
