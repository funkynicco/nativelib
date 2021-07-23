#pragma once

#ifdef DPID_ALLPLAYERS
#undef DPID_ALLPLAYERS
#endif

#ifdef DPID_SERVERPLAYER
#undef DPID_SERVERPLAYER
#endif

#ifdef DPID_UNKNOWN
#undef DPID_UNKNOWN
#endif

// Represents a broadcast
#define DPID_ALLPLAYERS 0

// Represents the server
//#define DPID_SERVERPLAYER 1

#ifdef NL_ARCHITECTURE_X64
#define DPID_UNKNOWN 0xffffffffffffffff
#else
#define DPID_UNKNOWN 0xffffffff
#endif

namespace nl::network
{
#ifdef NL_ARCHITECTURE_X64
    typedef uint64_t DPID;
#else
    typedef uint32_t DPID;
#endif

    //enum class IOEvent
    //{
    //    Accept,
    //    Connect,
    //    Receive,
    //    Send,
    //    Disconnect
    //};
}