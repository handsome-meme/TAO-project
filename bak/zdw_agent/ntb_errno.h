#ifndef NTB_ERRNO_H
#define NTB_ERRNO_H

#ifdef _cplusplus  
extern "C" {
#endif

#define NTB_ERRNO_START      (0)

#define foreach_ntb_err_code \
    _ (NTB_COMM_SUCCESS, "success") \
    _ (NTB_COMM_FAILURE, "failure") \
    _ (NTB_COMM_NOMEM, "out of memory") \
    _ (NTB_COMM_NOENT, "allocate memory failed") \
    _ (NTB_COMM_INVAL_PARAM, "invalid parameter") \
    _ (NTB_REGIST_REPORT_CHANNEL_FAIL, "ntb regist report channel fail") \
    _ (NTB_VRF_NAME_INVALID, "VRF name is invalid") \
    _ (NTB_VRF_NOENT, "VRF was used up") \
    _ (NTB_VRF_EXIST, "VRF exists already") \
    _ (NTB_VRF_INVAL_VRFID, "VRF id is invalid") \
    _ (NTB_VRF_IP_TYPE_NOT_SUPPORTED, "VRF ip type not supported") \
    _ (NTB_TUNNEL_NO_FOUND, "Tunnel is not found") \
    _ (NTB_TUNNEL_INUSED, "Tunnel is in used") \
    _ (NTB_TUNNEL_MAP_NOSPACE, "Tunnel mapping has no space") \
    _ (NTB_TUNNEL_BUNDLE_NOSPACE, "Tunnel bundle has no space") \
    _ (NTB_TUNNEL_BUNDLE_EXIST, "Tunnel bundle alreay exist") \
    _ (NTB_TUNNEL_BUNDLE_NO_EXIST, "Tunnel bundle not exist") \
    _ (NTB_TUNNEL_CLEAR_STAT_ERR, "Tunnel clear stat err") \
    _ (NTB_TUNNEL_GET_STAT_ERR, "Tunnel get stat err") \
    _ (NTB_EG_TUNNEL_MAP_NOSPACE, "Egress Tunnel mapping has no space") \
    _ (NTB_ND_PROXY_NOSPACE, "ND proxy has no space") \
    _ (NTB_ND_PROXY_NOEXIST, "ND proxy is not found") \
    _ (NTB_NEIGH_ENTRY_NO_FOUND, "neighbour entry not found") \
    _ (NTB_IPV4_PREFIXLEN_OUT_RANGE, "ipv4 prefix length out of range") \
    _ (NTB_IPV4_ROUTETYPE_NO_SAME, "ipv4 route type not same") \
    _ (NTB_VXLAN_VNI_INVALID, "vni is invalid") \
    _ (NTB_VXLAN_REMOTEIP_NOSET, "vxlan tunnel remote is not set") \
    _ (NTB_VXLAN_DSCP_OUT_RANGE, "vxlan tunnel DSCP is out of range") \
    _ (NTB_TGRE_VPCID_INVALID, "vpcid is invalid") \
    _ (NTB_ROUTE_NH_ALLOC_OID_FAIL, "route nexthop alloc oid faild") \
    _ (NTB_ROUTE_TYPE_CONFLICT, "route type conflict") \
    _ (NTB_ROUTE_ECMP_OUT_RANGE, "route ecmp is out of range") \
    _ (NTB_VRF_NOT_EXIST, "VRF does not exist") \
    _ (NTB_ROUTE_PREFIX_NOSPACE, "route prefix has no space") \
    _ (NTB_ROUTE_PREFIX_PER_VRF_NOSPACE, "route prefix perf vrf no space") \
    _ (NTB_ROUTE_RESULT_NOSPACE, "route result has no space") \
    _ (NTB_ROUTE_NEXTHOP_NOT_FOUND, "route nexthop is not found") \
    _ (NTB_ROUTE_NEXTHOP_ECMP_NOSPACE, "route nexhop ecmp has not space") \
    _ (NTB_GRPC_PARAMS_INVALID, "grpc msg params invalid ") \
    _ (NTB_ROUTE_PREFIX_NOT_FOUND, "route prefix is not found") \
    _ (NTB_LOAD_TO_P4_FAIL, "load to p4 fail") \
    _ (NTB_TUNNEL_OBJ_EXCEED_SPEC, "tunnel obj exceed spec") \
    _ (NTB_VXLAN_BD_OBJ_EXCEED_SPEC, "vxlan bundle obj exceed spec") \
    _ (NTB_FIB_HOST_OBJ_EXCEED_SPEC, "vxlan fib host obj exceed spec") \
    _ (NTB_FIB_LPM_OBJ_EXCEED_SPEC, "vxlan fib lpm obj exceed spec") \
    _ (NTB_NHOP_OBJ_EXCEED_SPEC, "vxlan nhop obj exceed spec") \
    _ (NTB_EGRESS_TUNNEL_OBJ_EXCEED_SPEC, "egress tunnel obj exceed spec") \
    _ (NTB_INNER_NEIGH_OBJ_EXCEED_SPEC, "inner neigh obj exceed spec") \
    _ (NTB_ECMP_OBJ_EXCEED_SPEC, "ecmp obj exceed spec") \
    _ (NTB_MROUTE_OBJ_EXCEED_SPEC, "mroute obj exceed spec") \
    _ (NTB_IPV6_NOT_SUPPORT, "ntb not support ipv6") \
    _ (NTB_HOSTIF_NO_SPACE, "ntb hostif no space") \
    _ (NTB_HOSTIF_NO_FOUND, "ntb hostif no found") \
    _ (NTB_HOSTIF_IP_SET_KERNEL_FAIL, "ntb set hostif ip to kernel faild") \
    _ (NTB_HOSTIF_IP_UNSET_KERNEL_FAIL, "ntb unset hostif ip to kernel faild") \
    _ (NTB_HOSTIF_IP_ALREADY_SET, "ntb hostif ip already set") \
    _ (NTB_TERMVIP_NO_SPACE, "ntb terminate vip no space") \
    _ (NTB_FORWARD_VIP_ALREADY_SET, "ntb forward vip already set") \
    _ (NTB_FORWARD_VIP_NO_SET, "ntb forward vip not set") \
    _ (NTB_LO_DEV_NO_SET, "ntb loopback device not set") \
    _ (NTB_CAPUTRE_DEV_ALREADY_SET, "ntb capture device already set") \
    _ (NTB_CAPUTRE_DEV_NOT_SUPPORTED, "ntb capture device not supported") \
    _ (NTB_CNTL_IP_ALREADY_SET, "ntb cntl ip already set") \
    _ (NTB_ONLINE_ALREADY, "ntb already online") \
    _ (NTB_GRESIP_NO_SPACE, "ntb gre sip no space") \
    _ (NTB_KERNEL_INTERFACE_NO_EXIST, "ntb kernel inteface not exist") \
    _ (NTB_CAPTURE_SUCCESS, "ntb start/stop capture success") \
    _ (NTB_CAPTURE_INVAL_PARAM, "ntb capture params invalid!") \
    _ (NTB_CAPTURE_NO_VRF, "ntb vrf is no exist!") \
    _ (NTB_CAPTURE_ENTRY_EXIST, "ntb capture already exist!") \
    _ (NTB_CAPTURE_UPDATE_TABLE_FAIL, "ntb acl table update fail!") \
    _ (NTB_CAPTURE_SET_PACKET_NUM_FAIL, "ntb capture set pcaket nums fail!") \
    _ (NTB_CAPTURE_START_TCPDUMP_FAIL, "ntb capture start tcpdump fail!") \
    _ (NTB_CAPTURE_STOP_TCPDUMP_FAIL, "ntb capture stop tcpdump fail!") \
    _ (NTB_CAPTURE_ALREADY_STOP, "ntb capture already stop!") \
    _ (NTB_CAPTURE_NO_FOUND_IN_ACL, "ntb capture entry no found in acl table!") \
    _ (NTB_CAPTURE_DEL_TABLE_FAIL, "ntb capture delete acl table fail!") \
    _ (NTB_ADD_ACL_ENTRY_FAIL, "ntb add a new acl table entry fail!") \
    _ (NTB_DEL_ACL_ENTRY_FAIL, "ntb del an acl table entry fail!") \
    _ (NTB_GET_ACL_STATIS_FAIL, "ntb acl malloc fail!") \
    _ (NTB_MALLOC_ACL_FAIL, "ntb acl malloc fail!") \
    _ (NTB_ERRNO_END, "terminator of error define")

    /* Notes: 
     *   Append new ntb error code before NTB_ERRNO_END
     */ 

enum {
#define _(f,s) f##_idx,
    foreach_ntb_err_code
#undef _
};

typedef enum {
#define _(f,s) f=-(f##_idx+NTB_ERRNO_START),
    foreach_ntb_err_code
#undef _
} ntb_err_code_t;

#define NTB_ERR_CODE_VALID(e) \
    ((e) >= NTB_ERRNO_END && (e) <= -NTB_ERRNO_START)


const char *
ntb_err_code_def(int _errno);

const char *
ntb_err_code_desc(int _errno);

#ifdef _cplusplus  
}
#endif

#endif
