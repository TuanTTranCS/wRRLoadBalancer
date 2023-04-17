/*
 * SimpleVNF.h
 *
 *  Created on: Apr. 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_SIMPLEVNFMANAGER_H_
#define NODES_VNF_SIMPLEVNFMANAGER_H_


#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include <map>
#include "slicing/VNF/Messages/VNF_m.h"
#include "slicing/VNF/Messages/VNF2VNFM_m.h"
#include "slicing/liveConfigurations/SliceVNFMapProvider.h"
#include "./LoadBalancing/Base/ILoadBalancer.h"

using namespace omnetpp;

class SimpleVNFManager : public cSimpleModule, public IVNFManager
{
private:
    SliceVNFMapProvider* sliceVnfMapProvider = nullptr;
    ILoadBalancer* loadBalancer = nullptr;

protected:
    int upLinkSourceByteNumber;
    int downLinkSourceByteNumber;
    std::map<int, int> upLinkSourceIpSliceMap;
    std::map<int, int> downLinkSourceIpSliceMap;
    std::map<int, std::vector<int>> sliceVnfGateIndexMap;
    std::map<long, int> packetIdVnfGateIndexMap;

    void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void handleReadyForUpperLayerMessage(inet::Packet *packet, VNFDirection direction, int vnfGateIndex, int arrivalIndex);
    void handleResourceAllocatedMessage(VNFMessage *vnfMessage);
    void handleNotificationPacketIsInLowerLayer(inet::Packet *packet);
    void handleMaxResourcesConfigUpdateMessage(Vnf2VnfmMessage *vnf2VnfmMessage);
    bool useWeightedRoundRobinLoadBalancer;

private:
    void initializeSourceSliceMaps();
    void initializeVnfGateIndexSliceMap();
    int getVnfGateIndexForPacket(inet::Packet *packet, VNFDirection direction);
    void parseSourceIpConfig(std::map<int, int> *sourceIpSliceMap, const char* config);

public:
    void notifySliceChangedVNF(int sliceNumber, std::vector<int> vnfGateIndexes) override;
    //void sendMaxResourcesConfigRequests(std::vector<int> currentVnfGateIndexes, std::vector<int> newVnfGateIndexes);
};


#endif /* NODES_VNF_SIMPLEVNFMANAGER_H_ */
