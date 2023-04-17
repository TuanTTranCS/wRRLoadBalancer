/*
 * SimpleVNF.h
 *
 *  Created on: Apr. 29, 2021
 *      Author: vibisson
 */

#ifndef NODES_VNF_SIMPLEVNF_H_
#define NODES_VNF_SIMPLEVNF_H_


#include <omnetpp.h>
#include "./Messages/VNF_m.h"
#include "./Messages/VNF2VNFM_m.h"
#include "inet/common/packet/Packet.h"
#include "./Queue/IVNFMessageQueue.h"
#include "slicing/closedLoopClients/IClosedLoopClient.h"
#include "slicing/liveConfigurations/MaxResourcesProvider.h"

using namespace omnetpp;

class SimpleVNF : public cSimpleModule, public IVNF
{
private:
    simsignal_t cpuChangesSignal;
    simsignal_t ramChangesSignal;
    simsignal_t storageChangesSignal;

    IClosedLoopClient* closedLoopClient;

    double k;
    double g;
    double h;

private:
    bool enoughResources(double requiredCpu, double requiredRam, double requiredStorage);
    void checkNextMessageInQueue();
    double getDelay(double constant, double computedResourceValue);
    void applyResourceConsumptionAndSchedulePacketDelivery(VNFMessage* vnfMessage);
    void schedulePacketDelivery(double delayInSecond, VNFMessage* vnfMessage);
    void signalComputedResourcesChanges();
    double getTotalDelayInSeconds();


public:
    void onMaxResourcesValueChange(double maxCpu, double maxRam, double maxStorage) override;

protected:

    IVNFMessageQueue *vnfMessageQueue;
    cGate* allocationCompletedGate;
    //cGate* maxResourcesConfigRequestGate;
    cGate* maxResourcesConfigUpdateGate;
    MaxResourcesProvider* maxResourcesProvider;

    double maxCpu;
    double maxRam;
    double maxStorage;

    double currentCpu;
    double currentRam;
    double currentStorage;

    void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    bool handleValidateResources(const VNFMessage* vnfMessage);
    void handleMaxResourcesConfigRequest(const Vnf2VnfmMessage* vnf2VnfmMessage);
    void handleDeallocateResources(inet::Packet *packet);
};


#endif /* NODES_VNF_SIMPLEVNF_H_ */
