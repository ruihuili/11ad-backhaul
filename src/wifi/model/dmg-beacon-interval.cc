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
 * Author: Nicolo' Facchi <nicolo.facchi@gmail.com>
           Rui Li <lirui628@gmail.com>
 */
#include "dmg-beacon-interval.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DmgBeaconInterval");

NS_OBJECT_ENSURE_REGISTERED (DmgBeaconInterval);

DmgServicePeriod::DmgServicePeriod ()
{
}

DmgServicePeriod::~DmgServicePeriod ()
{
}

void
DmgServicePeriod::SetSpStart (Time start)
{
  m_spStart = start;
}

Time
DmgServicePeriod::GetSpStart (void)
{
    return m_spStart;// + m_beamSwitchOverhead;
}

void
DmgServicePeriod::SetSpStop (Time stop)
{
  m_spStop = stop;
}

Time
DmgServicePeriod::GetSpStop (void)
{
  return m_spStop;
}

void
DmgServicePeriod::SetBeamSwitchOverhead (Time beamSwitchOverhead)
{
    m_beamSwitchOverhead = beamSwitchOverhead;
}

void
DmgServicePeriod::SetSpDestination (Mac48Address dest)
{
  m_spDestination = dest;
}
    
Mac48Address
DmgServicePeriod::GetSpDestination (void)
{
  return m_spDestination;
}

void
DmgServicePeriod::SetSpFlowSrcSinkIpv4Address (Ipv4Address src, Ipv4Address sink)
{
  m_spFlowSourceSinkIpv4Address.first = src;
  m_spFlowSourceSinkIpv4Address.second = sink;
}

std::pair <Ipv4Address, Ipv4Address>
DmgServicePeriod::GetSpFlowSourceSinkIpv4Address (void)
{
  return m_spFlowSourceSinkIpv4Address;
}

void
DmgServicePeriod::SetSpDestinationMobility(Ptr<MobilityModel> mob)
{
  m_spDestinationMobility = mob;
}

Ptr<MobilityModel>
DmgServicePeriod::GetSpDestinationMobility (void)
{
  return m_spDestinationMobility;
}

bool
DmgServicePeriod::GetSpIfTx(void)
{
  return m_spTransmitter;
}

void
DmgServicePeriod::SetSpIfTx(bool transmitt)
{
  m_spTransmitter = transmitt;
}


DmgBeaconInterval::DmgBeaconInterval ()
{
  m_dmgAntennaController = 0;
  m_alignAntennaTime = NanoSeconds(0);
  m_initialized = false;
}

DmgBeaconInterval::~DmgBeaconInterval ()
{
}

void
DmgBeaconInterval::SetBiDuration (Time biDur)
{
  m_biDuration = biDur;
}

Time
DmgBeaconInterval::GetBiDuration (void)
{
  return m_biDuration;
}

void
DmgBeaconInterval::SetBeamSwitchOverhead (Time beamSwitchOverhead)
{
    m_beamSwitchOverhead =beamSwitchOverhead;
}

void
DmgBeaconInterval::AddSp (Time start, Time stop,
			  Mac48Address dest, Ipv4Address srcIpv4, Ipv4Address sinkIpv4, Ptr<MobilityModel> mob, bool transmitt )
{
  Ptr<DmgServicePeriod> sp = CreateObject<DmgServicePeriod> ();
  sp->SetSpStart(start);
  sp->SetSpStop(stop);
  sp->SetSpDestination(dest);
  sp->SetSpFlowSrcSinkIpv4Address(srcIpv4, sinkIpv4);
  sp->SetSpDestinationMobility(mob);
  sp->SetSpIfTx(transmitt);
  m_sp.push_back(sp);
}

void
DmgBeaconInterval::EraseSp ()
{
  m_sp.clear();
}

Time
DmgBeaconInterval::GetNextSpStart (void)
{
  Time t;

  NS_ASSERT(m_sp.size() > 0);
  Time now = Simulator::Now();
  Time lastBiStart = (now / m_biDuration) *
				 m_biDuration;

  bool nextSpFound = false;
  do {
    for (uint32_t i = 0; i < m_sp.size(); i++) {
      Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();

      if (now < spStop) {
        t = lastBiStart + m_sp.at(i)->GetSpStart();
        nextSpFound = true;
        break;
      }
    }

    lastBiStart += m_biDuration;
  } while (!nextSpFound);

  return t;
}

Time
DmgBeaconInterval::GetNextSpStop (void)
{
  Time t;

  NS_ASSERT(m_sp.size() > 0);

  Time now = Simulator::Now();
  Time lastBiStart = (now / m_biDuration) *
				 m_biDuration;

  bool nextSpFound = false;
  do {
    for (uint32_t i = 0; i < m_sp.size(); i++) {
      Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();

      if (now < spStop) {
        t = spStop;
	nextSpFound = true;
        break;
      }
    }

    lastBiStart += m_biDuration;
  }while (!nextSpFound);

  return t;
}

