/*
 * SimpleVNF.cc
 *
 *  Created on: Apr. 27, 2021
 *      Author: vibisson
 */



#include "SimpleVNFManager.h"

#include <string>
#include <iostream>
#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "stack/ip2nic/IP2Nic.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "./LoadBalancing/RoundRobin/RoundRobinLoadBalancer.h"
#include "./LoadBalancing/WeightedRoundRobin/WeightedRoundRobinLoadBalancer.h"
#include <typeinfo>

Define_Module(SimpleVNFManager)

void SimpleVNFManager::initialize(int stage)
{
    upLinkSourceByteNumber = par("upLinkSourceByteNumber");
    downLinkSourceByteNumber = par("downLinkSourceByteNumber");
    useWeightedRoundRobinLoadBalancer = par("useWeightedRoundRobinLoadBalancer");

    std::cout << "Initializing SimpleVNFManager" << std::endl;

    sliceVnfMapProvider = check_and_cast<SliceVNFMapProvider*>(getSubmodule("sliceVnfMapProvider"));
    sliceVnfMapProvider->setVNFManager(this);
    std::string loadBalancerType;

    // Tuan Hugo: change load balancer into WeightedRoundRobinLoadBalancer
    //loadBalancer = new RoundRobinLoadBalancer();
    if (useWeightedRoundRobinLoadBalancer)
    {
        loadBalancer = new WeightedRoundRobinLoadBalancer();
        loadBalancerType = "WeightedRoundRobinLoadBalancer";
    }
    else
    {
        loadBalancer = new RoundRobinLoadBalancer();
        loadBalancerType = "RoundRobinLoadBalancer";
    }


    std::cout << "Load balancer type: " << loadBalancerType << std::endl;
    initializeSourceSliceMaps();
    initializeVnfGateIndexSliceMap();
}

void SimpleVNFManager::handleMessage(cMessage *msg)
{
    int arrivalIndex = msg->getArrivalGate()->getIndex();

    if (msg->arrivedOn("readyForUpperLayerUpLink"))
    {
        inet::Packet* packet = check_and_cast<inet::Packet*>(msg);

        int vnfGateIndex = getVnfGateIndexForPacket(packet, VNFDirection::UpLink);
        handleReadyForUpperLayerMessage(packet, VNFDirection::UpLink, vnfGateIndex, arrivalIndex);
    }
    else if (msg->arrivedOn("readyForUpperLayerDownLink"))
    {
        inet::Packet* packet = check_and_cast<inet::Packet*>(msg);

        int vnfGateIndex = getVnfGateIndexForPacket(packet, VNFDirection::DownLink);
        handleReadyForUpperLayerMessage(packet, VNFDirection::DownLink, vnfGateIndex, arrivalIndex);

    }
    else if (msg->arrivedOn("allocationCompleted"))
    {
        VNFMessage *vnfMessage = check_and_cast<VNFMessage*>(msg);
        handleResourceAllocatedMessage(vnfMessage);
    }
    else if (msg->arrivedOn("notifyPacketInLowerLayer") && msg->isPacket())
    {
        inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
        handleNotificationPacketIsInLowerLayer(packet);
    }
    else if (msg->arrivedOn("maxResourcesConfigUpdateFromVnf"))
    {
        Vnf2VnfmMessage *vnf2VnfmMessage = check_and_cast<Vnf2VnfmMessage*>(msg);
        handleMaxResourcesConfigUpdateMessage(vnf2VnfmMessage);
        take(msg);

        delete vnf2VnfmMessage;
    }

}

void SimpleVNFManager::handleReadyForUpperLayerMessage(inet::Packet* packet, VNFDirection direction, int vnfGateIndex, int arrivalIndex)
{
    inet::Packet* clonePacket = packet->dup();

    VNFMessage *msg = new VNFMessage();
    msg->setDirection(direction);
    msg->setPck(clonePacket);
    msg->setArrivalIndex(arrivalIndex);

    long packetId = packet->getTreeId();
    packetIdVnfGateIndexMap[packetId] = vnfGateIndex;

    sendDirect(msg, gate("toVnfGate", vnfGateIndex));

    delete packet;
}

void SimpleVNFManager::handleResourceAllocatedMessage(VNFMessage *vnfMessage)
{
    // We need to send the packet to the upper layer in gate of the PPPInterface
    inet::Packet *packet = vnfMessage->getPck()->dup();

    EV << "About to send packet to the upper layer with id " << packet->getTreeId() << std::endl;

    if (vnfMessage->getDirection() == VNFDirection::UpLink)
    {
        if (gateSize("toUpperLayerUpLink") > 0)
        {
            send(packet, gate("toUpperLayerUpLink", vnfMessage->getArrivalIndex()));
        }
    }
    else
    {
        if (gateSize("toUpperLayerDownLink") > 0)
        {
            send(packet, gate("toUpperLayerDownLink", vnfMessage->getArrivalIndex()));
        }
    }

    delete vnfMessage->getPck();
    delete vnfMessage;
}

