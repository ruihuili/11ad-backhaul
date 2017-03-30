/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 */

#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/llc-snap-header.h"

#include "wifi-mac-queue.h"
#include "qos-blocked-destinations.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiMacQueue");
NS_OBJECT_ENSURE_REGISTERED (WifiMacQueue);

WifiMacQueue::Item::Item (Ptr<const Packet> packet,
                          const WifiMacHeader &hdr,
                          Time tstamp)
  : packet (packet),
    hdr (hdr),
    tstamp (tstamp)
{
}

TypeId
WifiMacQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiMacQueue")
    .SetParent<Object> ()
    .AddConstructor<WifiMacQueue> ()
    .AddAttribute ("MaxPacketNumber", "If a packet arrives when there are already this number of packets, it is dropped.",
                   UintegerValue (400),
                   MakeUintegerAccessor (&WifiMacQueue::m_maxSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxDelay", "If a packet stays longer than this delay in the queue, it is dropped.",
                   TimeValue (Seconds (10.0)),
                   MakeTimeAccessor (&WifiMacQueue::m_maxDelay),
                   MakeTimeChecker ())
  ;
  return tid;
}

WifiMacQueue::WifiMacQueue ()
  : m_size (0)
{
  m_shallowBufTimerOn = false; // if the shallow buffer has been recorded 
  m_shallowBufStart =  Simulator::Now();   // The time at which the buffer has been reportedly shallow 
  m_shallowBufDurThresh = Seconds(1);
}

WifiMacQueue::~WifiMacQueue ()
{
  Flush ();
}

void
WifiMacQueue::SetMaxSize (uint32_t maxSize)
{
  m_maxSize = maxSize;
}

void
WifiMacQueue::SetMaxDelay (Time delay)
{
  m_maxDelay = delay;
}

uint32_t
WifiMacQueue::GetMaxSize (void) const
{
  return m_maxSize;
}

Time
WifiMacQueue::GetMaxDelay (void) const
{
  return m_maxDelay;
}

void
WifiMacQueue::DynamicQueueSize (void)
{
 if (m_size >= m_maxSize*0.95)
    {
      NS_LOG_UNCOND(this<< "Enlarging queue size to "<< m_maxSize * 2);        
      SetMaxSize(m_maxSize * 2);
    }
  else if (m_size < m_maxSize*0.4)//what is the best percentage of queue usage?
    {
       if(!m_shallowBufTimerOn)
        {
                m_shallowBufTimerOn = true;
                m_shallowBufStart = Simulator::Now();
         } 
       else if(Simulator::Now() - m_shallowBufStart >= m_shallowBufDurThresh)
        {
              NS_LOG_UNCOND(this<< "Shriking queue size to "<< std::max((floor(m_maxSize * 0.5)), 100.0));        
              SetMaxSize(std::max((floor(m_maxSize * 0.5)), 100.0));
              m_shallowBufTimerOn = false;
        }

    }
  else if (m_size >= m_maxSize*0.4)
    {
        m_shallowBufTimerOn = false;
    }
}

void
WifiMacQueue::Enqueue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  Cleanup ();

  DynamicQueueSize();

  //shouldn't reach here with DynamicQueueSize
  if (m_size == m_maxSize)
    {
      NS_LOG_DEBUG(this<< "QUEUE REACHES THE MAX SIZE!");
      return;
    }
  Time now = Simulator::Now ();
  m_queue.push_back (Item (packet, hdr, now));
  m_size++;

}

void
WifiMacQueue::Cleanup (void)
{
  if (m_queue.empty ())
    {
      return;
    }

  Time now = Simulator::Now ();
  uint32_t n = 0;
  for (PacketQueueI i = m_queue.begin (); i != m_queue.end ();)
    {
      if (i->tstamp + m_maxDelay > now)
        {
          i++;
        }
      else
        {
          i = m_queue.erase (i);
          n++;
        }
    }
  m_size -= n;
}

Ptr<const Packet>
WifiMacQueue::Dequeue (WifiMacHeader *hdr)
{
  Cleanup ();
  if (!m_queue.empty ())
    {
      Item i = m_queue.front ();
      m_queue.pop_front ();
      m_size--;
      *hdr = i.hdr;
      return i.packet;
    }
  return 0;
}

Ptr<const Packet>
WifiMacQueue::Peek (WifiMacHeader *hdr)
{
  Cleanup ();
  if (!m_queue.empty ())
    {
      Item i = m_queue.front ();
      *hdr = i.hdr;
      return i.packet;
    }
  return 0;
}

