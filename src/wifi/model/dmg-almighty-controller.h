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
           Rui Li <lirui628@gmail.com>
 */
#ifndef DMG_ALMIGHTY_CONTROLLER_H
#define DMG_ALMIGHTY_CONTROLLER_H

#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/wifi-mode.h"
#include "ns3/nstime.h"
#include "ns3/ipv4-interface-address.h"

#include <vector>
#include <algorithm>
#include <map>



namespace ns3 {

/* Controller of the DMG network.
 * The controller knows every node (AP and STA) of the network.
 * It is able to compute the standard based association (AP with the highest
 * SNR).
 * It is able to configure the DmgBeaconInterval and the DmgServicePeriod of the
 * nodes.
 * Currently the controller supports only static networks (nodes positions and
 * configurations do not change during the simulations. The nodes are configured
 * at the beginning of the simulations.)
 * In future implementations the controller will be able to handle dynamic
 * networks
 *
 * We suppose that all the nodes are configured in the same way wrt:
 * - Antenna type.
 * - Antenna gain.
 * - Tx Rx gain.
 * - Tx power. 
 * - BI duration.
 * - BI overhead.
 * - rx noise figure.
 */
class DmgAlmightyController : public Object
{
public:
  DmgAlmightyController ();

  virtual ~DmgAlmightyController ();

  /* Set the Mesh nodes container */
  void SetMeshNodes (NodeContainer *meshnodes);
  void SetFlowsPath  (std::vector < std::vector <uint32_t> > path);
  /* Return the nodes container */
  NodeContainer *GetMeshNodes (void);
  NodeContainer *GetCtrlNodes (void);

struct cliqueStruct
{
    std::vector < uint32_t > staMem;
    uint32_t staMemN;
    std::vector < std::vector < uint32_t > > flowSegs;//{to from; to from; to from }
    std::vector < uint32_t > flows;                   //{flow1;   flow1;   flow2   }
    std::vector < float > timeAlloc;                 //{timeFrac;timeFrac;timeFrac}
    std::vector < double > phyRate;                   //{rate1;   rate2;   rate2   }
    std::vector < std::vector < uint64_t > > bufStart;//same number of rows with flowSegs
    std::vector < std::vector < uint64_t > > bufDurNs;
    std::vector < uint32_t> bufStaOrder; // To be used by the conflict node for shuffling and get the correct order.
};
    std::vector <cliqueStruct> cliqueS;//to be renamed.
    
  /* Set the service period allocation vector */
  void SetServicePeriodAllocation (std::vector < std::vector <double> >  spAlloc);
  /* Return the service period allocation vector */
  std::vector < std::vector <double> >  GetServicePeriodAllocation (void);

  /* Setter and Getter for various nodes paramenters.
   * Not all these parameters are currently used.
   */
  void SetAntennaSectorsN (double sectorsN);
  double GetAntennaSectorsN (void);
  void SetAntennaBeamwidth (double beamwidth);
  double GetAntennaBeamwidth (void);
  void SetAntennaGainTxRx (double gain);
  double GetAntennaGainTxRx (void);
  void SetEnergyDetectionThreshold (double detTh);
  double GetEnergyDetectionThreshold (void);
  void SetCcaMode1Threshold (double cca);
  double GetCcaMode1Threshold (void);
  void SetTxGain (double gain);
  double GetTxGain (void);
  void SetRxGain (double gain);
  double GetRxGain (void);
  void SetTxPowerLevels (double txLevels);
  double GetTxPowerLevels (void);
  void SetTxPowerEnd (double txPowerEnd);
  double GetTxPowerEnd (void);
  void SetTxPowerStart (double txPowerStart);
  double GetTxPowerStart (void);
  void SetRxNoiseFigure (double noiseFigure);
  double GetRxNoiseFigure (void);
  void SetBiDuration (uint64_t biDur);
  uint64_t GetBiDuration (void);
  void SetBiOverheadFraction (double biOverhead);
  void SetNumSchedulePerBi(uint32_t numSchedulePerBi);
  void SetAckTimeFrac(double ackTimeFrac);
  double GetBiOverheadFraction (void);
  void SetBeamSwitchOverhead (uint64_t beamSwitchOverhead);
  uint64_t GetBeamSwitchOverhead (void);
  double GetActualTxDurationNs(WifiMode mode, uint32_t nMpdus);

  /*Configure a single scheudel and buffer all sps in the structure conlictS */
  void ConfigureSchedule (void);
  
  /* Configure a DmgBeaconInterval for each node of the network with the schedule.
   * A DmgBeaconInterval holds a list of DmgServicePeriod 
   */
  void ConfigureBeaconIntervals (void);

  /* Configure the WifiRemoteStationManager (DmgDestinationFixedWifiManager)
   * For each node, the corresponding DmgDestinationFixedWifiManager knows the 
   * ideal transmission MCS for each possible destination.
   */
  void ConfigureWifiManager (void);

  void ConfigureWritePath (std::string w2str);

  //void ConfigureStadardBasedAssociation (void);
  void ConfigureRoutingBasedAssociation (void);

  void ConfigureCliques (void);
  std::vector <double>  FlowRateProgressiveFilling(std::vector <double> flowsDmd, double fillingSteplength, uint32_t appPayloadBytes, double biOverheadFraction, uint32_t nMpdus);
  /* For each Mesh node, the controller assign the same amount of air-time to each
   * stations associated to that mesh station 
   */
  void AssignEqualAirTime(void);

