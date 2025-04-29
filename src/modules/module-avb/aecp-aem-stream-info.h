#ifndef __AECP_AEM_STREAM_INFO_H__
#define __AECP_AEM_STREAM_INFO_H__

// Add your declarations and definitions here


// This is used for the GET_COMMAND on the  stream inpuyt registering
/** Milan V1.2 Table 5.8 GET_STREAM_INFO */
#define AVB_AEM_STREAM_INFO_FLAG_EX_REGISTERING			    (0)

/* IEEE 1722.1-2016 Table 7-145: Stream Info Flags */
#define AVB_AEM_STREAM_INFO_FLAG_CLASS_B			        (0)

#define AVB_AEM_STREAM_INFO_FLAG_FAST_CONNECT			    (1)
#define AVB_AEM_STREAM_INFO_FLAG_SAVED_STATE			    (2)
#define AVB_AEM_STREAM_INFO_FLAG_STREAMING_WAIT			    (3)
#define AVB_AEM_STREAM_INFO_FLAG_SUPPORTS_ENCRYPTED         (4)
#define AVB_AEM_STREAM_INFO_FLAG_ENCRYPTED_PDU              (5)
#define AVB_AEM_STREAM_INFO_FLAG_TALKER_FAILED              (6)
#define AVB_AEM_STREAM_INFO_FLAG_NO_SRP                     (8)
#define AVB_AEM_STREAM_INFO_FLAG_IP_FLAGS_VALID             (19)
#define AVB_AEM_STREAM_INFO_FLAG_IP_SRC_PORT_VALID          (20)
#define AVB_AEM_STREAM_INFO_FLAG_IP_DST_PORT_VALID          (21)
#define AVB_AEM_STREAM_INFO_FLAG_IP_SRC_ADDR_VALID          (22)
#define AVB_AEM_STREAM_INFO_FLAG_IP_DST_ADDR_VALID          (23)
#define AVB_AEM_STREAM_INFO_FLAG_NO_REGISTERING_SRP         (24)
#define AVB_AEM_STREAM_INFO_FLAG_STREAM_VLAN_ID_VALID       (25)
#define AVB_AEM_STREAM_INFO_FLAG_CONNECTED                  (26)
#define AVB_AEM_STREAM_INFO_FLAG_MSRP_FAILURE_VALID         (27)
#define AVB_AEM_STREAM_INFO_FLAG_STREAM_DEST_MAC_VALID      (28)
#define AVB_AEM_STREAM_INFO_FLAG_MSRP_ACC_LAT_VALID         (29)
#define AVB_AEM_STREAM_INFO_FLAG_STREAM_ID_VALID            (30)
#define AVB_AEM_STREAM_INFO_FLAG_STREAM_FORMAT_VALID        (31)

#define AVB_AEM_STREAM_INFO_GET_MASK(flag)                  (1u << (flag))

#define AVB_AEM_STREAM_INFO_GET_FLAG(flags, flag)           \
    ((flags) & AVB_AEM_STREAM_INFO_GET_MASK(flag))

#define AVB_AEM_STREAM_INFO_SET_FLAG(flags, flag)           \
    ((flags) |= AVB_AEM_STREAM_INFO_GET_MASK(flag))

#define AVB_AEM_STREAM_INFO_CLEAR_FLAG(flags, flag)         \
    ((flags) &= ~AVB_AEM_STREAM_INFO_GET_MASK(flag))

#endif // __AECP_AEM_STREAM_INFO_H__