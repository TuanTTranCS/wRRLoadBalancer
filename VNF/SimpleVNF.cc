/*
 * SimpleVNF.cc
 *
 *  Created on: Apr. 29, 2021
 *      Author: vibisson
 */




#include "SimpleVNF.h"

#include "./Messages/VNF_m.h"
#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "apps/voip/VoipPacket_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "slicing/VNF/Messages/VNFTrailer_m.h"
#include <math.h>

Define_Module(SimpleVNF)

void SimpleVNF::initialize(int stage)
{
    cpuChangesSignal = registerSignal("cpuChanges");
    ramChangesSignal = registerSignal("ramChanges");
    storageChangesSignal = registerSignal("storageChanges");
    maxCpu = par("maxCpu");
    maxRam = par("maxRam");
    maxStorage = par("maxStorage");
    currentCpu = 0;
    currentRam = 0;
    currentStorage = 0;

    k = par("k").doubleValue();
    g = par("g").doubleValue();
    h = par("h").doubleValue();

    allocationCompletedGate = gate("allocationCompleted");
    maxResourcesConfigUpdateGate = gate("maxResourcesConfigUpdate");
    //maxResourcesConfigRequestGate = gate("maxResourcesConfigRequest");

    vnfMessageQueue = check_and_cast<IVNFMessageQueue*>(getSubmodule("queue"));
    maxResourcesProvider = check_and_cast<MaxResourcesProvider*>(getSubmodule("maxResourcesProvider"));

    closedLoopClient = check_and_cast<IClosedLoopClient*>(getSystemModule()->getSubmodule("closedLoopClient"));

    maxResourcesProvider->setVnf(this);
}

void SimpleVNF::handleMessage(cMessage* msg) {

    //Enter_Method("handleMessage");
    //take(msg);
    if (msg->isSelfMessage()) // time to deliver the packer to the upper layer
    {
        sendDirect(msg, allocationCompletedGate);
    }
    else if (msg->arrivedOn("validateResources"))
    {
        VNFMessage* vnfMessage = check_and_cast<VNFMessage*>(msg);

        vnfMessageQueue->addMessage(vnfMessage);
        checkNextMessageInQueue();
    }
    else if (msg->arrivedOn("deallocateResources"))
    {
        inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
        handleDeallocateResources(packet);
    }
    else if (msg->arrivedOn("maxResourcesConfigRequest"))
    {
        Vnf2VnfmMessage* vnf2VnfmMessage = check_and_cast<Vnf2VnfmMessage*>(msg);
        handleMaxResourcesConfigRequest(vnf2VnfmMessage);
        delete vnf2VnfmMessage;
    }
}

bool SimpleVNF::handleValidateResources(const VNFMessage* vnfMessage)
{
    inet::Packet* packet = vnfMessage->getPck()->dup();

    inet::PacketPrinter printer;
    printer.printPacket(std::cout, packet);

    inet::Ptr<const VNFTrailer> vnfTrailer;

    try
    {
        vnfTrailer = packet->popAtBack<VNFTrailer>(VNF_TRAILER_LENGTH);
    }
    catch(...)
    {
        delete packet;
        return true;
    }

    if (vnfTrailer == nullptr)
    {
        throw cRuntimeError("Invalid packet: VNFTrailer is missing");
    }

    double requiredCpu = vnfTrailer->getRequiredCpu();
    double requiredRam = vnfTrailer->getRequiredRam();
    double requiredStorage = vnfTrailer->getRequiredStorage();

    delete packet;

    if (enoughResources(requiredCpu, requiredRam, requiredStorage))
    {
        return true;
    }
    else
    {
        EV << "Not enough resources for new packet. Max cpu: " << maxCpu << " Max ram: " << maxRam << " Max storage: " << maxStorage << std::endl;
        return false;
    }
}

void SimpleVNF::handleMaxResourcesConfigRequest(const Vnf2VnfmMessage* vnf2VnfmMessage)
{
    // Tuan: Send a message to SimpleVnfManager containing new max resources
    //Enter_Method("handleMaxResourcesConfigRequest");
    double maxCpu = this->maxCpu;
    double maxRam = this->maxRam;
    double maxStorage = this->maxStorage;
    Vnf2VnfmMessage *msg = new Vnf2VnfmMessage();
    msg->setMaxCpu(maxCpu);
    msg->setMaxRam(maxRam);
    msg->setMaxStorage(maxStorage);

    // Tuan: Send the max resource info to VNFManager
    //std::cout << "Sending current max Resources config to VnfManager. Max CPU: " << maxCpu << " - Max RAM: " << maxRam << " - Max storage: " << maxStorage << std::endl;
    sendDirect(msg, maxResourcesConfigUpdateGate);
    //delete msg;
}

void SimpleVNF::applyResourceConsumptionAndSchedulePacketDelivery(VNFMessage* vnfMessage)
{
    inet::Packet* packet = vnfMessage->getPck()->dup();

    inet::Ptr<const VNFTrailer> vnfTrailer;

    try
    {
        vnfTrailer = packet->popAtBack<VNFTrailer>(VNF_TRAILER_LENGTH);
    }
    catch(...)
    {
        delete packet;
        sendDirect(vnfMessage, allocationCompletedGate);
        return;
    }

    if (vnfTrailer == nullptr)
    {
        throw cRuntimeError("Invalid packet: VNFTrailer is missing");
    }

    double requiredCpu = vnfTrailer->getRequiredCpu();
    double requiredRam = vnfTrailer->getRequiredRam();
    double requiredStorage = vnfTrailer->getRequiredStorage();

    EV << "Allocating resources. Packet id:" << packet->getTreeId() << " CPU: " << requiredCpu << " RAM: " << requiredRam << " Storage: " << requiredStorage << std::endl;

    currentCpu += requiredCpu;
    currentRam += requiredRam;
    currentStorage += requiredStorage;

    delete packet;

    signalComputedResourcesChanges();

    double delayInSeconds = getTotalDelayInSeconds();

    schedulePacketDelivery(delayInSeconds, vnfMessage);
}

