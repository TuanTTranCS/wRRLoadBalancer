//
// Generated file, do not edit! Created by nedtool 5.7 from slicing/VNF/Queue/Messages/PacketRemoval.msg.
//

#ifndef __PACKETREMOVAL_M_H
#define __PACKETREMOVAL_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0507
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



class PacketRemoval;
/**
 * Class generated from <tt>slicing/VNF/Queue/Messages/PacketRemoval.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * class PacketRemoval extends cMessage
 * {
 *     long packetTreeId;
 * }
 * </pre>
 */
class PacketRemoval : public ::omnetpp::cMessage
{
  protected:
    long packetTreeId = 0;

  private:
    void copy(const PacketRemoval& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const PacketRemoval&);

  public:
    PacketRemoval(const char *name=nullptr);
    PacketRemoval(const PacketRemoval& other);
    virtual ~PacketRemoval();
    PacketRemoval& operator=(const PacketRemoval& other);
    virtual PacketRemoval *dup() const override {return new PacketRemoval(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual long getPacketTreeId() const;
    virtual void setPacketTreeId(long packetTreeId);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const PacketRemoval& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, PacketRemoval& obj) {obj.parsimUnpack(b);}

#endif // ifndef __PACKETREMOVAL_M_H
