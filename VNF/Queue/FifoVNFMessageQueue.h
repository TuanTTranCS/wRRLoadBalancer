/*
 * FifoVNFMessageQueue.h
 *
 *  Created on: May 27, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_QUEUE_FIFOVNFMESSAGEQUEUE_H_
#define NODES_VNF_QUEUE_FIFOVNFMESSAGEQUEUE_H_


#include "./BaseVNFMessageQueue.h"
#include <vector>

class FifoVNFMessageQueue : public BaseVNFMessageQueue
{
    private:
        std::vector<VNFMessage*> vnfMessageQueue;

    protected:
        void initialize(int stage) override;
        void doAddMessage(VNFMessage* message) override;
        VNFMessage* doGetNextMessage() override;
        int getSize() override;
        virtual const VNFMessage* peekNextMessage() override;
        bool removeMessageFromQueue(long packetTreeId) override;
};

#endif /* NODES_VNF_QUEUE_FIFOVNFMESSAGEQUEUE_H_ */