  /* Utility functions for printing informations about the network */
  void PrintIdealRxPowerAndMcs(void);
  void PrintAssociationsInfo(void);
  void PrintSpInfo(void);

  double GetIdealRxPower(Ptr<Node> from, Ptr<Node> to);

  /* Calc the interference power at *victim*, when talking to *partner*, received from node1, when tx to node2*/
  double GetInterferencePowerFromPair (Ptr<Node> victim, Ptr<Node> partner, Ptr<Node> node1, Ptr<Node> node2);


  void EnforceAdditionalSignalLossBetween(Ptr<Node> node1, Ptr<Node> node2, double addLoss);

  /*Set Phy Rate for the clique class, 
   * with the 1) ideal phy rate as per MCS values (when m_phyRateEstimated == 0)
           or 2) estimated phy rates computed from measured 'round trip time' of mac nMPDUs (when m_phyRateEstimated == 1)
   * called by progressive filling*/
  void ConfigurePhyRate(uint32_t appPayloadBytes, double biOverheadFraction, uint32_t nMpdus);
  /* Force Block Ack agreement creation */
  void CreateBlockAckAgreement(void);

  void StartMacEnqueueRateMeasurement (void);
  void RestartMacEnqueueRateMeasurement (void);
  std::vector <double> GetMacEnqueueRateMeasurement (void);

  void SetGw  (uint32_t node);
  uint32_t GetGw  (void);

  std::vector <uint32_t> ConfigureHierarchy (void);
  uint32_t GetMasterCliqueId (uint32_t node);
  uint32_t GetMasterNodeId (uint32_t node);

  void ConfigureAntennaAlignment (void);

  std::vector <double> FlowRateMaxDlmac(uint32_t payloadBytes, double biOverheadFraction);

private:

  /* Function used by ConfigureWifiManager for configuring the 
   * DmgDestinationFixedWifiManager
   */
  WifiMode GetWifiMode (double rxPowerDbi, bool ofdm);

  /* it a caller responsibility to ensure that the internal state
   * of the controller (e.g. assocPairs) is set correctly after one of the following two
   * containers has been modified.
   */
    NodeContainer *m_meshNodes; //Mesh stations under control
    NodeContainer *m_ctrlNodes; //Mesh controler
  
  /* assocPairs.at(i) = j
   * i is the index of staNodes
   * j is the index of controller
   */
    std::vector < std::vector <uint32_t> > m_cliques;
    std::vector <uint32_t> m_nextHops;
    std::vector < std::vector <uint32_t> > m_neighbourNodes;

  /* NOTE: Ignore the following three variables. They are not currently used.
   * In the current version we suppose every node is equipped with the same
   * antenna (everyone has the same number of sector and m_antennaSectorN *
   * m_antennaBeamwidth = 360 degress). When one of the following three
   * variables is set the other two are computed according to the value of the
   * first one.
   * NOTE: the real nodes setting in performed by DmgScenarioHelper. The
   * controller keeps this values only for caching purposes.
   * NOTE: if you want to copute the association and resource allocation
   * solution ofline with matlab you must be sure that th configuration file
   * given as input to matlab share the same login to set the RX and TX gain.
   */
  double m_antennaSectorsN;
  double m_antennaBeamwidth;
  double m_antennaGainTxRx;

  /* we assume that the following values are shared by all the nodes and that
   * are the same used in Matlab for computing the solutions of the optimisation
   * problem
   * NOTE: This values are not currently used by the controller.
   */
  double m_energyDetectionThreshold;
  double m_ccaMode1Threshold;
  double m_txGain;
  double m_rxGain;
  double m_txPowerLevels;
  double m_txPowerEnd;
  double m_txPowerStart;
  double m_rxNoiseFigure;
    

  /* Duration in nanoseconds of the Beacon Interval.
   * We assume that all the Mesh stations and controller share this value.
   */
  uint64_t m_biDuration;
  /* Fraction of the Beacon Interval used as overhead (nodes does not transmit
   * during this time).
   * We suppose that this value is the same for all the Mesh stations and controller
   */
  double m_biOverhaedFraction; 
  /* Duration in nanoseconds used as beam switch overhead 
   * We suppose that this value is the same for all the mesh stations and controller
   */
  uint64_t m_beamSwitchOverheadNs;

  /* Number of repeated schedule in the beacon interval, each of which contains   
   * a full set of all SPs in the scheduled order.
   */
  uint32_t m_numSchedulePerBi;

  /* Fraction of Data Tx used for the traffic to the inversed direction of a flow such as   
   * TCP acks
   */
  double m_ackTimeFrac;

  std::vector < std::vector <uint32_t> > m_flowsPath;

  std::vector < std::vector <uint32_t> > m_linkList;
  /* service period allocation
   * spAlloc.at(i) = f
   * staNode i has a fraction f of the available time in the Beacon Interval
   * (m_biDuration - m_biOverheadFraction * m_biDuration)
   */
    std::vector < std::vector <double> > m_spAlloc;

   /*  
    * The measured flow demand vector temporarily put here 
    */
   std::vector <double> m_flowDemandMeasured;

   /*  
    * PHY rate estimated or not
    */
   bool m_phyRateEstimated;

   /*Index of the gateway node*/
   uint32_t m_gw;

   std::map <int32_t, int32_t> m_master;
   std::map <int32_t, int32_t> m_masterClique;


   std::string m_writePath;

};

} // namespace ns3

#endif /* DMG_ALMIGHTY_CONTROLLER_H */