void SimpleVNFManager::handleNotificationPacketIsInLowerLayer(inet::Packet *packet)
{
    int vnfGateIndex = packetIdVnfGateIndexMap[packet->getTreeId()];

    packetIdVnfGateIndexMap.erase(packet->getTreeId());

    EV << "Just received notification packet arrived. Id: " << packet->getTreeId() << " VnfGateIndex: " << vnfGateIndex << std::endl;
    EV << std::endl;

    sendDirect(packet, gate("deallocateResourceToVnf", vnfGateIndex));
}

// Tuan: method to send max vnf resource configs received from VNF --> VNFManager to the WeightedRoundRobin load balancer
void SimpleVNFManager::handleMaxResourcesConfigUpdateMessage(Vnf2VnfmMessage *vnf2VnfmMessage)
{
    if (useWeightedRoundRobinLoadBalancer)
    {
        int vnfGateIndex = vnf2VnfmMessage->getArrivalGate()->getIndex();
            double maxCpu = vnf2VnfmMessage->getMaxCpu();
            double maxRam = vnf2VnfmMessage->getMaxRam();
            double maxStorage = vnf2VnfmMessage->getMaxStorage();


            // Find the sliceNumber for current vnfGateIndex
            int sliceNumber = -1;
            for (auto it = sliceVnfGateIndexMap.begin(); it != sliceVnfGateIndexMap.end(); it++)
            {
                //it->first    // string (key)
                // it->second   // string's value

                std::vector<int> vnfIndexes = it->second;
                if (std::find(vnfIndexes.begin(), vnfIndexes.end(), vnfGateIndex) != vnfIndexes.end())
                {
                    sliceNumber = it->first;
                    break;
                }

            }
            if (sliceNumber > -1)
            {
                /*
                std::cout << "SimpleVNFManager received and sending max resources config "
                        << "from SimpleVnf to load balancer on sliceNumber: " << sliceNumber
                        << " - VNFGateIndex: " << vnfGateIndex << " - Max CPU: " << maxCpu
                        << " - Max RAM: " << maxRam << " - Max storage: " << maxStorage  << std::endl;
                */
                loadBalancer->updateMaxResourceConfigs(sliceNumber, vnfGateIndex, maxCpu, maxRam, maxStorage);
            }
            else
            {
                /*
                std::cout << "SimpleVNFManager received max resources config "
                            << "from SimpleVnf - VNFGateIndex: " << vnfGateIndex << " - Max CPU: " << maxCpu
                            << " - Max RAM: " << maxRam << " - Max storage: " << maxStorage
                            << ". This VNF is not used by any slice yet." << std::endl;
                            */
            }

            //delete vnf2VnfmMessage;
    }


}

int SimpleVNFManager::getVnfGateIndexForPacket(inet::Packet *packet, VNFDirection direction)
{
    int sliceNumber;

    const auto& ipv4Header = packet->peekAtFront<inet::Ipv4Header>();
    const inet::Ipv4Address &srcAddr = ipv4Header->getSrcAddress();

    EV << "Just received packet with ip " << srcAddr.str() << ". ID: " << packet->getTreeId() << std::endl;

    if(direction == VNFDirection::UpLink)
    {
        int byte = srcAddr.getDByte(upLinkSourceByteNumber);
        EV << "Byte value: " << byte << std::endl;
        sliceNumber = upLinkSourceIpSliceMap[byte];
    }
    else
    {
        int byte = srcAddr.getDByte(downLinkSourceByteNumber);
        EV << "Byte value: " << byte << std::endl;
        sliceNumber = downLinkSourceIpSliceMap[byte];
    }

    EV << "This ip is for slice " << sliceNumber << std::endl;

    int vnfGateIndex = loadBalancer->getVNFGateIndex(sliceNumber);

    EV << "This packet need to go on vnf gate " << vnfGateIndex << std::endl;
    EV << std::endl;

    return vnfGateIndex;
}

