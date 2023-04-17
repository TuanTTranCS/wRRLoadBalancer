/*
 * BaseVNFMessageQueue.cc
 *
 *  Created on: May 27, 2021
 *      Author: vibisson
 */


#include "BaseVNFMessageQueue.h"
#include "./Messages/PacketRemoval_m.h"



void BaseVNFMessageQueue::addMessage(VNFMessage *message)
{
    Enter_Method("addMessage");

    if(messageQueuedSignal == 0)
    {
        throw cRuntimeError("messageQueuedSignal is null");
    }

    doAddMessage(message);

    emit(messageQueuedSignal, 1);
    emit(messageQueueSizeSignal, getSize());

    scheduleVNFMessageRemoval(message->getPck()->getTreeId());
}

void BaseVNFMessageQueue::scheduleVNFMessageRemoval(long packetTreeId)
{
    PacketRemoval* packetRemovalMessage = new PacketRemoval();
    packetRemovalMessage->setPacketTreeId(packetTreeId);

    scheduleAt(simTime() + packetTTLInVNF, packetRemovalMessage);
}

void BaseVNFMessageQueue::handleMessage(omnetpp::cMessage* message)
{
    if (message->isSelfMessage())
    {
        PacketRemoval *packetRemovalMessage = check_and_cast<PacketRemoval*>(message);

        bool removalSucceed = removeMessageFromQueue(packetRemovalMessage->getPacketTreeId());

        if (removalSucceed)
        {
            emit(messageDroppedSignal, 1);
        }

        delete packetRemovalMessage;
    }
}

VNFMessage* BaseVNFMessageQueue::getNextMessage()
{
    VNFMessage* message = doGetNextMessage();

    emit(messageQueueSizeSignal, getSize());

    return message;
}

void BaseVNFMessageQueue::doAddMessage(VNFMessage *message)
{
    throw cRuntimeError("addMessage is not implemented");
}

VNFMessage* BaseVNFMessageQueue::doGetNextMessage()
{
    throw cRuntimeError("doGetNextMessage is not implemented");
}

const VNFMessage* BaseVNFMessageQueue::peekNextMessage()
{
    throw cRuntimeError("peekNextMessage is not implemented");
}

int BaseVNFMessageQueue::getSize()
{
    throw cRuntimeError("getSize is not implemented");
}

bool BaseVNFMessageQueue::removeMessageFromQueue(long packetTreeId)
{
    throw cRuntimeError("removeMessageFromQueue is not implemented");
}
