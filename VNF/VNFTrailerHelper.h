/*
 * VNFTrailerFactory.h
 *
 *  Created on: Aug. 19, 2021
 *      Author: vibisson
 */

#ifndef COMMON_VNF_VNFTRAILERFACTORY_H_
#define COMMON_VNF_VNFTRAILERFACTORY_H_


#include <omnetpp.h>
#include "slicing/VNF/Messages/VNFTrailer_m.h"
#include "inet/common/packet/Packet_m.h"
#include <functional>

using namespace omnetpp;
using namespace inet;

namespace vnf
{

Ptr<VNFTrailer> createVNFTrailer(cSimpleModule* module);

int getRouteConfigId(Packet* packet);

}


#endif /* COMMON_VNF_VNFTRAILERFACTORY_H_ */