void SimpleVNFManager::initializeVnfGateIndexSliceMap()
{
    const char* vnfGateIndexSliceMapConfig = par("sliceVnfGateIndexMap");

    const char* delimiter = "=";

    std::vector<std::string> sliceVnfGateIndexes = omnetpp::cStringTokenizer(vnfGateIndexSliceMapConfig).asVector();

    for(std::vector<std::string>::iterator it = sliceVnfGateIndexes.begin(); it != sliceVnfGateIndexes.end(); ++it)
    {
        std::vector<std::string> sliceVnfGateIndex = omnetpp::cStringTokenizer((*it).c_str(), delimiter).asVector();

        int sliceNumber = std::stoi(sliceVnfGateIndex[0]);
        std::vector<int> vnfGateIndexes = omnetpp::cStringTokenizer(sliceVnfGateIndex[1].c_str(), ";").asIntVector();

        sliceVnfGateIndexMap[sliceNumber] = vnfGateIndexes;

        loadBalancer->updateVNFGateIndexes(sliceNumber, sliceVnfGateIndexMap[sliceNumber]);

        std::cout << "Slice number: " << sliceNumber << ", VNF gate indexes: " << sliceVnfGateIndex[1] << std::endl;
    }

    std::cout << std::endl;
}

void SimpleVNFManager::initializeSourceSliceMaps()
{
    const char* upLinkSourceIpSliceMapConfig = par("upLinkSourceIpSliceMap");
    const char* downLinkSourceIpSliceMapConfig = par("downLinkSourceIpSliceMap");

    parseSourceIpConfig(&upLinkSourceIpSliceMap, upLinkSourceIpSliceMapConfig);
    parseSourceIpConfig(&downLinkSourceIpSliceMap, downLinkSourceIpSliceMapConfig);
}

void SimpleVNFManager::parseSourceIpConfig(std::map<int, int> *sourceIpSliceMap, const char* config)
{
    const char* delimiter = "=";

    std::vector<std::string> srcIpSlices = omnetpp::cStringTokenizer(config).asVector();

    for(std::vector<std::string>::iterator it = srcIpSlices.begin(); it != srcIpSlices.end(); ++it)
    {
        std::vector<std::string> srcIpSlice = omnetpp::cStringTokenizer((*it).c_str(), delimiter).asVector();

        int srcIpByte = std::stoi(srcIpSlice[0]);
        int sliceNumber = std::stoi(srcIpSlice[1]);

        EV << "Source ip byte: " << srcIpByte << ", SliceNumber: " << sliceNumber << std::endl;

        (*sourceIpSliceMap)[srcIpByte] = sliceNumber;
    }

    EV << std::endl;
}


void SimpleVNFManager::notifySliceChangedVNF(int sliceNumber, std::vector<int> vnfGateIndexes)
{
    // Tuan: Use Enter_Method here to enable macro to send request to SimpleVNF retrieve max VNF Resources config
    Enter_Method("notifySliceChangedVNF");

    //std::vector<int> currentVnfGateIndexes;


    //take(msg);

    //currentVnfGateIndexes = sliceVnfGateIndexMap[sliceNumber];
    //if (currentVnfGateIndexes != vnfGateIndexes)
    //{
        /*
        std::cout << "VNF mapping changed on slice " << sliceNumber << std::endl;
                std::cout << "    Current VNF(s): ";

        for (int vnfIndex: currentVnfGateIndexes){
            std::cout << vnfIndex << " ";
        }
        std::cout << std::endl;
        std::cout << "    New VNF(s): ";
        for (int vnfIndex: vnfGateIndexes){
                std::cout << vnfIndex << " ";
            }
        std::cout << std::endl;*/
    //}


    sliceVnfGateIndexMap[sliceNumber] = vnfGateIndexes;

    // Pass new vnfGateIndexes to load balancer to update
    loadBalancer->updateVNFGateIndexes(sliceNumber, vnfGateIndexes);
    //int existedVnfIndexCount = 0;

    if (useWeightedRoundRobinLoadBalancer)
    {
        // For each vnfGateIndex, send the request to VnfSimple to get the max resource config
            for(std::vector<int>::const_iterator it = vnfGateIndexes.begin(); it != vnfGateIndexes.end(); ++it)
            {
                //if (std::find(currentVnfGateIndexes.begin(), currentVnfGateIndexes.end(), *it) == currentVnfGateIndexes.end())
                //{
                    //newVnfGateIndexes.push_back(*it);
                    //Send request message to SimpleVNF on the new vnfGateIndex (*it)
                    Vnf2VnfmMessage *msg = new Vnf2VnfmMessage();
                    /*
                    std::cout << " Sending maxResourcesConfigRequest to new VNF#" << *it
                            << " due to VNF map change on slice#" << sliceNumber << std::endl;
                    */
                    sendDirect(msg, gate("maxResourcesConfigRequestToVnf", *it));
                //}
                //else
                //{
                    //existedVnfIndexCount++;
                //}
            }
            // Check case that 1 or more vnfGateIndex is removed (no new VNF added), just send request to update VNF index probs only
            //if ((existedVnfIndexCount == vnfGateIndexes.size()) && vnfGateIndexes.size() < currentVnfGateIndexes.size())
            //{
                //loadBalancer->requestUpdateVnfIndexSelectProb(sliceNumber);
            //}
    }


    //delete msg;//, currentVnfGateIndexes;
}






















