/* -*- P4_16 -*- */

#include <core.p4>
#include <tna.p4>

//-----------------------------------------------------------------------------
// Protocol Header Definitions
//-----------------------------------------------------------------------------

// #ifndef _P4_HEADERS_
// #define _P4_HEADERS_

typedef bit<48> mac_addr_t;
typedef bit<32> ipv4_addr_t;
typedef bit<24> vni_t;

// const bit<16> ETHERTYPE_TPID = 0x8100;
const bit<16> ETHERTYPE_IPV4 = 0x0800;

const int NEXTHOP_ID_WIDTH = 14;
typedef bit<(NEXTHOP_ID_WIDTH)> nexthop_id_t;

const int NEXTHOP_TABLE_SIZE   = 1 << NEXTHOP_ID_WIDTH;
// @pa_container_size("ingress", "hdr.ethernet.src_addr", 16, 32)
// @pa_container_size("ingress", "hdr.ethernet.dst_addr", 16, 32)
// @pa_container_size("ingress", "hdr.ethernet.$valid", 16)

header ethernet_h {
    mac_addr_t dst_addr;
    mac_addr_t src_addr;
    bit<16> ether_type;
}

header ipv4_h {
    bit<4> version;
    bit<4> ihl;
    bit<8> diffserv;
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

struct shc_ingress_metadata_t {
    bit<32> hash_value1;
    bit<32> hash_value2;

    bit<32> hash_size1;
    bit<32> hash_size2;
    bit<32> hash_min_size;
    bit<64> all_flow_size;

    bit<32> hash_cond;

    bit<16> rnd;

    three_tuple_t key;

    bit<1> key_flag;

    bit<1> e_flow;
}

struct three_tuple_t {
    bit<24> vni;
    ipv4_addr_t src_addr;
    ipv4_addr_t dst_addr;
}

//ingress parser
parser SwitchIngressParser(
        packet_in pkt,
        out shc_header_t hdr,
        out shc_ingress_metadata_t ig_md,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    //value_set<bit<16>>(1) udp_port_vxlan; //4789
    // value_set<switch_cpu_port_value_set_t>(1) cpu_port;

    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        // ig_md.port = ig_intr_md.ingress_port;
        // ig_md.timestamp = ig_intr_md.ingress_mac_tstamp;
     
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            // ETHERTYPE_TPID:  parse_vlan_tag;
            ETHERTYPE_IPV4:  parse_ipv4;
            // ETHERTYPE_DEAD:  parse_dead_drop;

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
            //udp_port_vxlan : parse_vxlan;
            4789 : parse_vxlan;
            // UDP_PORT_ROCEV2 : parse_rocev2;
            default : accept;
        }
    }

    state parse_vxlan {
        pkt.extract(hdr.vxlan);
        // ig_md.tunnel.type = SWITCH_TUNNEL_TYPE_VXLAN;
        // ig_md.tunnel.id = hdr.vxlan.vni;
        transition parse_inner_ethernet;
        // transition accept;  
    }

    state parse_inner_ethernet {
        pkt.extract(hdr.inner_ethernet);
        transition select(hdr.inner_ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_inner_ipv4;
            // ETHERTYPE_IPV6 : parse_inner_ipv6;
            default : accept;
        }
    }

    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        // inner_ipv4_checksum.add(hdr.inner_ipv4);
        // ig_md.flags.inner_ipv4_checksum_err = inner_ipv4_checksum.verify();
        // transition select(hdr.inner_ipv4.protocol) {
        //     // IP_PROTOCOLS_ICMP : parse_inner_icmp;
        //     // IP_PROTOCOLS_TCP : parse_inner_tcp;
        //     // IP_PROTOCOLS_UDP : parse_inner_udp;
        //     default : accept;
        // }

        transition accept;
    }

}

// sketch 