Ptr<const Packet>
WifiMacQueue::DequeueByTidAndAddress (WifiMacHeader *hdr, uint8_t tid,
                                      WifiMacHeader::AddressType type, 
                                      Mac48Address nextHopMacAddr,
                                      std::pair <Ipv4Address,Ipv4Address> srcsinkIp)
{
  NS_LOG_FUNCTION (this);
  Cleanup ();
  Ptr<const Packet> packet = 0;
  if (!m_queue.empty ())
    {
      PacketQueueI it;
      for (it = m_queue.begin (); it != m_queue.end (); ++it)
        {
          if (it->hdr.IsQosData ())
            {
              if (GetNextHopMacAddressForPacket (type, it) == nextHopMacAddr
                  && it->hdr.GetQosTid () == tid && GetSrcDestinIpv4AddressForPacket(it) == srcsinkIp )
                {
                  packet = it->packet;
                  *hdr = it->hdr;
                  m_queue.erase (it);
                  m_size--;
                  break;
                }
            }
        }
    }
  return packet;
}

Ptr<const Packet>
WifiMacQueue::PeekByTidAndAddress (WifiMacHeader *hdr, uint8_t tid,
                                   WifiMacHeader::AddressType type, 
                                   Mac48Address nextHopMacAddr, 
                                   std::pair <Ipv4Address,Ipv4Address> srcsinkIp,
                                   Time *timestamp)
{
  NS_LOG_FUNCTION (this);
  Cleanup ();
  if (!m_queue.empty ())
    {
      PacketQueueI it;
      for (it = m_queue.begin (); it != m_queue.end (); ++it)
        {
          if (it->hdr.IsQosData ())
            {
              if (GetNextHopMacAddressForPacket (type, it) == nextHopMacAddr
                  && it->hdr.GetQosTid () == tid  && GetSrcDestinIpv4AddressForPacket(it) == srcsinkIp )
                {
                  *hdr = it->hdr;
                  *timestamp=it->tstamp;
                  return it->packet;
                }
            }
        }
    }
  return 0;
}
 
    Ptr<const Packet>
    WifiMacQueue::DequeueByAddress (WifiMacHeader *hdr,
                                        WifiMacHeader::AddressType type, 
                                        Mac48Address nextHopMacAddr,
                                        std::pair <Ipv4Address,Ipv4Address> srcsinkIp)
    {
        NS_LOG_FUNCTION (this);
        Cleanup ();
        Ptr<const Packet> packet = 0;
        if (!m_queue.empty ())
        {
            PacketQueueI it;
            for (it = m_queue.begin (); it != m_queue.end (); ++it)
            {
                if (it->hdr.IsQosData ())
                {
                    if (GetNextHopMacAddressForPacket (type, it) == nextHopMacAddr  && GetSrcDestinIpv4AddressForPacket(it) == srcsinkIp )
                    {
                        packet = it->packet;
                        *hdr = it->hdr;
                        m_queue.erase (it);
                        m_size--;
                        break;
                    }
                }
            }
        }
        return packet;
    }
    
    Ptr<const Packet>
    WifiMacQueue::PeekByAddress (WifiMacHeader *hdr,
                                        WifiMacHeader::AddressType type, 
                                        Mac48Address nextHopMacAddr,
                                        std::pair <Ipv4Address,Ipv4Address> srcsinkIp,
                                        Time *timestamp)
    {
        NS_LOG_FUNCTION (this << "peeking for packets to"<< nextHopMacAddr<<"src addr "<< srcsinkIp.first << " sink Ip Addr"<< srcsinkIp.second);
        Cleanup ();
        if (!m_queue.empty ())
        {
            PacketQueueI it;
            for (it = m_queue.begin (); it != m_queue.end (); ++it)
            {
                if (it->hdr.IsQosData ())
                {
                    NS_LOG_DEBUG ("TYPE: "<<type);
                    if (GetNextHopMacAddressForPacket (type, it) == nextHopMacAddr && GetSrcDestinIpv4AddressForPacket(it) == srcsinkIp )
                    {
                        *hdr = it->hdr;
                        *timestamp=it->tstamp;
                        NS_LOG_DEBUG("peek packet to " << nextHopMacAddr);
                        return it->packet;
                    }
                }
            }
        }
        NS_LOG_DEBUG("Return 0");
        return 0;
    }


bool
WifiMacQueue::IsEmpty (void)
{
  Cleanup ();
  return m_queue.empty ();
}

uint32_t
WifiMacQueue::GetSize (void)
{
  return m_size;
}

