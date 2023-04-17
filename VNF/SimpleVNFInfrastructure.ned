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

import simu5g.slicing.VNF.Queue.IVNFMessageQueue;
import simu5g.slicing.liveConfigurations.LiveConfigurationsProvider;
import simu5g.slicing.liveConfigurations.MaxResourcesProvider;
import simu5g.slicing.liveConfigurations.SliceVNFMapProvider;

module SimpleVNFInfrastructure {
    parameters:
        int vnfCount = default(1);
    
    gates:
        input notifyPacketInLowerLayer[] @directIn;
        
        // Up link
        input readyForUpperLayerUpLink[];
        output toUpperLayerUpLink[];
        
        // Down link
        input readyForUpperLayerDownLink[];
        output toUpperLayerDownLink[];
        
    submodules:
        vnfManager: SimpleVNFManager {
            gates:
            	toVnfGate[vnfCount];
            	deallocateResourceToVnf[vnfCount];
            	// New gate for updating vnfResourceUpdate
            	maxResourcesConfigUpdateFromVnf[vnfCount];
            	maxResourcesConfigRequestToVnf[vnfCount];
        };
        
        vnf[vnfCount]: SimpleVNF;
        
    connections allowunconnected: 
        
        for i=0..sizeof(readyForUpperLayerUpLink)-1 {
            readyForUpperLayerUpLink[i] --> vnfManager.readyForUpperLayerUpLink++;
        	toUpperLayerUpLink[i] <-- vnfManager.toUpperLayerUpLink++ if sizeof(toUpperLayerUpLink) > 0;
        }
        
        for i=0..sizeof(notifyPacketInLowerLayer)-1 {
            notifyPacketInLowerLayer[i] --> vnfManager.notifyPacketInLowerLayer++;
        }
        
        for i =0..sizeof(readyForUpperLayerDownLink)-1 {
            readyForUpperLayerDownLink[i] --> vnfManager.readyForUpperLayerDownLink++;
        	toUpperLayerDownLink[i] <-- vnfManager.toUpperLayerDownLink++ if sizeof(toUpperLayerDownLink) > 0;
        }
        
        for i=0..vnfCount-1 {
            vnfManager.toVnfGate[i] --> vnf[i].validateResources;
            vnfManager.deallocateResourceToVnf[i] --> vnf[i].deallocateResources;
            vnfManager.maxResourcesConfigRequestToVnf[i] --> vnf[i].maxResourcesConfigRequest;
            vnfManager.maxResourcesConfigUpdateFromVnf[i] <-- vnf[i].maxResourcesConfigUpdate; 
            vnfManager.allocationCompleted++ <-- vnf[i].allocationCompleted;
        }
}

module SimpleVNFManager {
    parameters:
        @class(SimpleVNFManager);
        string upLinkSourceIpSliceMap = default("10=0 11=1 12=2 13=3"); // SourceIpByte=SliceNumber
        string downLinkSourceIpSliceMap = default("0=0 1=1 2=2 3=3"); // SourceIpByte=SliceNumber
        string sliceVnfGateIndexMap = default("0=0 1=0 2=0 3=0"); // SliceNumber=VnfGateIndex
        int upLinkSourceByteNumber = default(1);
        int downLinkSourceByteNumber = default(2);
        bool useWeightedRoundRobinLoadBalancer = default(true);
             
    gates:
        input allocationCompleted[] @directIn;
        output toVnfGate[];
        output deallocateResourceToVnf[];
        output maxResourcesConfigRequestToVnf[];
        input maxResourcesConfigUpdateFromVnf[] @directIn;
        input notifyPacketInLowerLayer[] @directIn;
        
        // Up link
        input readyForUpperLayerUpLink[];
        output toUpperLayerUpLink[];
        
        // Down link
        input readyForUpperLayerDownLink[];
        output toUpperLayerDownLink[];
        
    submodules:
        sliceVnfMapProvider: SliceVNFMapProvider;
}


module SimpleVNF {
    double maxRam @unit("B") = default(32GB);
    double maxStorage @unit("B") = default(100TB);
    double maxCpu = default(1);	
    string maxResourcesFilePath = default("");
    double k = default(0.7);
    double g = default(0.7);
    double h = default(0.7);
    
    
    @class(SimpleVNF);
    @signal[cpuChanges](type="double");
    @signal[ramChanges](type="double");
    @signal[storageChanges](type="double");
    
    @statistic[cpuValue](title="CPU"; source=cpuChanges; record=vector,stats; interpolationmode=none);
    @statistic[ramValue](title="RAM"; source=ramChanges; record=vector,stats; interpolationmode=none);
    @statistic[storageValue](title="Storage"; source=storageChanges; record=vector,stats; interpolationmode=none);
    
    gates:
        input validateResources @directIn;
        input deallocateResources @directIn;
        input maxResourcesConfigRequest @directIn;
        output maxResourcesConfigUpdate;
        output allocationCompleted;
        
    submodules:
        queue: <default("FifoVNFMessageQueue")> like IVNFMessageQueue;
       
       	maxResourcesProvider: MaxResourcesProvider {
       	    filePath = maxResourcesFilePath;
       	}
}
