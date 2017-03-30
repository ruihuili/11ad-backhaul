#include "ns3/flyways-topo-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/canvas-location.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/enum.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/on-off-helper.h"
#include <string.h>
#include "ns3/point-to-point-net-device.h"
#include <stdio.h>
#include "ns3/mac48-address.h"
#include "ns3/wifi-net-device.h"

NS_LOG_COMPONENT_DEFINE("FlywaysTopoHelper");
using namespace std;

namespace ns3 {
    GlobalOpt g_Options;

    GlobalOpt::GlobalOpt()
    {
        /* Configure with default values */
        numTors = 160; // one agg switch's content
        aggFanOut = 160;
        l3FanOut = 10;

        scaleDownBy = 1;

        agg2L3_Bandwidth = DataRate(DataRate("40Gbps").GetBitRate()/ scaleDownBy);
        tor2Agg_Bandwidth = DataRate(DataRate("10Gbps").GetBitRate()/ scaleDownBy);
        flywayBandwidth = DataRate(DataRate("1Gbps").GetBitRate()/ scaleDownBy);

        rackWidth_meter = 2;
        rackHeight_meter = 4;
        clumpSpaceLeft_meter = 10;
        clumpSpaceBelow_meter = 6;
        clumpNumRacks = 10;

        antenna = FW_Antenna_Cone;

        arrangeType = FW_ToA_Square;
        trafficType = FW_TT_Unknown;
        donglesPerTor = donglesPerAgg = donglesPerL3 = 0;

        standard = FW_Standard_OFDM;
    }