control sketch(inout shc_header_t hdr,
               inout shc_ingress_metadata_t ig_md)
{
    Register<bit<64>,bit<1>>(1) size_all;
    RegisterAction<bit<64>,bit<1>,bit<64>>(size_all) size_all_count={
		void apply(inout bit<32> register_data, out bit<64> flow_size){
            
			register_data = register_data + (bit<32>)hdr.inner_ipv4.total_len >> 3;
			flow_size = register_data;
		}
	};

    action count_all_size(){
        ig_md.all_flow_size = size_all_count.execute(0);
    }
    table count_all_size_table{
        actions = {
            count_all_size();
        }
        size = 1;
        const default_action = count_all_size();
    }

    /* CRC32  as crc32_1*/
    CRCPolynomial<bit<32>>(coeff=0x04C11DB7,reversed=true, msb=false, extended=false, init=0xFFFFFFFF, xor=0xFFFFFFFF) crc32_1;
    /*CRC32-C as crc32_2 */
    CRCPolynomial<bit<32>>(coeff=0x1EDC6F41,reversed=true, msb=false, extended=false, init=0xFFFFFFFF, xor=0xFFFFFFFF) crc32_2;

    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, crc32_1) hash1;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, crc32_2) hash2;


    Register<bit<32>,bit<16>>(65536) reg_for_flow_size_1;
    RegisterAction<bit<32>,bit<16>,bit<32>>(reg_for_flow_size_1) size_count_1={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			register_data = register_data + (bit<32>)hdr.inner_ipv4.total_len >> 3;
			flow_size = register_data;
		}
	};
    RegisterAction<bit<32>,bit<16>,bit<32>>(reg_for_flow_size_1) size_check_1={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			flow_size = register_data;
		}
	};

    Register<three_tuple_t,bit<16>>(65536) reg_for_flow_key_1;
    RegisterAction<three_tuple_t,bit<16>,bit<32>>(reg_for_flow_key_1) key_update_1={
		void apply(inout three_tuple_t register_data){
            register_data = ig_md.key;		
		}
	};
    RegisterAction<three_tuple_t,bit<16>,bit<32>>(reg_for_flow_key_1) key_check_1={
		void apply(inout three_tuple_t register_data){
            if(register_data == ig_md.key){
                ig_md.key_flag = 1;
            }
            else{
                ig_md.key_flag = 0;                
            }			
		}
	};


    Register<bit<32>,bit<16>>(65536) reg_for_flow_size_2;
    RegisterAction<bit<32>,bit<16>,bit<32>>(reg_for_flow_size_2) size_count_2={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			register_data = register_data + (bit<32>)hdr.inner_ipv4.total_len >> 3;
			flow_size = register_data;
		}
	};
    RegisterAction<bit<32>,bit<16>,bit<32>>(reg_for_flow_size_2) size_check_2={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			flow_size = register_data;
		}
	};

    Register<three_tuple_t,bit<16>>(65536) reg_for_flow_key_2;
    RegisterAction<three_tuple_t,bit<16>,bit<32>>(reg_for_flow_key_2) key_update_2={
		void apply(inout three_tuple_t register_data){
            register_data = ig_md.key;
		}
	};
    RegisterAction<three_tuple_t,bit<16>,bit<32>>(reg_for_flow_key_2) key_check_2={
		void apply(inout three_tuple_t register_data){
            if(register_data == ig_md.key){
                ig_md.key_flag = 1;
            }
            else{
                ig_md.key_flag = 0;                
            }			
		}
	};

    // get 2 hash_funcions' hash value by 3 tuple
    action get_hash_value(){
        ig_md.hash_value1 = hash1.get({hdr.vxlan.vni, hdr.inner_ipv4.src_addr, hdr.inner_ipv4.dst_addr});
        ig_md.hash_value2 = hash2.get({hdr.vxlan.vni, hdr.inner_ipv4.src_addr, hdr.inner_ipv4.dst_addr});
    }
    table get_hash_value_table{
        actions = {
            get_hash_value;
        }
        size = 1;
        const default_action = get_hash_value();
    }

    action get_reg_value(){
        ig_md.hash_size1 = size_check_1.execute(ig_md.hash_value1);
        ig_md.hash_size2 = size_check_2.execute(ig_md.hash_value2);
    }
    table get_reg_value_table{
        actions = {
            get_reg_value;
        }
        size = 1;
        const default_action = get_reg_value();
    }


    bit<1> flag;  // ?
    // select the min_size and hash_fun then compute it's probability
    Register<bit<32>,bit<1>>(1) num_32;
	MathUnit<bit<32>>(true,0,9,{68,73,78,85,93,102,113,128,0,0,0,0,0,0,0,0}) prog_64K_div_mu;
	RegisterAction<bit<32>,bit<1>,bit<32>>(num_32) prog_64K_div_x = {
		void apply(inout bit<32> register_data, out bit<32> mau_value){
            if(ig_md.hash_size1 > ig_md.hash_size2){
                register_data = ig_md.hash_size2;
                flag = 0;
            }
            else{
                register_data = ig_md.hash_size1;
                flag = 1;
            }
            ig_md.hash_min_size = register_data;
			mau_value = prog_64K_div_mu.execute(register_data); 
		}
	};


    Random<bit<16>>() random_generator;
	action generate_random_number(){
		ig_md.rnd = random_generator.get();
	}
	table random_number_table{
		actions = {
			generate_random_number;
		}
		size = 1;
		const default_action = generate_random_number();
	}


    action calc_cond(){
        ig_md.hash_cond = prog_64K_div_x.execute(0);
    }
    table calc_cond_table{
        actions = {
            calc_cond;
        }
        size = 1;
        const default_action = calc_cond();
    }


    action update_hash_key_1(){
        key_update_1.execute(ig_md.hash_value1);
    }
    action update_hash_key_2(){
        key_update_2.execute(ig_md.hash_value2);
    }
    table update_hash_key{
        key = {
            flag : exact;
        }
        actions = {
            update_hash_key_1;
            update_hash_key_2;
            NoAction;
        }

        const entries = {
            0 : update_hash_key_2();
            1 : update_hash_key_1();
        }
        size = 2;
        default_action = NoAction;
    }

    action check_hash_key_1(){
        key_check_1.execute(ig_md.hash_value1);
    }
    action check_hash_key_2(){
        key_check_2.execute(ig_md.hash_value2);
    }
    table check_hash_key{
        key = {
            flag : exact;
        }
        actions = {
            check_hash_key_1;
            check_hash_key_2;
            NoAction;
        }

        const entries = {
            0 : check_hash_key_1();
            1 : check_hash_key_2();
        }
        size = 2;
        default_action = NoAction;
    }

    action count_size_1(){
        ig_md.hash_size1 = size_count_1.execute(ig_md.hash_value1);
    }
    action count_size_2(){
        ig_md.hash_size2 = size_count_2.execute(ig_md.hash_value2);
    }
    table count_size{
        key = {
            flag : exact;
        }
        actions = {
            count_size_1;
            count_size_2;
            NoAction;
        }

        const entries = {
            0 : count_size_2();
            1 : count_size_1();
        }
        size = 2;
        default_action = NoAction;
    }


    apply {
        count_all_size_table.apply();

        get_hash_value_table.apply();
        get_reg_value_table.apply();

        calc_cond.apply();

        check_hash_key.apply();
        count_size.apply();

        random_number_table.apply();
        if(ig_md.hash_cond > ig_md.rnd){
            update_hash_key.apply();
        }

        if(ig_md.key_flag == 1 && ig_md.hash_min_size > bit<32>(ig_md.all_flow_size >> 3)){
            ig_md.e_flow = 1;
        }
        else{
            ig_md.e_flow = 0;
        }
    }

}