double SimpleVNF::getTotalDelayInSeconds()
{
    double cpuDelayInSeconds = getDelay(k, currentCpu / maxCpu);
    double ramDelayInSeconds = getDelay(g, currentRam / maxRam);
    double storageDelayInSeconds = getDelay(h, currentStorage / maxStorage);

    return cpuDelayInSeconds + ramDelayInSeconds + storageDelayInSeconds;
}

double SimpleVNF::getDelay(double constant, double computedResourceValue)
{
    double delayInMilliseconds = exp(constant * computedResourceValue) - 1;

    return delayInMilliseconds < 0 ? 0 : delayInMilliseconds / 1000; // return the value in seconds
}

void SimpleVNF::handleDeallocateResources(inet::Packet *packet)
{
    inet::PacketPrinter printer;
    printer.printPacket(EV, packet);

    inet::Ptr<const VNFTrailer> vnfTrailer;

    try
    {
        vnfTrailer = packet->popAtBack<VNFTrailer>(VNF_TRAILER_LENGTH);
    }
    catch (...)
    {
        delete packet;
        return;
    }

    if (vnfTrailer == nullptr)
    {
        throw cRuntimeError("Invalid packet: VNFTrailer is missing");
    }

    double requiredCpu = vnfTrailer->getRequiredCpu();
    double requiredRam = vnfTrailer->getRequiredRam();
    double requiredStorage = vnfTrailer->getRequiredStorage();

    EV << "Deallocating resources CPU: " << requiredCpu << " RAM: " << requiredRam << " Storage: " << requiredStorage << std::endl;

    currentCpu -= requiredCpu;
    currentRam -= requiredRam;
    currentStorage -= requiredStorage;

    signalComputedResourcesChanges();

    delete packet;

    checkNextMessageInQueue();
}

void SimpleVNF::signalComputedResourcesChanges()
{
    double cpuUsage = currentCpu / maxCpu;
    double ramUsage = currentRam / maxRam;
    double storageUsage = currentStorage / maxStorage;

    closedLoopClient->postComputedResources(getFullPath(), cpuUsage, ramUsage, storageUsage);

    emit(cpuChangesSignal, cpuUsage);
    emit(ramChangesSignal, ramUsage);
    emit(storageChangesSignal, storageUsage);
}

void SimpleVNF::checkNextMessageInQueue()
{
    auto peekMessage = vnfMessageQueue->peekNextMessage();

    if (peekMessage != nullptr)
    {
        if (handleValidateResources(peekMessage))
        {
            auto nextMessage = vnfMessageQueue->getNextMessage(); // Remove the message from the queue;
            applyResourceConsumptionAndSchedulePacketDelivery(nextMessage);
        }
    }
}

bool SimpleVNF::enoughResources(double requiredCpu, double requiredRam, double requiredStorage)
{
    return currentCpu + requiredCpu <= maxCpu &&
            currentRam + requiredRam <= maxRam &&
            currentStorage + requiredStorage <= maxStorage;
}

void SimpleVNF::refreshDisplay() const {
    std::string cpuString = std::to_string(currentCpu);
    std::string storageString = std::to_string(currentStorage);
    std::string ramString = std::to_string(currentRam);

    std::string text = "CPU: " + cpuString + ", Storage: " + storageString + ", Ram: " + ramString;
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void SimpleVNF::onMaxResourcesValueChange(double maxCpu, double maxRam, double maxStorage)
{
    Enter_Method("onMaxResourcesValueChange");
    this->maxCpu = maxCpu;
    this->maxRam = maxRam;
    this->maxStorage = maxStorage;
    /*
    std::cout << "Received new max Resources config from file. Max CPU: " << maxCpu
            << " - Max RAM: " << maxRam
            << " - Max storage: " << maxStorage << std::endl;
            */

    checkNextMessageInQueue();
    // Tuan: Send a message to SimpleVnfManager containing new max resources
    //sendMaxResourceUpdateToVnfManager(maxCpu, maxRam, maxStorage);
    Vnf2VnfmMessage *msg = new Vnf2VnfmMessage();
    msg->setMaxCpu(maxCpu);
    msg->setMaxRam(maxRam);
    msg->setMaxStorage(maxStorage);

    // Tuan: Send the max resource info to VNFManager
    //std::cout << "Sending new max resources config to VnfManager. Max CPU: " << maxCpu << " - Max RAM: " << maxRam << " - Max storage: " << maxStorage << std::endl;
    sendDirect(msg, maxResourcesConfigUpdateGate);
}



void SimpleVNF::schedulePacketDelivery(double delayInSecond, VNFMessage* vnfMessage)
{
    //Phill change to make sure we are using the correct unit
    simtime_t timeout = (simTime() + SimTime(delayInSecond, SIMTIME_S));
    scheduleAt(timeout, vnfMessage);
}
