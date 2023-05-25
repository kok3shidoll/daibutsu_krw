#ifndef SOCKPUPPET_H
#define SOCKPUPPET_H

#include <netinet/in.h>

#include <common.h>

#define MAX_ATTEMPTS 50000
#define IPV6_USE_MIN_MTU 42
#define IPV6_PKTINFO 46
#define IPV6_PREFER_TEMPADDR 63

struct ool_msg  {
    mach_msg_header_t hdr;
    mach_msg_body_t body;
    mach_msg_ool_ports_descriptor_t ool_ports;
};

struct route_in6 {
    struct rtentry *ro_rt;
    struct llentry *ro_lle;
    struct ifaddr *ro_srcia;
    uint32_t ro_flags;
    struct sockaddr_in6 ro_dst;
};

struct ip6po_rhinfo {
    struct ip6_rthdr *ip6po_rhi_rthdr; /* Routing header */
    struct route_in6 ip6po_rhi_route; /* Route to the 1st hop */
};

struct ip6po_nhinfo {
    struct sockaddr *ip6po_nhi_nexthop;
    struct route_in6 ip6po_nhi_route; /* Route to the nexthop */
};

struct ip6_pktopts {
    struct mbuf *ip6po_m;
    int ip6po_hlim;
    struct in6_pktinfo *ip6po_pktinfo;
    struct ip6po_nhinfo ip6po_nhinfo;
    struct ip6_hbh *ip6po_hbh;
    struct ip6_dest *ip6po_dest1;
    struct ip6po_rhinfo ip6po_rhinfo;
    struct ip6_dest *ip6po_dest2;
    int ip6po_tclass;
    int ip6po_minmtu;
    int ip6po_prefer_tempaddr;
    int ip6po_flags;
};

addr_t leakPortAddress(mach_port_t port);

#endif