control SwitchIngress( inout shc_header_t hdr,
                 inout shc_ingress_metadata_t ig_md,
                 
                 in    ingress_intrinsic_metadata_t               ig_intr_md,
                 in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
                 inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
                 inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md    )
{
    nexthop_id_t    nexthop_id = 0;

    action set_nexthop(nexthop_id_t nexthop) {
        nexthop_id = nexthop;
        ig_tm_md.copy_to_cpu = 0; //为了防止ig_tm_md.copy_to_cpu无值
    }


    action report_and_to_nfv(){
        ig_tm_md.ucast_egress_port = 64; //指定端口去NFV
        ig_tm_md.copy_to_cpu = 1; //miss包上报，待完善-------考虑使用mirror
    }
    table eflow_forward{
        key     =   {
                     hdr.vxlan.vni : exact;
                     hdr.inner_ipv4.dst_addr : exact;
                     hdr.inner_ipv4.src_addr : exact; 
                    }
        actions =   {
                     set_nexthop;
                     report_and_to_nfv;               
                    }
        default_action = report_and_to_nfv();
        size    = 2048;
    }


    action send(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
    }
    action drop() {
        ig_dprsr_md.drop_ctl = 1;
    }
    action rewrite_outer_header(PortId_t port, vni_t vni, mac_addr_t src_mac, mac_addr_t dst_mac, 
                                ipv4_addr_t src_addr, ipv4_addr_t dst_addr){
        send(port);
        hdr.vxlan.vni = vni;
        hdr.ipv4.src_addr = src_addr;
        hdr.ipv4.dst_addr = dst_addr;
        hdr.ethernet.src_addr = src_mac;
        hdr.ethernet.dst_addr = dst_mac;
    }
    table nexthop{
        key = { nexthop_id : exact; }
        actions = { 
                    rewrite_outer_header;
                    drop;
                    send;
                  }

        default_action = drop();
        size = NEXTHOP_TABLE_SIZE;
    }
    
    apply{
        if (hdr.ipv4.isValid()) {
            eflow_forward.apply();

            if(ig_tm_md.copy_to_cpu != 1){
                nexthop.apply();
            }
        }

    }
}

