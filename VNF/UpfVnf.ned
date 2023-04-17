//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package simu5g.slicing.VNF;

import inet.common.MessageDispatcher;
import inet.linklayer.ppp.PppInterface;
import inet.transportlayer.udp.Udp;
import inet.node.base.NetworkLayerNodeBase;
import simu5g.corenetwork.trafficFlowFilter.TrafficFlowFilter;
import simu5g.slicing.VNF.SimpleVNFInfrastructure;
import simu5g.slicing.gtp.SliceGtpUser;

module UpfVnf extends NetworkLayerNodeBase
{
    parameters:
        @display("bgb=1350,750;i=device/mainframe");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=1000,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=1245,11; anchor=ne; text="application layer");
        @figure[transportLayer](type=rectangle; pos=250,156; size=1000,130; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=1245,161; anchor=ne; text="transport layer");
        @figure[submodules];

        string nodeType = "UPF";
        forwarding = default(true);
        multicastForwarding = default(false);
        // numPppInterfaces = 1;
        //# Slincing configurations
        string gNodeB1 = default("none");
        string gNodeB2 = default("none");
        string gNodeB3 = default("none");
        string gNodeB4 = default("none");
        string gNodeB5 = default("none");
        string gNodeB6 = default("none");
        string gNodeB7 = default("none");
        string gNodeB8 = default("none");
        string destinationModuleNames = default("none");
        string srcIdSliceMapping = default("none");

    gates:
        inout filterGate @labels(PppFrame-conn);
        output filterGateOut;
        input filterGateIn;

    submodules:
        pppIf: PppInterface {
            parameters:
                @display("p=1150,525");
        }
        udp: Udp {
            parameters:
                @display("p=329,206");
        }
        trafficFlowFilter: TrafficFlowFilter {
            parameters:
                @display("p=675,80");
                ownerType = nodeType;
        }
        gtp_user: SliceGtpUser {
            parameters:
                @display("p=525,80");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=750,296;b=1000,5,,,,1");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
        vnfInfrastructure: SimpleVNFInfrastructure {
            parameters:
                @display("p=878.4075,93.0875");
        }

    connections:

        gtp_user.socketOut --> at.in++;
        gtp_user.socketIn <-- at.out++;

        pppIf.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        
        // Up link
        vnfInfrastructure.readyForUpperLayerUpLink++ <-- gtp_user.pppGate;
        vnfInfrastructure.toUpperLayerUpLink++ --> pppIf.upperLayerIn;
        pppIf.notifyPacketInLowerLayer --> vnfInfrastructure.notifyPacketInLowerLayer++;
        
        // Down link
        for i=0..sizeof(pppg)-1 {
            ppp[i].notifyPacketInLowerLayer --> vnfInfrastructure.notifyPacketInLowerLayer++;
            ppp[i].toUpperLayerIn <-- vnfInfrastructure.toUpperLayerDownLink++;
            ppp[i].notifyReadyForUpperLayerIn --> vnfInfrastructure.readyForUpperLayerDownLink++;
        }

        trafficFlowFilter.gtpUserGateOut --> gtp_user.trafficFlowFilterGate;

        // at dispatcher to transport layer connections
        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;

        // transport layer to tn dispatcher connections
        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;

        // tn dispatcher to network layer connections
        tn.out++ --> ipv4.transportIn if hasIpv4;
        tn.in++ <-- ipv4.transportOut if hasIpv4;

        tn.out++ --> ipv6.transportIn if hasIpv6;
        tn.in++ <-- ipv6.transportOut if hasIpv6;

        tn.out++ --> generic.transportIn if hasGn;
        tn.in++ <-- generic.transportOut if hasGn;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;

        pppIf.phys <--> filterGate;
}