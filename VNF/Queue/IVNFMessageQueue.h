/*
 * IVNFMessageQueue.h
 *
 *  Created on: May 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_IVNFMESSAGEQUEUE_H_
#define NODES_VNF_IVNFMESSAGEQUEUE_H_


#include <omnetpp.h>
#include "slicing/VNF/Messages/VNF_m.h"

using namespace omnetpp;

class IVNFMessageQueue
{
    public:
        virtual void addMessage(VNFMessage *message) = 0;
        virtual VNFMessage* getNextMessage() = 0;
        virtual const VNFMessage* peekNextMessage() = 0;
        virtual int getSize() = 0;
};


#endif /* NODES_VNF_IVNFMESSAGEQUEUE_H_ */