//----------------added on 20 Jan
bool
DmgBeaconInterval::GetNextSpIfTx(void)
{
    bool transmitter;
    NS_ASSERT(m_sp.size() > 0);
    
    Time now = Simulator::Now();
    Time lastBiStart = (now / m_biDuration) *
    m_biDuration;
    bool nextSpFound = false;
    do {
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
            if (now < spStop) {
                transmitter = m_sp.at(i)->GetSpIfTx();
                nextSpFound = true;
                break;
            }
        }
        lastBiStart += m_biDuration;
    }while (!nextSpFound);
    
    return transmitter;
}

Time
DmgBeaconInterval::GetNextTxSpStart (void)
{
    bool ifAnyTxSp=false;
    for (uint32_t i = 0; i < m_sp.size(); i++) {
        if (m_sp.at(i)->GetSpIfTx()){
            ifAnyTxSp=true;
           }
    }
    if(!ifAnyTxSp){
        return (Seconds(65535)+Simulator::Now());
    }
    else{
        
    Time t;
    
    NS_ASSERT(m_sp.size() > 0);
    
    Time now = Simulator::Now();
    Time lastBiStart = (now / m_biDuration) *
    m_biDuration;
    bool nextTxSpFound = false;
    do {
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            NS_LOG_DEBUG("m_sp"<<i);
            Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
            NS_LOG_DEBUG(now << "(now) < spstop? "<< spStop << " tx? " <<m_sp.at(i)->GetSpIfTx());
            if ((now < spStop)&&(m_sp.at(i)->GetSpIfTx())) {
                t = lastBiStart + m_sp.at(i)->GetSpStart();
                NS_LOG_DEBUG("lastBiStart "<<lastBiStart<< " Bi SpStart "<< m_sp.at(i)->GetSpStart() << " Bi SpStop "<< m_sp.at(i)->GetSpStop());
                NS_LOG_DEBUG("Time start "<< t << " Time stop "<< lastBiStart + m_sp.at(i)->GetSpStop());
                nextTxSpFound = true;
                break;
            }
        }
        
        lastBiStart += m_biDuration;
    } while (!nextTxSpFound);
    
    return t;
    }
}

Time
DmgBeaconInterval::GetNextTxSpStop (void)
{
    bool ifAnyTxSp=false;
    for (uint32_t i = 0; i < m_sp.size(); i++) {
        if (m_sp.at(i)->GetSpIfTx()){
            ifAnyTxSp=true;
        }
    }
    if(!ifAnyTxSp){
        return (Seconds(65535)+Simulator::Now());
    }
    else{
        
    Time t;
    
    NS_ASSERT(m_sp.size() > 0);
    
    Time now = Simulator::Now();
    Time lastBiStart = (now / m_biDuration) *
    m_biDuration;
    
    bool nextTxSpFound = false;
    do {
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
            if ((now < spStop)&&(m_sp.at(i)->GetSpIfTx()))  {
                t = spStop;
                nextTxSpFound = true;
                break;
            }
        }
        
        lastBiStart += m_biDuration;
    }while (!nextTxSpFound);
    
    return t;
    }
}

    Mac48Address
    DmgBeaconInterval::GetNextTxSpDestination (void)
    {
        bool ifAnyTxSp=false;
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            if (m_sp.at(i)->GetSpIfTx()){
                ifAnyTxSp=true;
            }
        }
        if(ifAnyTxSp){
        Mac48Address mac;
        
        NS_ASSERT(m_sp.size() > 0);
        
        Time now = Simulator::Now();
        Time lastBiStart = (now / m_biDuration) *
        m_biDuration;
        
        bool nextSpFound = false;
        do {
            for (uint32_t i = 0; i < m_sp.size(); i++) {
                Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
                
                if ((now < spStop)&&(m_sp.at(i)->GetSpIfTx())) {
                    
                    mac = m_sp.at(i)->GetSpDestination();
                    nextSpFound = true;
                    break;
                }
                
            }
            
            lastBiStart += m_biDuration;
        }while (!nextSpFound);
        
        return mac;
        }
    }

    std::pair<Ipv4Address, Ipv4Address> 
    DmgBeaconInterval::GetNextTxSpSrcSinkIpv4Address (void)
    {
        bool ifAnyTxSp=false;
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            if (m_sp.at(i)->GetSpIfTx()){
                ifAnyTxSp=true;
            }
        }
        if(ifAnyTxSp){
        std::pair <Ipv4Address,Ipv4Address> ipv4;
        
        NS_ASSERT(m_sp.size() > 0);
        
        Time now = Simulator::Now();
        Time lastBiStart = (now / m_biDuration) *
        m_biDuration;
        
        bool nextSpFound = false;
        do {
            for (uint32_t i = 0; i < m_sp.size(); i++) {
                Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
                
                if ((now < spStop)&&(m_sp.at(i)->GetSpIfTx())) {
                    
                    ipv4 = m_sp.at(i)->GetSpFlowSourceSinkIpv4Address();
                    nextSpFound = true;
                    break;
                }
                
            }
            
            lastBiStart += m_biDuration;
        }while (!nextSpFound);
        return ipv4;
        }
    }
    
    Ptr<MobilityModel>
    DmgBeaconInterval::GetNextTxSpDestinationMobility (void)
    {
        bool ifAnyTxSp=false;
        for (uint32_t i = 0; i < m_sp.size(); i++) {
            if (m_sp.at(i)->GetSpIfTx()){
                ifAnyTxSp=true;
            }
        }
        if(ifAnyTxSp){

        Ptr<MobilityModel> mob;
        
        NS_ASSERT(m_sp.size() > 0);
        
        Time now = Simulator::Now();
        Time lastBiStart = (now / m_biDuration) *
        m_biDuration;
        
        bool nextSpFound = false;
        do {
            for (uint32_t i = 0; i < m_sp.size(); i++) {
                Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
                
                if ((now < spStop)&&(m_sp.at(i)->GetSpIfTx())) {
                    mob = m_sp.at(i)->GetSpDestinationMobility();
                    nextSpFound = true;
                    break;
                }
            }
            
            lastBiStart += m_biDuration;
        }while (!nextSpFound);
        
        return mob;
        }
    }

