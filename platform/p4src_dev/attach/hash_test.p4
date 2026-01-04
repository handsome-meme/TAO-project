/* -*- P4_16 -*- */

/*
combine flow table and sketch 
need to get the hash table's hash values
*/

#include <core.p4>
#include <tna.p4>

typedef bit<48> mac_addr_t;
typedef bit<32> ipv4_addr_t;

const bit<16> ETHERTYPE_IPV4 = 0x0800;
const bit<48> FAKE_SRC_MAC = 0x112233445566;
const int NEXTHOP_ID_WIDTH = 17;
typedef bit<(NEXTHOP_ID_WIDTH)> nexthop_id_t;

const int HASH_BUCKET_WIDTH = 13;
typedef bit<(HASH_BUCKET_WIDTH)> hash_bucket_t;

const bit<8> HIT_COUNT = 3;
const bit<16> PROB_NUM = 65535; //若改成int类型与bit<16>进行饱和减，其结果有问题。条件 >0 不生效。 6554 9806 13108 65535

const int NEXTHOP_TABLE_SIZE   = 1 << NEXTHOP_ID_WIDTH;

const bit<3> FK_LEARN_DIGEST = 1;

const bit<15> PAD = 0;

header ethernet_h {
    mac_addr_t dst_addr;
    mac_addr_t src_addr;
    bit<16> ether_type;
}

header ipv4_h {
    bit<4> version;
    bit<4> ihl;
    bit<6> dscp;
    bit<2> ecn;
    bit<16> total_len;
    bit<16> identification;
    bit<3> flags;
    bit<13> frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdr_checksum;
    ipv4_addr_t src_addr;
    ipv4_addr_t dst_addr;
}

header udp_h {
    bit<16> src_port;
    bit<16> dst_port;
    bit<16> length;
    bit<16> checksum;
}

header vxlan_h {
    bit<8> flags;
    bit<24> reserved;
    bit<24> vni;
    bit<8> reserved2;
}

struct shc_header_t {
    ethernet_h ethernet;
    ipv4_h ipv4;
    udp_h udp;
    vxlan_h vxlan;
    ethernet_h inner_ethernet;
    ipv4_h inner_ipv4;  
}

struct shc_ingress_metadata_t{

}

//ingress parser
parser ShcIngressParser(
        packet_in pkt,

        out shc_header_t hdr,
        out shc_ingress_metadata_t ig_md,

        out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        // ig_md.eflag = 0;
        // ig_md.hash_pkt1_num = 0;
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4:  parse_ipv4;

            default: accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            17 :  parse_udp;

            default: accept;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        transition select(hdr.udp.dst_port) {
            4789 : parse_vxlan;
            default : accept;
        }
    }

    state parse_vxlan {
        pkt.extract(hdr.vxlan);
        // ig_md.vni = hdr.vxlan.vni;
        // ig_md.type = 1;
        // ig_md.eflag = 0;
        transition parse_inner_ethernet;
    }

    state parse_inner_ethernet {
        pkt.extract(hdr.inner_ethernet);
        transition select(hdr.inner_ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_inner_ipv4;
            default : accept;
        }
    }

    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        // ig_md.src_addr = hdr.inner_ipv4.src_addr;
        // ig_md.dst_addr = hdr.inner_ipv4.dst_addr;
        transition accept;
    }

}

