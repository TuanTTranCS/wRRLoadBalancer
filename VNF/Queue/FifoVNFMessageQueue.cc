/*
 * FifoVNFMessageQueue.cc
 *
 *  Created on: May 27, 2021
 *      Author: vibisson
 */


#include "./FifoVNFMessageQueue.h"
#include <iostream>
#include <algorithm>

Define_Module(FifoVNFMessageQueue)

void FifoVNFMessageQueue::initialize(int stage)
{
    messageQueuedSignal = registerSignal("messageQueuedSignal");
    messageDroppedSignal = registerSignal("messageDroppedSignal");
    messageQueueSizeSignal = registerSignal("messageQueueSizeSignal");

    packetTTLInVNF = par("packetTTLInVNF").doubleValue();
}

void FifoVNFMessageQueue::doAddMessage(VNFMessage *message)
{
    vnfMessageQueue.push_back(message);
}

VNFMessage* FifoVNFMessageQueue::doGetNextMessage()
{
    if (vnfMessageQueue.size() > 0)
    {
        VNFMessage* nextMessage = vnfMessageQueue.front();
        vnfMessageQueue.erase(vnfMessageQueue.begin());

        return nextMessage;
    }

    return nullptr;
}


const VNFMessage* FifoVNFMessageQueue::peekNextMessage()
{
    if (vnfMessageQueue.size() > 0)
    {
        VNFMessage* vnfMessage = vnfMessageQueue.front();
        return vnfMessage;
    }

    return nullptr;
}

int FifoVNFMessageQueue::getSize()
{
    return vnfMessageQueue.size();
}

bool FifoVNFMessageQueue::removeMessageFromQueue(long packetTreeId)
{
    auto positionToErase = std::find_if(vnfMessageQueue.begin(), vnfMessageQueue.end(), [packetTreeId](VNFMessage* message)
    {
        if(message->getPck()->getTreeId() == packetTreeId)
        {
            return true;
        }

        return false;
    });


    if (positionToErase == vnfMessageQueue.end())
    {
        return false;
    }

    auto message = (*positionToErase);

    vnfMessageQueue.erase(positionToErase);

    delete message->getPck();
    delete message;

    std::cout << "Removing packet with id " << packetTreeId << " from the VNF queue" << std::endl;

    return true;
}