// -----------------------------------

Mac48Address
DmgBeaconInterval::GetNextSpDestination (void)
{
  Mac48Address mac;

  NS_ASSERT(m_sp.size() > 0);
    
  Time now = Simulator::Now();
  Time lastBiStart = (now / m_biDuration) *
				 m_biDuration;

  bool nextSpFound = false;
  do {
    for (uint32_t i = 0; i < m_sp.size(); i++) {
      Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();
        
      if (now < spStop) {

        mac = m_sp.at(i)->GetSpDestination();
	    nextSpFound = true;
        break;
      }

    }

    lastBiStart += m_biDuration;
  }while (!nextSpFound);
  
  return mac;
}

Ptr<MobilityModel>
DmgBeaconInterval::GetNextSpDestinationMobility (void)
{
  Ptr<MobilityModel> mob;

  NS_ASSERT(m_sp.size() > 0);

  Time now = Simulator::Now();
  Time lastBiStart = (now / m_biDuration) *
				 m_biDuration;

  bool nextSpFound = false;
  do {
    for (uint32_t i = 0; i < m_sp.size(); i++) {
      Time spStop = lastBiStart + m_sp.at(i)->GetSpStop();

      if (now < spStop) {
        mob = m_sp.at(i)->GetSpDestinationMobility();
	nextSpFound = true;
        break;
      }
    }

    lastBiStart += m_biDuration;
  }while (!nextSpFound);

  return mob;
}

void
DmgBeaconInterval::SetDmgAntennaController (Ptr<DmgAntennaController> dmgBi)
{
  m_dmgAntennaController = dmgBi;
}

Ptr<DmgAntennaController>
DmgBeaconInterval::GetDmgAntennaController (void)
{
  return m_dmgAntennaController;
}

void
DmgBeaconInterval::AlignAntenna()
{
    NS_LOG_FUNCTION(this);
  Vector targetPos = m_sp.at(m_lastAlignAntennaSpIndex)->
	  GetSpDestinationMobility()->GetPosition();
    
  m_dmgAntennaController->PointAntenna(targetPos);

  m_lastAlignAntennaSpIndex++;

  if (m_lastAlignAntennaSpIndex == m_sp.size()) {
    m_lastAlignAntennaSpIndex = 0;

    Time now = Simulator::Now();
    Time nextBiStart = ((now / m_biDuration) + 1) *
				 m_biDuration;

    Simulator::Schedule (nextBiStart - now,
			&DmgBeaconInterval::ScheduleNextAntennaAlignment, this);

  } else {
    ScheduleNextAntennaAlignment();
  }
}

void
DmgBeaconInterval::ScheduleNextAntennaAlignment ()
{
  if (m_sp.size() == 0) {
    return;
  }

  if (!m_initialized) {//This happens only at the first call
    m_lastAlignAntennaSpIndex = 0;
    m_initialized = true;
  }

  Time now = Simulator::Now();
  Time lastBiStart = (now / m_biDuration + 1) *
				 m_biDuration;
    
  m_alignAntennaTime = lastBiStart + m_sp.at(m_lastAlignAntennaSpIndex)->
	  GetSpStart();

  m_alignAntenna = Simulator::Schedule (m_alignAntennaTime - now,
					 &DmgBeaconInterval::AlignAntenna, this);

}

std::vector<Ptr<DmgServicePeriod> >
DmgBeaconInterval::GetSps(void)
{
  return m_sp;
}

} // namespace ns3