void
WifiMacQueue::Flush (void)
{
  m_queue.erase (m_queue.begin (), m_queue.end ());
  m_size = 0;
}

Mac48Address
WifiMacQueue::GetNextHopMacAddressForPacket (enum WifiMacHeader::AddressType type, PacketQueueI it)
{
  if (type == WifiMacHeader::ADDR1)
    {
      return it->hdr.GetAddr1 ();
    }
  if (type == WifiMacHeader::ADDR2)
    {
      return it->hdr.GetAddr2 ();
    }
  if (type == WifiMacHeader::ADDR3)
    {
      return it->hdr.GetAddr3 ();
    }
  return 0;
}

std::pair <Ipv4Address,Ipv4Address> 
WifiMacQueue::GetSrcDestinIpv4AddressForPacket (PacketQueueI it)
{
  Ptr<Packet> pCopy = it->packet->Copy ();

  LlcSnapHeader llc;
  pCopy->RemoveHeader(llc);

  Ipv4Header ipv4Header;
  pCopy->RemoveHeader (ipv4Header);

  std::pair <Ipv4Address,Ipv4Address> srcsinkIp;  

  srcsinkIp.first = ipv4Header.GetSource ();
  srcsinkIp.second = ipv4Header.GetDestination ();

  return srcsinkIp;
}

bool
WifiMacQueue::Remove (Ptr<const Packet> packet)
{
  PacketQueueI it = m_queue.begin ();
  for (; it != m_queue.end (); it++)
    {
      if (it->packet == packet)
        {
          m_queue.erase (it);
          m_size--;
          return true;
        }
    }
  return false;
}

void
WifiMacQueue::PushFront (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  Cleanup ();
  if (m_size == m_maxSize)
    {
      NS_LOG_DEBUG(this<< "QUEUE REACHES THE MAX SIZE!");
      return;
    }
  Time now = Simulator::Now ();
  m_queue.push_front (Item (packet, hdr, now));
  m_size++;
}

uint32_t
WifiMacQueue::GetNPacketsByTidAndAddress (uint8_t tid, WifiMacHeader::AddressType type,
                                          Mac48Address addr)
{
  Cleanup ();
  uint32_t nPackets = 0;
  if (!m_queue.empty ())
    {
      PacketQueueI it;
      for (it = m_queue.begin (); it != m_queue.end (); it++)
        {
          if (GetNextHopMacAddressForPacket (type, it) == addr)
            {
              if (it->hdr.IsQosData () && it->hdr.GetQosTid () == tid)
                {
                  nPackets++;
                }
            }
        }
    }
  return nPackets;
}

uint32_t
WifiMacQueue::GetNPacketsByAddress (WifiMacHeader::AddressType type,
                                          Mac48Address addr,
                                          std::pair <Ipv4Address,Ipv4Address> srcsinkIp)
{
  Cleanup ();
  uint32_t nPackets = 0;
  if (!m_queue.empty ())
    {
      PacketQueueI it;
      for (it = m_queue.begin (); it != m_queue.end (); it++)
        {
          if (GetNextHopMacAddressForPacket (type, it) == addr && GetSrcDestinIpv4AddressForPacket(it) == srcsinkIp)
            {
              if (it->hdr.IsQosData ())
                {
                  nPackets++;
                }
            }
        }
    }
  return nPackets;
}

Ptr<const Packet>
WifiMacQueue::DequeueFirstAvailable (WifiMacHeader *hdr, Time &timestamp,
                                     const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this);
  Cleanup ();
  Ptr<const Packet> packet = 0;
  for (PacketQueueI it = m_queue.begin (); it != m_queue.end (); it++)
    {
      if (!it->hdr.IsQosData ()
          || !blockedPackets->IsBlocked (it->hdr.GetAddr1 (), it->hdr.GetQosTid ()))
        {
          *hdr = it->hdr;
          timestamp = it->tstamp;
          packet = it->packet;
          m_queue.erase (it);
          m_size--;
          return packet;
        }
    }
  return packet;
}

Ptr<const Packet>
WifiMacQueue::PeekFirstAvailable (WifiMacHeader *hdr, Time &timestamp,
                                  const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this);
  Cleanup ();
  for (PacketQueueI it = m_queue.begin (); it != m_queue.end (); it++)
    {
      if (!it->hdr.IsQosData ()
          || !blockedPackets->IsBlocked (it->hdr.GetAddr1 (), it->hdr.GetQosTid ()))
        {
          *hdr = it->hdr;
          timestamp = it->tstamp;
          return it->packet;
        }
    }
  return 0;
}

} // namespace ns3