    void   
    FlywaysTopoHelper::ReadTopo
    ( FILE* fin )
    {
        #define FTH_readtopo_MAXLINELEN 100
        char line[FTH_readtopo_MAXLINELEN];
        int linelen=0;
        int linenum = 0;
        /* read the topo in */
        while(fgets(line, FTH_readtopo_MAXLINELEN, fin) != NULL ) {

            linenum++;
            linelen = strlen(line); 

	        if(linelen == FTH_readtopo_MAXLINELEN) {
	            printf("%s: Too long a line: %d\n", "flyways-topo-helper", linelen); 
	            abort();
	        }
	        else 
	            line[--linelen]=0;
	
	        if(line[0] == '#') 
	            continue;
	
	        if (strstr(line, "DonglesPerTor:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'DonglesPerTor: int'\n", line);
	                exit(-1);
	            }
	            g_Options.donglesPerTor = _int1;
	        } 
            else if (strstr(line, "Antenna:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'DonglesPerTor: int'\n", line);
	                exit(-1);
	            }
                if (_int1 == 0)
                {
                    printf("setting cone antenna\n");
                    g_Options.antenna = FW_Antenna_Cone;
                }
                else if (_int1 == 1)
                {
                    printf("setting measured antenna\n");
                    g_Options.antenna = FW_Antenna_Measured;
                }
                else 
                {
                    printf("only vali antenna types are 0 or 1\n");
	                exit(-1);
                }
	        } 
	        else if (strstr(line, "DonglesPerAgg:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){ 
	                printf("bad line in topo file: [%s] shld be 'DonglesPerAgg: int'\n", line);
	                exit(-1);
	            }
	            g_Options.donglesPerAgg = _int1;
	    
	        } 
	        else if (strstr(line, "DonglesPerL3:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'DonglesPerL3: int'\n", line);
	                exit(-1);
	            }
	            g_Options.donglesPerL3 = _int1;
	        } 
	        else if (strstr(line, "FlywayBandwidth:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            char _bandwidth[10];
	            if ( sscanf(ctmp, " %s", _bandwidth) != 1){ 
	                printf("bad line in topo file: [%s] shld be 'FlywayBandwidth: bandwidth'\n", line);
	                exit(-1);
	            }
	            g_Options.flywayBandwidth = DataRate(_bandwidth);
	        } 
	        else if (strstr(line, "NUM-TORS:") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'NUM-TORS: int'\n", line);
	                exit(-1);
	            }
	            g_Options.numTors = _int1;
	        }
	        else if (strstr(line, "Agg2L3_Bandwidth:") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            char _bandwidth[10];
	            if ( sscanf(ctmp, " %s", _bandwidth) != 1){
	                printf("bad line in topo file: [%s] shld be 'Agg2L3_Bandwidth: bandwidth'\n", line);
	                exit(-1);
	            }
	            g_Options.agg2L3_Bandwidth = DataRate(_bandwidth);
	        }
	        else if (strstr(line, "Tor2Agg_Bandwidth:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            char _bandwidth[10];
	            if ( sscanf(ctmp, " %s", _bandwidth) != 1){
	                printf("bad line in topo file: [%s] shld be 'Tor2Agg_Bandwidth: bandwidth'\n", line);
	                exit(-1);
	            }
	            g_Options.tor2Agg_Bandwidth = DataRate(_bandwidth);
	        }
	        else if (strstr(line, "ReplayFlowFile:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            if (sscanf(ctmp, " %s", g_Options.replayFlowFileName) != 1 || strlen(g_Options.replayFlowFileName) > GO_MAX_FILENAMELEN-1){
	                printf("bad line in topo file: [%s] shld be 'ReplayFlowFile: fileName'\n", line);
	                exit(-1);
	            }
	            g_Options.trafficType = FW_TT_ReplayFlows;
	        }
	        else if (strstr(line, "AggFanOut:") == line){
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'AggFanOut: int'\n", line);
	                exit(-1);
	            }
	            g_Options.aggFanOut = _int1;
	
	        }
	        else if (strstr(line, "L3FanOut:") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'L3FanOut: int'\n", line);
	                exit(-1);
	            }
	            g_Options.l3FanOut = _int1;
	
	        }
	        else if (strstr(line, "RackWidth") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            float _float1;
	            if ( sscanf(ctmp, " %f", &_float1) != 1){
	                printf("bad line in topo file: [%s] shld be 'RackWidth: float'\n", line);
	                exit(-1);
	            }
	            g_Options.rackWidth_meter = _float1;
	        }
	        else if (strstr(line, "RackHeight") == line){
	            char* ctmp = strchr(line, ':')+1;
	            float _float1;
	            if ( sscanf(ctmp, " %f", &_float1) != 1){
	                printf("bad line in topo file: [%s] shld be 'RackHeight: float'\n", line);
	                exit(-1);
	            }
	            g_Options.rackHeight_meter = _float1;
	
	        }
	        else if (strstr(line, "ClumpNumRacks") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            int _int1;
	            if ( sscanf(ctmp, " %d", &_int1) != 1){
	                printf("bad line in topo file: [%s] shld be 'ClumpNumRacks: int'\n", line);
	                exit(-1);
	            }
	            g_Options.clumpNumRacks =(uint)(_int1);
	        }
	        else if (strstr(line, "ClumpSpaceLeft") == line){
	            char* ctmp = strchr(line, ':')+1;
	            float _float1;
	            if ( sscanf(ctmp, " %f", &_float1) != 1){
	                printf("bad line in topo file: [%s] shld be 'ClumpSpaceLeft: float'\n", line);
	                exit(-1);
	            }
	            g_Options.clumpSpaceLeft_meter = _float1;
	        }
	        else if (strstr(line, "ClumpSpaceBelow") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            float _float1;
	            if ( sscanf(ctmp, " %f", &_float1) != 1){
	                printf("bad line in topo file: [%s] shld be 'ClumpSpaceBelow: float'\n", line);
	                exit(-1);
	            }
	            g_Options.clumpSpaceBelow_meter = _float1;
	
	        }
	        else if (strstr(line, "ScaleDownBy") == line){
	
	            char* ctmp = strchr(line, ':')+1;
	            float _float1;
	            if ( sscanf(ctmp, " %f", &_float1) != 1){
	                printf("bad line in topo file: [%s] shld be 'ScaleDownBy: float'\n", line);
	                exit(-1);
	            }
	            g_Options.scaleDownBy = _float1;
	        }
	        else if (strstr(line, "Arrangement") == line){
	            char *ctmp = strchr(line, ':') + 1;
	            if ( strcmp(ctmp, "Square")==0 )
	                g_Options.arrangeType = FW_ToA_Square;
	            else
	                g_Options.arrangeType = FW_ToA_Unknown;
	
	        }
	        else if (strstr(line, "Standard") == line){
	            char *ctmp = strchr(line, ':') + 1;
	            if ( strcmp(ctmp, "SC")==0 )
	                g_Options.standard = FW_Standard_SC;
	            else
	                g_Options.standard = FW_Standard_OFDM;
	
	        }
	        else
	            printf("%s: Ignoring %d:[%s]\n", "flyways-topo-helper", linenum, line);
        }

        if ( g_Options.scaleDownBy != 1){
            g_Options.agg2L3_Bandwidth = DataRate( g_Options.agg2L3_Bandwidth.GetBitRate() / g_Options.scaleDownBy );
            g_Options.tor2Agg_Bandwidth = DataRate( g_Options.tor2Agg_Bandwidth.GetBitRate() / g_Options.scaleDownBy );
        }
    }

    void FlywaysTopoHelper::InitGeometry()
    {
        m_finishedX = m_finishedY = 0;
        m_currClumpX = m_currClumpY = m_currClumpNumRacksReady = 0;
    }


    uint FlywaysTopoHelper::GetCentralRack(vector<uint>* rackids)
    {
        double xmean = 0, ymean = 0, num = 0;
        for(uint ind=0; ind < rackids->size(); ind++){
            uint rackid = (*rackids)[ind];
            num ++;
            xmean += (m_v_nodeXYLocations_meter[rackid].first - xmean)/ num;
            ymean += (m_v_nodeXYLocations_meter[rackid].second - ymean)/ num;
        }
        cout << "[Geo]: central rack (x, y) = (" << xmean << ", " << ymean << ")" << endl;
        double mindist = 2 * sqrt( pow(m_finishedY, 2) + pow(m_finishedX, 2));
        uint bestindex = -1;
        for(uint ind=0; ind < rackids->size(); ind++){
            uint rackid = (*rackids)[ind];
            double dist = sqrt ( pow(xmean - m_v_nodeXYLocations_meter[rackid].first, 2) + pow(ymean - m_v_nodeXYLocations_meter[rackid].second, 2) );
            if ( dist < mindist ){
                bestindex = ind;
                mindist = dist;
            }           
        }
        cout << "[Geo]: bestRack index = " << bestindex << " dist = " << mindist << endl;
        NS_ASSERT_MSG(bestindex >= 0, "bad matching for central rack");
        return bestindex;
    }


    void FlywaysTopoHelper::SetupFlows(FILE* fin)
    {
        #define FTH_setupflows_MAXLINELEN 1000
        char line[FTH_setupflows_MAXLINELEN];
        int linelen=0;

        /* read the flow file in */
        TrafficApp *_next_app = NULL;
        int i = 0;
        while(fgets(line, FTH_setupflows_MAXLINELEN, fin) != NULL ) {

            linelen = strlen(line); 

            if(linelen == FTH_setupflows_MAXLINELEN) {
                printf("%s: Too long a line: %d\n", "flyways-topo-helper", linelen); 
                abort();
            }
            else 
                line[--linelen]=0;

            if(line[0] == '#') 
                continue;

            if ( _next_app == NULL )
                _next_app = new TrafficApp();
            if (sscanf(line, " %f %d %d %s %f %s", 
                        &(_next_app->beginTime), 
                        &(_next_app->fromNode), 
                        &(_next_app->toNode),
                        (_next_app->proto),
                        &(_next_app->transferSize),
                        (_next_app->dataRateString)
                ) != 6) {
                printf("%s: not six fields: %d\n", "flyways-topo-helper", linelen); 
                abort();
            }

            NS_ASSERT_MSG ( strlen(_next_app->proto) <10 && 
              strlen(_next_app->dataRateString) < 30, "too long protocol or dataRate string");
            SetupFlow(_next_app, i);
            //cout << i << ":" << _next_app->transferSize << endl;
            i++;
            if (i % 1000 == 0) { cout << "set up " << i << "flows" << endl;}
            _next_app = NULL;
        }
    }

    void FlywaysTopoHelper::SetupFlow(TrafficApp* ta, int i){

        char protoString[100];
        if ( strcmp (ta->proto, "Tcp") == 0 )
            strcpy(protoString, "ns3::TcpSocketFactory");
        else if (strcmp(ta->proto, "Udp") == 0 )
            strcpy(protoString, "ns3::UdpSocketFactory");
        else{
            cout << "proto invalid " << ta->proto << endl;
            exit(6);
        }

        /* Receiver */
        uint toPort = m_v_node2NextPort[ta->toNode]++;
        //cout << "port for this: " << toPort << endl;
        //NS_ASSERT_MSG ( toPort < (1<<16), "toNode ports filled up");
        PacketSinkHelper sinkHelper (protoString, InetSocketAddress(Ipv4Address::GetAny(), toPort));
        //PacketSinkHelper sinkHelper (protoString, InetSocketAddress(Ipv4Address::GetAny(), (ushort)toPort));
        ApplicationContainer sinkApp = sinkHelper.Install( GetNode(ta->toNode) );
        sinkApp.Start(Seconds(ta->beginTime - 0.000001 > 0 ? ta->beginTime - 0.000001 : 0));
        //sinkApp.Start(Seconds(0));

        /* Sender */
        OnOffHelper src (protoString, Address());
        //cout << "dst addr is " << GetIpv4Address(ta->toNode) << endl;
        src.SetAttribute("Remote", AddressValue(InetSocketAddress(GetIpv4Address(ta->toNode), toPort)));
        src.SetAttribute("Local", AddressValue(InetSocketAddress(GetIpv4Address(ta->fromNode), 0)));
        //src.SetAttribute("Remote", AddressValue(InetSocketAddress(GetIpv4Address(ta->toNode), (ushort)toPort)));
        src.SetAttribute("MaxBytes", UintegerValue( (uint)ta->transferSize));
        src.SetAttribute("DataRate", DataRateValue(DataRate(ta->dataRateString)));
        //src.SetAttribute("OffTime", RandomVariableValue(ConstantVariable(ta->beginTime)));
        ApplicationContainer srcApp;
        srcApp.Add (src.Install(GetNode(ta->fromNode)));
        srcApp.Start (Seconds(ta->beginTime));
        //srcApp.Start (Seconds(i * 1e-6));

        // store the sink and source app
        ta->sourceApp = srcApp.Get(0);
        ta->sinkApp = DynamicCast<PacketSink>(sinkApp.Get(0)); 
        ta->sinkApp->SetExpected(ta->transferSize);

        m_v_apps.push_back(ta);
    }

    // constructor
    FlywaysTopoHelper::FlywaysTopoHelper 
        (char *topoFile,
        InternetStackHelper stack,
        Ipv4AddressHelper address)
    {

        m_address = address; 

        FILE *f_in;
        if (topoFile == NULL || (f_in = fopen(topoFile, "r")) == NULL) {
            printf("cannot read topo: %s\n", topoFile);
            exit(-1);
        }

        // Read the topo file.
        ReadTopo(f_in);

        // init geometry
        InitGeometry();

        m_numtors = g_Options.numTors;
        m_numdongles = g_Options.donglesPerTor;

        m_numaggs = 0;
        int unassignedTors = m_numtors;
        while ( unassignedTors > 0 ){
            m_numaggs += 2;
            unassignedTors -= g_Options.aggFanOut;
        }

        m_numl3s = 0;
        int unassignedAggs = m_numaggs;
        while( unassignedAggs > 0 ){
            m_numl3s += 2;
            unassignedAggs -= g_Options.l3FanOut;
        }

        m_numnodes = m_numtors + m_numaggs + m_numl3s;

        cout << "Nodes: " << m_numnodes << "ToRs/Aggs/L3s: " << m_numtors << " " << m_numaggs << " " << m_numl3s << endl;;
        assert (m_numnodes > 0);

        m_nodes.Create(m_numnodes);

        stack.Install(m_nodes);

        for(uint ind1=0; ind1 < m_numnodes; ind1++){
            m_v_devices.push_back(NULL); // empty map
            m_v_interfaces.push_back(NULL); // empty interface
            m_v_nodeXYLocations_meter.push_back(pair<double, double>(-1, -1)); // initialize
            m_v_node2NextPort.push_back(1); // next usable port is 1. 0 is a special value - cannot be used.
        }

        // get geo for ToRs
        for(uint ind1=0; ind1 < m_numtors; ind1++){
            m_v_nodeXYLocations_meter[ind1]=GetNextRackLocation();
            cout << "[GeoLoc] node " << ind1 << " (x, y) = (" << m_v_nodeXYLocations_meter[ind1].first << ", " << m_v_nodeXYLocations_meter[ind1].second << ")" << endl; 
        }

        // connect ToRs to their Agg Switch
        uint currentAggIndex_a = m_numtors;
        for(uint ind1=0; ind1 < m_numtors; ind1++){
            Time lat = MilliSeconds(0.01);
            //Time lat = MilliSeconds(10);
            //uint32_t qSize = g_Options.tor2Agg_Bandwidth.GetBitRate() * lat.GetSeconds() * 10/ 8;  // BDP assuming paths ~ 5hops
            for(uint ind2=0; ind2 < 2; ind2++){
                PointToPointHelper pointToPoint;
                pointToPoint.SetDeviceAttribute("DataRate", DataRateValue(g_Options.tor2Agg_Bandwidth));
                pointToPoint.SetChannelAttribute("Delay", TimeValue(lat));
                pointToPoint.SetQueueAttribute("Mode", EnumValue(DropTailQueue::BYTES));
                pointToPoint.SetQueueAttribute("MaxBytes", UintegerValue(1e7));

                int n1 = ind1;
                int n2 = ind2+ currentAggIndex_a;
            
                NodeContainer nc = NodeContainer(m_nodes.Get(n1), m_nodes.Get(n2));
                NetDeviceContainer linkDevices = pointToPoint.Install(nc);
              
                wiredUplinks[n1].Add(linkDevices.Get(0));

                Ipv4InterfaceContainer linkInterfaces = m_address.Assign(linkDevices);
            
                AddDeviceAt(n1, n2, linkDevices.Get(0));
                AddDeviceAt(n2, n1, linkDevices.Get(1));
    
                AddInterfaceAt(n1, n2, linkInterfaces.Get(0));
                AddInterfaceAt(n2, n1, linkInterfaces.Get(1));
           
                m_address.NewNetwork();
            }

            m_tor2agg_index.insert(pair<int, int> (ind1, currentAggIndex_a));

            if ( m_agg2tor_index.find(currentAggIndex_a) == m_agg2tor_index.end()){ 
                m_agg2tor_index.insert(pair<uint, vector<uint>* >(currentAggIndex_a, new vector<uint> () ));
            }
            m_agg2tor_index[currentAggIndex_a]->push_back(ind1);
      
            if ( ind1 !=  m_numtors-1 && (ind1+1) % g_Options.aggFanOut == 0 ) {
                currentAggIndex_a += 2;
            }
        }
        assert ( currentAggIndex_a + 1 < m_numtors + m_numaggs);

        cout << "======================================================================" << endl;
        // get geo for agg switch
        for(uint ind=m_numtors; ind < m_numtors+m_numaggs; ind+=2){
            m_v_nodeXYLocations_meter[ind] = m_v_nodeXYLocations_meter[GetCentralRack ( m_agg2tor_index[ind] )];
            m_v_nodeXYLocations_meter[ind+1] = m_v_nodeXYLocations_meter[ind];
        }

        // connect agg switches to l3
        uint currentL3Index_a = m_numtors + m_numaggs;
        for(uint ind1=0; ind1 < m_numaggs; ind1++){
            Time lat = MilliSeconds(0.01);
            //uint qSize = g_Options.agg2L3_Bandwidth.GetBitRate() * lat.GetSeconds() * 10/8;

            for(uint ind2=0; ind2<2; ind2++){
                PointToPointHelper pointToPoint;
                pointToPoint.SetDeviceAttribute("DataRate", DataRateValue(g_Options.agg2L3_Bandwidth));
                pointToPoint.SetChannelAttribute("Delay", TimeValue(lat));
                pointToPoint.SetQueueAttribute("Mode", EnumValue(DropTailQueue::BYTES));
                pointToPoint.SetQueueAttribute("MaxBytes", UintegerValue(1e7));
                int n1 = ind1 + m_numtors;
                int n2 = ind2 + currentL3Index_a;
    
                NodeContainer nc = NodeContainer(m_nodes.Get(n1), m_nodes.Get(n2));
                NetDeviceContainer linkDevices = pointToPoint.Install(nc);
                Ipv4InterfaceContainer linkInterfaces = m_address.Assign(linkDevices);
    
                wiredUplinks[n1].Add(linkDevices.Get(0));

                AddDeviceAt(n1, n2, linkDevices.Get(0));
                AddDeviceAt(n2, n1, linkDevices.Get(1));
    
                AddInterfaceAt(n1, n2, linkInterfaces.Get(0));
                AddInterfaceAt(n2, n1, linkInterfaces.Get(1));
                   
                m_address.NewNetwork();
            }
            m_agg2l3_index.insert(pair<int, int>(ind1+m_numtors, currentL3Index_a));
            if ( ind1 != m_numaggs - 1 && (ind1+1)%g_Options.l3FanOut == 0 ) {
                currentL3Index_a += 2;
            }
        } 
        SetupMobilityModel();
        SetupWirelessInterfaces();
        // Set up traffic
        if ( g_Options.trafficType == FW_TT_ReplayFlows ){

            FILE* flows = fopen(g_Options.replayFlowFileName, "r");
            if ( flows == NULL ) {
                printf("cannot read flow file: %s\n", g_Options.replayFlowFileName);
                exit(-1);
            }
            SetupFlows ( flows );
        }
        assert(currentL3Index_a +1 < m_numnodes);
    }

    void FlywaysTopoHelper::SetupMobilityModel()
    {
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        for (int i = 0; i < (int)(m_v_nodeXYLocations_meter.size()); i++) 
            // The postions must be recorded in meters, for that is the
            // universal unit of distance for all wireless stuff.
            positionAlloc->Add (Vector (m_v_nodeXYLocations_meter[i].first, m_v_nodeXYLocations_meter[i].second, 0.0));
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install(m_nodes);
    }

    
    void FlywaysTopoHelper::SetFlywayInterfaceUpDown(FW_NodeType nodeType, int nodeId, int dongleGroupId, bool up)
    {
        map<uint, Ipv4InterfaceContainer> interfaces;
        if (nodeType == FW_Node_ToR) 
            interfaces = torInterfaces;
        else if (nodeType == FW_Node_Agg)
            interfaces = aggInterfaces;
        else 
            interfaces = l3Interfaces;

        Ipv4InterfaceContainer ifc = interfaces[dongleGroupId];
        if (up) 
            ifc.Get(nodeId).first->SetUp(ifc.Get(nodeId).second);
        else
            ifc.Get(nodeId).first->SetDown(ifc.Get(nodeId).second);
    }

    void FlywaysTopoHelper::SetupWifi(int channelNumber)
    {
        double freq = 59.4e9;

        /* use 802.11ad */
        if (g_Options.standard == FW_Standard_SC)
            wifi.SetStandard (WIFI_PHY_STANDARD_80211ad_SC);
        else
            wifi.SetStandard (WIFI_PHY_STANDARD_80211ad_OFDM);
        /* Set default rate algorithm for all nodes */
        wifi.SetRemoteStationManager ("ns3::IdealWifiManager", "BerThreshold",DoubleValue(1e-9));
        //wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue("VHTMCS4"), "ControlMode", StringValue("VHTMCS4"));

        /* Simple propagation delay model */
        wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        /* Friis model with standard-specific wavelength */
        wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Lambda", DoubleValue(3e8/freq));
        
        /* Add a QoS upper mac */
        wifiMac = QosWifiMacHelper::Default ();
        /* Set it to flyways adhoc mode */
        wifiMac.SetType ("ns3::FlywaysWifiMac");
        /* Enable packet aggregation */
        wifiMac.SetBlockAckThresholdForAc(AC_BE, 4);
        /* Parametrize */
        wifiMac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (262143));
        
        /* set up phy */
        wifiPhy =  YansWifiPhyHelper::Default ();
        /* Nodes will be added to the channel we set up earlier */
        wifiPhy.SetChannel (wifiChannel.Create ());
        /* use cone antenna, but set default beamwdith to 360. */
        if (g_Options.antenna == FW_Antenna_Cone) 
        {
            wifiPhy.SetAntenna ("ns3::ConeAntenna", "Beamwidth", DoubleValue(ConeAntenna::GainDbiToBeamwidth(0)));
        }
        else if (g_Options.antenna == FW_Antenna_Measured)
        {
            wifiPhy.SetAntenna ("ns3::Measured2DAntenna", "Mode", DoubleValue(10));
        }
        /* Set the phy layer error model */
        wifiPhy.SetErrorRateModel ("ns3::SensitivityModel60GHz");
        /* set channel number*/
        wifiPhy.Set("ChannelNumber", UintegerValue(channelNumber));
    }

    void FlywaysTopoHelper::SetupWirelessInterfaces()
    {
        // Set up devices for ToRs
        SetupWifi(1);
        m_address.SetBase("20.1.0.0", "255.255.0.0");
        m_address.NewNetwork();
        for (int i = 0; i < (int)m_numtors;i ++) {
            tors.Add(m_nodes.Get(i));
        }
        for (int i = 0; i < (int)m_numdongles; i++) {
            torDongles[(uint)i] = wifi.Install(wifiPhy, wifiMac, tors);
            for (int nodeId = 0; nodeId < (int)m_numtors; nodeId++) {
                flywayLinks[nodeId].Add(torDongles[(uint)i].Get(nodeId));
            }
            torInterfaces[(uint)i] = m_address.Assign(torDongles[(uint)i]);
        }

        // We set them "down" to avoid routing traffic over them.
        for (int i = 0; i < (int) m_numdongles; i ++) {
            for (int j = 0; j < (int)m_numtors; j++) {
                Ipv4InterfaceContainer ifc = torInterfaces[i];
                ifc.Get(j).first->SetDown(ifc.Get(j).second);
            }
        }

        // Set up devices for Aggs.
        SetupWifi(2);
        m_address.NewNetwork();
        for (uint i = m_numtors; i < m_numtors + m_numaggs; i++) {
            aggs.Add(m_nodes.Get((int)i));
        }
        for (int i = 0; i < g_Options.donglesPerAgg; i++) {
            aggDongles[(uint)i] = wifi.Install(wifiPhy, wifiMac, tors);
            for (int nodeId = m_numtors; nodeId < (int)(m_numtors + m_numaggs); nodeId++) {
                flywayLinks[nodeId].Add(aggDongles[(uint)i].Get(nodeId));
            }
            aggInterfaces[(uint)i] = m_address.Assign(aggDongles[(uint)i]);
        }

        // We set them "down" to avoid routing traffic over them.
        for (int i = 0; i < (int) g_Options.donglesPerAgg; i ++) {
            for (int j = 0; j < (int)m_numaggs; j++) {
                Ipv4InterfaceContainer ifc = aggInterfaces[i];
                ifc.Get(j).first->SetDown(ifc.Get(j).second);
            }
        }

        // Set up devices for L3
        SetupWifi(3);
        m_address.NewNetwork();
        for (uint i = m_numtors + m_numaggs; i < m_numtors + m_numaggs + m_numl3s; i++) {
            l3s.Add(m_nodes.Get((int)i));
        }
        for (int i = 0; i < g_Options.donglesPerL3; i++) {
            l3Dongles[(uint)i] = wifi.Install(wifiPhy, wifiMac, tors);
            for (int nodeId = m_numtors + m_numaggs; nodeId <  (int)(m_numtors + m_numaggs + m_numl3s); nodeId++) {
                flywayLinks[nodeId].Add(l3Dongles[(uint)i].Get(nodeId));
            }
            l3Interfaces[(uint)i] = m_address.Assign(l3Dongles[(uint)i]);
        }

        // We set them "down" to avoid routing traffic over them.
        for (int i = 0; i < (int) g_Options.donglesPerL3; i ++) {
            for (int j = 0; j < (int)m_numaggs; j++) {
                Ipv4InterfaceContainer ifc = l3Interfaces[i];
                ifc.Get(j).first->SetDown(ifc.Get(j).second);
            }
        }
    }

    NetDeviceContainer FlywaysTopoHelper::GetWiredUplinks(uint nodeid)
    {
        if (wiredUplinks.find(nodeid) == wiredUplinks.end())
             NS_FATAL_ERROR("wireduplink not found");
        NetDeviceContainer x = wiredUplinks[nodeid];
        return wiredUplinks[nodeid];
    }

    NetDeviceContainer FlywaysTopoHelper::GetFlywayLinks(uint nodeid)
    {
        if (flywayLinks.find(nodeid) == flywayLinks.end())
             NS_FATAL_ERROR("flywaylink not found" << nodeid);
        return flywayLinks[nodeid];
    }

    void FlywaysTopoHelper::AddInterfaceAt(uint ind1, uint ind2, pair< Ptr<Ipv4>, uint> ipv)
    {
        Ipv4Address ia = ipv.first->GetAddress(ipv.second, 0).GetLocal();
   
        /*
        cout << "ind1 " << ind1 << " ind2 " << ind2 
        << " address: " << ia 
        << endl;
        */

        if ( ind1 < 0 || ind1 >= m_numnodes )
            NS_FATAL_ERROR("too large ind1 @ AddInterfaceAt");

        if ( m_v_interfaces.at(ind1) == NULL )
        m_v_interfaces.at(ind1) = new map<uint, pair<Ptr<Ipv4>, uint> > ();

        NS_ASSERT_MSG ( m_v_interfaces.at(ind1), "AddInterfaceAt creating map" );

        m_v_interfaces[ind1]->insert( pair<uint, pair< Ptr<Ipv4>, uint> > (ind2, ipv) );

        //cout << "add interface @ " << ind1 << ":" << ind2 << endl;

        m_m_addresses.insert( pair<Ipv4Address, uint> (ia, ind1) );
    }

    pair<double, double> FlywaysTopoHelper::GetNextRackLocation()
    {

        if( m_currClumpNumRacksReady == g_Options.clumpNumRacks ){
            // figure out where to place the next clump

            cout << "[Geo] NewClump-- finX/Y: " << m_finishedX << "/" << m_finishedY;
            cout << " currX/Y: " << m_currClumpX << "/" << m_currClumpY;
      
            double clumpEndX = m_currClumpX + m_currClumpNumRacksReady * g_Options.rackWidth_meter + g_Options.clumpSpaceLeft_meter;
            double clumpEndY = m_currClumpY + g_Options.rackHeight_meter + g_Options.clumpSpaceBelow_meter;

            cout << " endsX/Y " << clumpEndX << " " << clumpEndY << endl;

            if ( clumpEndX > m_finishedX )
                m_finishedX = clumpEndX;
            if ( clumpEndY > m_finishedY )
                m_finishedY = clumpEndY;

            cout << "[Geo]  ---> finsX/Y: " << m_finishedX << "/" << m_finishedY << endl;

            m_currClumpNumRacksReady = 0;

            if ( clumpEndX < m_finishedX ){
                // start a clump at the same y location but to the right
                m_currClumpX += g_Options.clumpSpaceLeft_meter + g_Options.clumpNumRacks * g_Options.rackWidth_meter;
                m_currClumpY = m_currClumpY; // unmodified

                cout << "[Geo] --> same row, move left  currX/Y: " << m_currClumpX << "/" << m_currClumpY << endl;
            }
            else if ( clumpEndY < m_finishedY ){
                // start a clump at the same x location but underneath
                m_currClumpX = m_currClumpX;
                m_currClumpY += g_Options.clumpSpaceBelow_meter + g_Options.rackHeight_meter;

                cout << "[Geo] --> same column, move down  currX/Y: " << m_currClumpX << "/" << m_currClumpY << endl;
            }
            else {
                // finished the rectangle, start a new row or a new column?
                if ( m_finishedY < m_finishedX ){
                    // start a new row of clumps
                    m_currClumpX = 0;
                    m_currClumpY += g_Options.clumpSpaceBelow_meter + g_Options.rackHeight_meter;

                    cout << "[Geo] --> new row  currX/Y: " << m_currClumpX << "/" << m_currClumpY << endl;

                }
                else{
                    // start a new column
                    m_currClumpX = m_finishedX;
                    m_currClumpY = 0;
                    cout << "[Geo] --> new col  currX/Y: " << m_currClumpX << "/" << m_currClumpY << endl;
                }
            }
        }
        NS_ASSERT_MSG ( m_currClumpNumRacksReady < g_Options.clumpNumRacks, "clump numbering trouble");

        // lets add a rack to current clump
        double xloc = m_currClumpX + m_currClumpNumRacksReady*g_Options.rackWidth_meter;
        double yloc = m_currClumpY;
    
        m_currClumpNumRacksReady ++;
        return pair<double, double> (xloc, yloc);

    }

    void FlywaysTopoHelper::AddDeviceAt (uint ind1, uint ind2, Ptr<NetDevice> nd){
        if ( ind1 < 0 || ind1 >= m_numnodes )
            NS_FATAL_ERROR("too large ind1 @ AddDeviceAt");

        if( m_v_devices.at(ind1) == NULL )
            m_v_devices.at(ind1) = new map< uint, Ptr<NetDevice>  > ();

        NS_ASSERT_MSG ( m_v_devices.at(ind1), "AddDeviceAt creating map" );

        m_v_devices[ind1]->insert( pair<uint, Ptr<NetDevice> > (ind2, nd) );
        //cout << "add device @ " << ind1  << ":" << ind2 << endl;
    }


    FlywaysTopoHelper::~FlywaysTopoHelper ()
    {
        // fill this out, for now memory leak
    }

    vector<uint32_t> * FlywaysTopoHelper::GetNbrsOfNode(uint32_t index)
    {
        vector<uint32_t>* p_retval = new vector<uint32_t> ();
        NS_ASSERT_MSG ( index <= m_numnodes, "index " << index << " > #nodes " << m_numnodes);

        if ( m_v_devices[index] == NULL ) 
            return p_retval;

        map<uint, Ptr<NetDevice> >::iterator mipit;
        for(mipit = m_v_devices[index]->begin(); mipit != m_v_devices[index]->end(); mipit++) {
            p_retval->push_back( (*mipit).first );
        }
        return p_retval;
    }

    Ptr<NetDevice> FlywaysTopoHelper::GetDevice(uint32_t index, int32_t nbr)
    {
        if ( index < 0 || index >= m_numnodes || m_v_devices[index] == NULL )
        {
            NS_FATAL_ERROR ("Index not valid in FlywaysTopoHelper::GetDevice.");
            return NULL;
        }

        if ( nbr < -1 || 
                (nbr == -1 && m_v_devices[index] == NULL) || 
                (nbr >= 0 && ( (uint) nbr >= m_numnodes || m_v_devices[index] == NULL || m_v_devices[index]->find(nbr) == m_v_devices[index]->end() ))) {
            NS_FATAL_ERROR ("Nbr not valid in FlywaysTopoHelper::GetDevice.");
            return NULL;
        }


        if ( nbr == -1 ) 
            return m_v_devices[index]->begin()->second;
        else
        return m_v_devices[index]->find(nbr)->second;
    }

    Ptr<Node> FlywaysTopoHelper::GetNode(uint32_t index)
    {
        if ( index < 0 || index >= m_numnodes )
            NS_FATAL_ERROR("Index not valid in FlywaysTopoHelper::GetNode.");

        return m_nodes.Get(index);
    }


    Ipv4Address FlywaysTopoHelper::GetIpv4Address (uint32_t index, int32_t nbr)
    {
        if ( index < 0 || index >= m_numnodes ||
            m_v_interfaces[index] == NULL ){
            NS_FATAL_ERROR ("Index not valid in FlywaysTopoHelper::GetIpv4Address.");
            return NULL;
        }
        if (nbr < -1 || 
            (nbr == -1 && m_v_interfaces[index] == NULL) ||
            (nbr >= 0 && ((uint) nbr >= m_numnodes || m_v_interfaces[index] == NULL || m_v_interfaces[index]->find(nbr) == m_v_interfaces[index]->end()))){
            NS_FATAL_ERROR ("Nbr not valid in FlywaysTopoHelper::GetIpv4Address.");
            return NULL;
        }

        pair<Ptr<Ipv4>, uint32_t> result;
        if (nbr == -1) {
            result =  m_v_interfaces[index]->begin()->second;
        }
        else {
            result =  m_v_interfaces[index]->find(nbr)->second;
        }
        return (result.first)->GetAddress(result.second, 0).GetLocal();
    }

    int FlywaysTopoHelper::GetNumNodes() const
    {
        return m_numnodes;
    }

    uint FlywaysTopoHelper::GetNumToRs() const
    {
        return m_numtors;
    }

    uint FlywaysTopoHelper::GetNumDongles() const
    {
        return m_numdongles;
    }

    int FlywaysTopoHelper::GetNumApps() const
    {

        return m_v_apps.size();
    }

    int FlywaysTopoHelper::GetNumFlyways() const
    {
        return flywayLinks.size();
    }

    uint64_t FlywaysTopoHelper::GetTotalRx(int i)
    {
        if (i < 0 || i >= GetNumApps()) 
            return 0; 
        else  
            return m_v_apps[i]->GetTotalRx();
    }

    bool FlywaysTopoHelper::IsFinished(int i)
    {
        if (i < 0 || i >= GetNumApps()) 
            return false; 
        else  
            return m_v_apps[i]->IsFinished();
    }

    Time FlywaysTopoHelper::WhenFinished(int i)
    {
        if (i < 0 || i >= GetNumApps()) 
            return Seconds(0); 
        else  
            return m_v_apps[i]->WhenFinished();
    }

    double FlywaysTopoHelper::TotalDelay(int i)
    {
        if (i < 0 || i >= GetNumApps()) 
            return 0; 
        else  
            return m_v_apps[i]->TotalDelay();
    }

    double FlywaysTopoHelper::CalcSignalStrength(int from, uint dongleFrom, int to, uint dongleTo)
    {
        Ptr<YansWifiPhy> sender = GetFlywayPhy(from, dongleFrom);
        Ptr<YansWifiPhy> receiver = GetFlywayPhy(to, dongleTo);

        Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
        Ptr<MobilityModel> receiverMobility = receiver->GetMobility ()->GetObject<MobilityModel> ();

        Vector sender_pos = senderMobility->GetPosition();
        Vector receiver_pos = receiverMobility->GetPosition();

        Ptr<Antenna> sender_ant = sender->GetAntenna();
        Ptr<Antenna> receiver_ant = receiver->GetAntenna();

        double azimuth = CalculateAzimuthAngle( sender_pos, receiverMobility->GetPosition());
	double elevation = CalculateElevationAngle( sender_pos, receiverMobility->GetPosition());

        Ptr<YansWifiChannel> channel = DynamicCast<YansWifiChannel>(sender->GetChannel());
        Ptr<PropagationLossModel> loss = channel->GetPropagationLossModel();

        /*
        cout << "azimuuth=" << azimuth << endl;
        cout << "elevation=" << elevation << endl;
        cout << "power=" << sender->GetTxPowerEnd() << endl;
        cout << "afterloss=" << loss->CalcRxPower (sender->GetTxPowerEnd(), senderMobility, receiverMobility) << endl;
        cout << "sg=" << sender_ant->GetTxGainDbi(azimuth, elevation) << endl;
        cout << "rg=" <<  receiver_ant->GetRxGainDbi(azimuth+M_PI, -elevation) << endl;
        */

        double rxPowerDbm = loss->CalcRxPower (sender->GetTxPowerEnd(), senderMobility, receiverMobility) +
		  sender_ant->GetTxGainDbi(azimuth, elevation) +
		  receiver_ant->GetRxGainDbi(azimuth+M_PI, -elevation);
	//cout << "rxPowerDbm=" << rxPowerDbm << " loss=" << loss->CalcRxPower (sender->GetTxPowerEnd(), senderMobility, receiverMobility) << " txgain=" << sender_ant->GetTxGainDbi(azimuth, elevation) << " rxgain=" << receiver_ant->GetRxGainDbi(azimuth+M_PI, -elevation) << endl;

        return rxPowerDbm;
    }

    Ptr<YansWifiPhy> FlywaysTopoHelper::GetFlywayPhy(uint nodeId, uint dongleGroup)
    {
        NetDeviceContainer flywayLinks = GetFlywayLinks(nodeId);
        Ptr<WifiNetDevice> flywayLink = DynamicCast<WifiNetDevice>(flywayLinks.Get(dongleGroup));
        Ptr<YansWifiPhy> phy = DynamicCast<YansWifiPhy>(flywayLink->GetPhy());
        return phy; 
    }

    void FlywaysTopoHelper::ChangeChannel(uint nodeId, uint dongleGroup, uint16_t channelNumber)
    {
        Ptr<YansWifiPhy> phy = GetFlywayPhy(nodeId, dongleGroup); 
        phy->SetChannelNumber(channelNumber);
    }

    void FlywaysTopoHelper::SetAntennaAzimuthAngle(uint nodeId, uint dongleGroup, double angle)
    {
        Ptr<YansWifiPhy> phy = GetFlywayPhy(nodeId, dongleGroup); 
        if (g_Options.antenna == FW_Antenna_Cone) 
        {
            Ptr<ConeAntenna> ca = DynamicCast<ConeAntenna>(phy->GetAntenna());
            ca->SetAzimuthAngle(angle);
        }
        else if (g_Options.antenna == FW_Antenna_Measured)
        {
            Ptr<Measured2DAntenna> ca = DynamicCast<Measured2DAntenna>(phy->GetAntenna());
            ca->SetAzimuthAngle(angle);
        }
    }

    void FlywaysTopoHelper::SetAntennaElevationAngle(uint nodeId, uint dongleGroup, double angle)
    {
        Ptr<YansWifiPhy> phy = GetFlywayPhy(nodeId, dongleGroup); 
        if (g_Options.antenna == FW_Antenna_Cone) 
        {
            Ptr<ConeAntenna> ca = DynamicCast<ConeAntenna>(phy->GetAntenna());
            ca->SetElevationAngle(angle);
        }
        else if (g_Options.antenna == FW_Antenna_Measured)
        {
            Ptr<Measured2DAntenna> ca = DynamicCast<Measured2DAntenna>(phy->GetAntenna());
            ca->SetElevationAngle(angle);
        }
    }

    void FlywaysTopoHelper::SetAntennaBeamwidth(uint nodeId, uint dongleGroup, double beamwidth)
    {
        Ptr<YansWifiPhy> phy = GetFlywayPhy(nodeId, dongleGroup); 
        if (g_Options.antenna == FW_Antenna_Measured) 
        {
            printf ("Cannot set beamwidth on measured antenna\n");
            exit(-1);
        }
        Ptr<ConeAntenna> ca = DynamicCast<ConeAntenna>(phy->GetAntenna());
        ca->SetBeamwidth(beamwidth);
    }

    void FlywaysTopoHelper::SetAntennaGain(uint nodeId, uint dongleGroup, double gain)
    {
        Ptr<YansWifiPhy> phy = GetFlywayPhy(nodeId, dongleGroup); 
//	std::cout << "nodeId=" << nodeId << " MAC=" << Mac48Address::ConvertFrom(DynamicCast<WifiNetDevice>(phy->GetDevice())->GetAddress()) << std::endl;
        if (g_Options.antenna == FW_Antenna_Cone) 
        {
            Ptr<ConeAntenna> ca = DynamicCast<ConeAntenna>(phy->GetAntenna());
            ca->SetGainDbi(gain);
        }
        else if (g_Options.antenna == FW_Antenna_Measured)
        {
            Ptr<Measured2DAntenna> ca = DynamicCast<Measured2DAntenna>(phy->GetAntenna());
            ca->SetMode(gain);
        } 
    }

    double FlywaysTopoHelper::GetAngle(double x1, double y1, double x2, double y2)
    {
        double dx = x2-x1; 
        double dy = y2-y1;
        return atan2(dy, dx);
    }

    void FlywaysTopoHelper::PointNodeTo(uint nodeId, uint dongleGroup, double x, double y)
    {
        double angle = GetAngle(m_v_nodeXYLocations_meter[nodeId].first, m_v_nodeXYLocations_meter[nodeId].second, x, y);
        SetAntennaAzimuthAngle(nodeId, dongleGroup, angle);
        SetAntennaElevationAngle(nodeId, dongleGroup, 0);
    }

    void FlywaysTopoHelper::PointAtEachOther(uint node1, uint dongleId1, uint node2, uint dongleId2)
    {
        PointNodeTo(node1, dongleId1, m_v_nodeXYLocations_meter[node2].first, m_v_nodeXYLocations_meter[node2].second);
        PointNodeTo(node2, dongleId2, m_v_nodeXYLocations_meter[node1].first, m_v_nodeXYLocations_meter[node1].second);
    }

    void FlywaysTopoHelper::PointNowhere(uint nodeId, uint dongleId)
    {
        SetAntennaAzimuthAngle(nodeId, dongleId, 0.123);
        SetAntennaElevationAngle(nodeId, dongleId, M_PI/2);
        if (g_Options.antenna == FW_Antenna_Cone) 
        {
            SetAntennaBeamwidth(nodeId, dongleId, 0.01);
        }
    }
    
    double FlywaysTopoHelper::Distance (pair<double, double> a, pair<double, double> b)
    {
        return sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
    }

    double FlywaysTopoHelper::GetDistance(int node1, int node2)
    {
        return Distance(m_v_nodeXYLocations_meter[node1], m_v_nodeXYLocations_meter[node2]);
    }

} // namespace ns3
