/*
 * VNFTrailerFactory.cc
 *
 *  Created on: Aug. 19, 2021
 *      Author: vibisson
 */



#include "VNFTrailerHelper.h"

namespace vnf
{

static void setMultiplicationFactors(std::function<void(size_t,double)> setter, std::vector<std::string> multiplicationFactorsConfig)
{
    for(size_t i = 0; i < multiplicationFactorsConfig.size(); ++i)
    {
        double factor = std::stod(multiplicationFactorsConfig[i]);
        setter(i, factor);
    }
}

static double getMultiplicationFactor(std::vector<std::string> multiplicationFactors)
{
    if (multiplicationFactors.size() == 0)
    {
        return 1;
    }

    auto randomIndex = rand() % multiplicationFactors.size();
    return std::stod(multiplicationFactors[randomIndex]);
}

Ptr<VNFTrailer> createVNFTrailer(cSimpleModule* module)
{
    double cpu = module->par("cpu");
    double ram = module->par("ram");
    double storage = module->par("storage");
    double sliceId = module->par("sliceId").intValue();

    auto ramMultiplicationFactors = cStringTokenizer(module->par("ramMultiplicationFactors").stringValue()).asVector();
    auto storageMultiplicationFactors = cStringTokenizer(module->par("storageMultiplicationFactors").stringValue()).asVector();

    auto ramMultiplicationFactor = getMultiplicationFactor(ramMultiplicationFactors);
    auto storageMultiplicationFactor = getMultiplicationFactor(storageMultiplicationFactors);

    auto vnfTrailer = makeShared<VNFTrailer>();

    vnfTrailer->setRequiredCpu(cpu);
    vnfTrailer->setRequiredRam(ram * ramMultiplicationFactor);
    vnfTrailer->setRequiredStorage(storage * storageMultiplicationFactor);
    vnfTrailer->setChunkLength(VNF_TRAILER_LENGTH);
    vnfTrailer->setSliceId(sliceId);

    return vnfTrailer;
}

int getRouteConfigId(Packet* packet)
{
    inet::Ptr<const VNFTrailer> vnfTrailer;

    try
    {
        vnfTrailer = packet->peekAtBack<VNFTrailer>(VNF_TRAILER_LENGTH);
    }
    catch(...)
    {
        return 1; // by default go through UPF. Not UpfMec
    }

    return vnfTrailer->getRouteConfigId();
}

}