control ShcIngress( 
                 inout shc_header_t hdr,
                 inout shc_ingress_metadata_t ig_md,
                 
                 in    ingress_intrinsic_metadata_t               ig_intr_md,
                 in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
                 inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
                 inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md    )
{
    bit<16> FingerPrint;
    bit<16> ghost_hash;

    action route_for_nexthop(){
        ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
        // ig_md.port = ig_intr_md.ingress_port;
        // ig_dprsr_md.digest_type = FK_LEARN_DIGEST;
    }
    action route_for_miss(){
        ig_dprsr_md.drop_ctl = 1;
        // ig_tm_md.ucast_egress_port = 16;
        ig_tm_md.bypass_egress = 1;
        // ig_md.type_for_digest = 0;
        // ig_dprsr_md.digest_type = FK_LEARN_DIGEST;
    }
    table route_tbl{
        key = {
            // ig_md.vrf : exact;
            hdr.ipv4.src_addr : exact;
            // hdr.ipv4.dst_addr : exact;
        }
        actions = {
            route_for_nexthop;
            route_for_miss;
            NoAction;
        }
        const default_action = route_for_miss;
        size = 65535;
    }

    /* CRC32  as crc32_1*/
    CRCPolynomial<bit<32>>(coeff=0x0002,reversed=false, msb=false, extended=false, init=0x0000, xor=0x0000) crc32_1;
    // CRCPolynomial<bit<32>>(coeff=0x1EDC6F41,reversed=true, msb=false, extended=false, init=0xFFFFFFFF, xor=0xFFFFFFFF) crc32_2;
    // Hash<bit<32>>(HashAlgorithm_t.CUSTOM, crc32_1) FP_hash;
    /* HashAlgorithm_t.IDENTITY 直接截取低位*/
    Hash<bit<16>>(HashAlgorithm_t.IDENTITY) FP_hash;
    // Hash<bit<16>>(HashAlgorithm_t.CUSTOM, crc32_2) hash2;


    Register<bit<16>,bit<1>>(1,0) FP;
    RegisterAction<bit<16>,bit<1>,bit<16>>(FP) FP_record={
		void apply(inout bit<16> register_data, out bit<16> outt){
            register_data = ghost_hash;
		}
	};
    
    apply{
        if (hdr.ipv4.isValid()) { 
            route_tbl.apply();
            // ghost_hash = FP_hash.get({FAKE_SRC_MAC});
            ghost_hash = (bit<16>)(hdr.ipv4.src_addr[0:0] ^ hdr.ipv4.src_addr[1:1] ^ hdr.ipv4.src_addr[2:2] );
            //  ++ (hdr.ipv4.src_addr[23:20] ^ hdr.ipv4.src_addr[19:16]) ++ (hdr.ipv4.src_addr[27:24] ^ hdr.ipv4.src_addr[31:28])
            FP_record.execute(0);
        }
    }
}
// 当使用两个digest时，会出现32bit container Consecutive location 的错误，应该是巧合
// 由于PortId_t为16bit，与后面的bit<16> vrf_digest 凑成一个32bit，编译器会将两个16bit放在同一个32bit container里面，导致出错

//ingress deparser
control ShcIngressDeparser(
    packet_out pkt,
    inout shc_header_t hdr,
    in shc_ingress_metadata_t ig_md,
    in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md){

    Checksum() ipv4_checksum;
    Checksum() inner_ipv4_checksum;

    apply {  
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.dscp,
            hdr.ipv4.ecn,
            hdr.ipv4.total_len,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.frag_offset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr
        });

        
        hdr.inner_ipv4.hdr_checksum = inner_ipv4_checksum.update({
            hdr.inner_ipv4.version,
            hdr.inner_ipv4.ihl,
            hdr.inner_ipv4.dscp,
            hdr.inner_ipv4.ecn,
            hdr.inner_ipv4.total_len,
            hdr.inner_ipv4.identification,
            hdr.inner_ipv4.flags,
            hdr.inner_ipv4.frag_offset,
            hdr.inner_ipv4.ttl,
            hdr.inner_ipv4.protocol,
            hdr.inner_ipv4.src_addr,
            hdr.inner_ipv4.dst_addr
        });
        
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.vxlan);
        pkt.emit(hdr.inner_ethernet);
        pkt.emit(hdr.inner_ipv4);
    }
}


// -------------------------EGRESS-------------------

struct shc_egress_metadata_t {

}
//egress parser
parser ShcEgressParser(
        packet_in pkt,
        out shc_header_t hdr,
        out shc_egress_metadata_t eg_md,
        out egress_intrinsic_metadata_t eg_intr_md){

    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }

}


control ShcEgress(
    /* User */
    inout shc_header_t                              hdr,
    inout shc_egress_metadata_t                     eg_meta,
    /* Intrinsic */    
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md)
{
    apply{

    }
}


//egress deparser
control ShcEgressDeparser(
        packet_out pkt,
        inout shc_header_t hdr,
        in shc_egress_metadata_t eg_md,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr){

    apply{

    }
}

Pipeline(
        ShcIngressParser(),
        ShcIngress(),
        ShcIngressDeparser(),
        ShcEgressParser(),
        ShcEgress(),
        ShcEgressDeparser()
    ) pipe;

Switch(pipe) main;