//ingress deparser
control SwitchIngressDeparser(
    packet_out pkt,
    inout switch_header_t hdr,
    in switch_ingress_metadata_t ig_md,
    in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr){

    Checksum() ipv4_checksum;
    Checksum() inner_ipv4_checksum;

    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.diffserv,
            hdr.ipv4.total_len,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.frag_offset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr});

        
        hdr.inner_ipv4.hdr_checksum = inner_ipv4_checksum.update({
            hdr.inner_ipv4.version,
            hdr.inner_ipv4.ihl,
            hdr.inner_ipv4.diffserv,
            hdr.inner_ipv4.total_len,
            hdr.inner_ipv4.identification,
            hdr.inner_ipv4.flags,
            hdr.inner_ipv4.frag_offset,
            hdr.inner_ipv4.ttl,
            hdr.inner_ipv4.protocol,
            hdr.inner_ipv4.src_addr,
            hdr.inner_ipv4.dst_addr});
        
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.vxlan);
        pkt.emit(hdr.inner_ethernet);
        pkt.emit(hdr.inner_ipv4);

            }
}


// -------------------------EGRESS-------------------

struct switch_egress_metadata_t {
}
//egress parser
parser SwitchEgressParser(
        packet_in pkt,

        out switch_header_t hdr,
        out switch_egress_metadata_t eg_md,

        out egress_intrinsic_metadata_t eg_intr_md){

    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }

}


control SwitchEgress(
    /* User */
    inout switch_header_t                              hdr,
    inout switch_egress_metadata_t                     meta,
    /* Intrinsic */    
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md)
{
    apply {
    }
}


//egress deparser
control SwitchEgressDeparser(
        packet_out pkt,
        inout switch_header_t hdr,
        in switch_egress_metadata_t eg_md,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr){

    apply{

    }
}

Pipeline(SwitchIngressParser(),
        SwitchIngress(),
        SwitchIngressDeparser(),
        SwitchEgressParser(),
        SwitchEgress(),
        SwitchEgressDeparser()) pipe;

Switch(pipe) main;