/*
 * BaseVNFMessageQueue.h
 *
 *  Created on: May 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_BASEVNFMESSAGEQUEUE_H_
#define NODES_VNF_BASEVNFMESSAGEQUEUE_H_


#include <omnetpp.h>
#include "./IVNFMessageQueue.h"

class BaseVNFMessageQueue : public cSimpleModule, public IVNFMessageQueue
{
    protected:
        simsignal_t messageQueuedSignal = 0;
        simsignal_t messageDroppedSignal = 0;
        simsignal_t messageQueueSizeSignal = 0;
        double packetTTLInVNF = 1;

    protected:
        void addMessage(VNFMessage *message) override;
        virtual void doAddMessage(VNFMessage* message);
        virtual VNFMessage* doGetNextMessage();
        virtual VNFMessage* getNextMessage() override;
        virtual const VNFMessage* peekNextMessage() override;
        virtual int getSize() override;
        virtual bool removeMessageFromQueue(long packetTreeId);
        void handleMessage(cMessage *msg) override;

    private:
        void scheduleVNFMessageRemoval(long packetTreeId);
};

#endif /* NODES_VNF_BASEVNFMESSAGEQUEUE_H_ */
