#ifndef FLYWAYS_TOPO_HELPER_H
#define FLYWAYS_TOPO_HELPER_H

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>

#include "ns3/data-rate.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-address.h"

#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/cone-antenna.h"
#include "ns3/measured-2d-antenna.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/qos-wifi-mac-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/wifi-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/propagation-loss-model.h"

using namespace std;

namespace ns3
{

enum FW_TypeOfArrangement { FW_ToA_Square, FW_ToA_Unknown };
enum FW_TrafficType { FW_TT_ReplayFlows, FW_TT_Unknown };
enum FW_NodeType { FW_Node_ToR, FW_Node_Agg, FW_Node_L3 };
enum FW_AntennaType { FW_Antenna_Cone, FW_Antenna_Measured };
enum FW_StandardType { FW_Standard_SC, FW_Standard_OFDM };

class GlobalOpt{
 public:
  int numTors;
  int aggFanOut, l3FanOut;
  int donglesPerTor, donglesPerAgg, donglesPerL3;
  double rackWidth_meter, rackHeight_meter;
  uint clumpNumRacks;
  double clumpSpaceLeft_meter, clumpSpaceBelow_meter;

  double scaleDownBy;

  FW_AntennaType antenna;
  FW_StandardType standard;

  FW_TypeOfArrangement arrangeType;
  DataRate agg2L3_Bandwidth, tor2Agg_Bandwidth, flywayBandwidth;

  FW_TrafficType trafficType;
#define GO_MAX_FILENAMELEN 100
  char replayFlowFileName[GO_MAX_FILENAMELEN];

  void Print(){
    /* tbw: prints out the options */
  }

  // Constructor
  GlobalOpt();

};

class TrafficApp
{
 public:
  uint fromNode, toNode;
  float beginTime, transferSize;

  char fromApp[100], toApp[100]; // type of application
  char proto[10]; // protocol
  char dataRateString[30];
  Ptr<Application> sourceApp;
  Ptr<PacketSink> sinkApp;

  uint64_t GetTotalRx()
  {
      return sinkApp->GetTotalRx();
  }

  bool IsFinished()
  {
      return sinkApp->IsFinished();
  }

  Time WhenFinished()
  {
      return sinkApp->WhenFinished();
  }

  double TotalDelay()
  {
      return sinkApp->TotalDelay();
  }

  TrafficApp(){
    bzero(fromApp, 100);
    bzero(toApp, 100);
    bzero(proto, 10);
    bzero(dataRateString, 30);
  }
};

class FlywaysTopoHelper
{
 public:
  FlywaysTopoHelper (char *topoFile,
		     InternetStackHelper stack,
		     Ipv4AddressHelper address);
  void ReadTopo(FILE*);
  void AddDeviceAt (uint ind1, uint ind2, Ptr<NetDevice> nd);
  void AddInterfaceAt(uint ind1, uint ind2, 
		      pair< Ptr<Ipv4>, uint32_t> ipv);
  ~FlywaysTopoHelper ();

  Ptr<NetDevice> GetDevice(uint32_t index, int32_t nbr);
  vector<uint32_t> * GetNbrsOfNode(uint32_t index);
  Ptr<Node> GetNode(uint32_t index);
  Ipv4Address GetIpv4Address (uint32_t index, int32_t nbr=-1);
  int GetNumNodes() const;
  uint GetNumToRs() const;
  uint GetNumDongles() const;
  int GetNumApps() const;
  int GetNumFlyways() const;
  uint64_t GetTotalRx(int appId);
  bool IsFinished(int appId);
  Time WhenFinished(int appId);
  double TotalDelay(int appId);
  pair<double, double> GetNextRackLocation();
  uint GetCentralRack(vector<uint> *rackids);
  void InitGeometry();
  void SetupFlows(FILE*);
  void SetupFlow(TrafficApp*, int i);
  void SetupMobilityModel();
  void SetupWirelessInterfaces();
  void SetFlywayInterfaceUpDown(FW_NodeType nodeType, int nodeId, int dongleGroupId, bool up);
  void SetupWifi(int channelNumber);
  NetDeviceContainer GetWiredUplinks(uint nodeid);
  NetDeviceContainer GetFlywayLinks(uint nodeid);
  Ptr<YansWifiPhy> GetFlywayPhy(uint nodeId, uint dongleGroup);
  void ChangeChannel(uint nodeId, uint dongleGroup, uint16_t channelNumber);
  void SetAntennaAzimuthAngle(uint nodeId, uint dongleGroup, double angle);
  void SetAntennaElevationAngle(uint nodeId, uint dongleGroup, double angle);
  void SetAntennaBeamwidth(uint nodeId, uint dongleGroup, double beamwidth);
  void SetAntennaGain(uint nodeId, uint dongleGroup, double gain);
  double GetAngle(double x1, double y1, double x2, double y2);
  void PointNodeTo(uint nodeId, uint dongleGroup, double x, double y);
  void PointAtEachOther(uint node1, uint dongleId1, uint node2, uint dongleId2);
  void PointNowhere(uint nodeId, uint dongleId);
  double Distance (pair<double, double> a, pair<double, double> b);
  double GetDistance(int node1, int node2);
  double CalcSignalStrength(int from, uint dongleFrom, int to, uint dongleTo);

  // Mobility helper. In this case, it is just a position holder.
  MobilityHelper mobility;
  
  // WiFi helper
  WifiHelper wifi;
  QosWifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;

  NodeContainer tors; // Top of the rack nodes
  NodeContainer aggs; // Aggregator
  NodeContainer l3s;  // Layer 3 Switches

  // Wired uplinks for the node.
  map<uint, NetDeviceContainer> wiredUplinks; 

  // Flyway uplinks for the node.
  map<uint, NetDeviceContainer> flywayLinks;

  map<uint, NetDeviceContainer> torDongles;
  map<uint, NetDeviceContainer> aggDongles;
  map<uint, NetDeviceContainer> l3Dongles;

  map<uint, Ipv4InterfaceContainer> torInterfaces;
  map<uint, Ipv4InterfaceContainer> aggInterfaces;
  map<uint, Ipv4InterfaceContainer> l3Interfaces;

  uint m_numtors, m_numaggs, m_numl3s;
  uint m_numdongles;
  uint m_numnodes;
  Ipv4AddressHelper m_address;

  // ns3 state vars
  NodeContainer m_nodes;
  vector<map<uint, Ptr<NetDevice> >* > m_v_devices;           // for eacFlywaysTopoHelperh node, a pair of <neighbor,device>
  vector<map<uint, pair<Ptr<Ipv4>, uint> >*> m_v_interfaces;  // for each node a map of <neighbor <ipaddress, nbr>>
  map<Ipv4Address, uint> m_m_addresses;                       // for each ipv4address, a nodeId

  // for geometry
  uint m_finishedX, m_finishedY; // [0, 0] to [finishedX, finishedY] is full
  uint m_currClumpX, m_currClumpY, m_currClumpNumRacksReady; // statistics on current clump being filled

  vector<pair<double, double> > m_v_nodeXYLocations_meter; // x, y co-ordinates of each node, in feet

  // for topology
  map<uint, uint> m_tor2agg_index;
  map<uint, vector<uint>* > m_agg2tor_index;

  map<uint, uint > m_agg2l3_index;

  // for traffic sources
  vector<uint> m_v_node2NextPort;
  vector<TrafficApp*> m_v_apps; // traffic sources

  // constructor
  FlywaysTopoHelper();
};

}
#endif /* FLYWAYS_TOPO_HELPER_H */
