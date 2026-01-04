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
// const bit<16> SIZE_K = 1024;
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


struct ip_pair_t {
    ipv4_addr_t src_addr;
    ipv4_addr_t dst_addr;
}

struct three_tuple_t {
    bit<32> vni;
    ip_pair_t pair;
}

struct shc_ingress_metadata_t {
    bit<32> hash_value1;
    bit<32> hash_value2;
    // bit<32> hash_value;

    bit<32> hash_size1;
    bit<32> hash_size2;
    bit<32> hash_size;
    bit<32> hash_size_m;
    bit<32> all_flow_size;

    bit<16> hash_cond; //概率
    bit<16> rnd;

    three_tuple_t key;

    bit<1> cmp_flag;
    bit<1> key_vni_flag;
    bit<1> key_ip_pair_flag;
    
    // 输出参数
    bit<1> key_flag; //key是否相同
    bit<1> key_change; //key不同是否概率更改
    bit<1> size_flag; //是否大于阈值
    // bit<1> e_flow;
}

//ingress parser
parser ShcIngressParser(
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
        ig_md.key.vni = (bit<32>)hdr.vxlan.vni;
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
        ig_md.key.pair.src_addr = hdr.inner_ipv4.src_addr;
        ig_md.key.pair.dst_addr = hdr.inner_ipv4.dst_addr;
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
control Sketch(in bit<32> len,
               inout shc_ingress_metadata_t ig_md)()
{
    Register<bit<32>,bit<1>>(1) size_all;
    RegisterAction<bit<32>,bit<1>,bit<32>>(size_all) size_all_count={
		void apply(inout bit<32> register_data, out bit<32> flow_size){            
			register_data = register_data + len;
			flow_size = register_data;
		}
	};

    action count_all_size(){
        ig_md.all_flow_size = size_all_count.execute(0);
    }
    table count_all_size_table{ //计算所有流的总size
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

    //hash1
    Register<bit<32>,bit<32>>(32w5120) reg_for_flow_size_1;
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_size_1) size_count_1={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			register_data = register_data + len;
			flow_size = register_data;
		}
	};
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_size_1) size_check_1={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			flow_size = register_data;
		}
	};

        // reg for key: vni, ip_pair
    Register<bit<32>,bit<32>>(32w5120) reg_for_flow_key_vni_1;
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_key_vni_1) key_vni_update_1={
		void apply(inout bit<32> register_data){
            register_data = ig_md.key.vni;	
		}
	};
    RegisterAction<bit<32>,bit<32>,bit<1>>(reg_for_flow_key_vni_1) key_vni_check_1={
		void apply(inout bit<32> register_data,out bit<1> out_flag){
            if(register_data == ig_md.key.vni){
                out_flag = 1;
            }
            else{
                out_flag = 0;                
            }			
		}
	};

    Register<ip_pair_t,bit<32>>(32w5120) reg_for_flow_key_ip_pair_1;
    RegisterAction<ip_pair_t,bit<32>,bit<32>>(reg_for_flow_key_ip_pair_1) key_ip_pair_update_1={
		void apply(inout ip_pair_t register_data){
            register_data.src_addr = ig_md.key.pair.src_addr;
            register_data.dst_addr = ig_md.key.pair.dst_addr;
		}
	};
    RegisterAction<ip_pair_t,bit<32>,bit<1>>(reg_for_flow_key_ip_pair_1) key_ip_pair_check_1={
		void apply(inout ip_pair_t register_data,out bit<1> out_flag){
            if(register_data.src_addr == ig_md.key.pair.src_addr && register_data.dst_addr == ig_md.key.pair.dst_addr){
                out_flag = 1;
            }
            else{
                out_flag = 0;                
            }			
		}
	};

    // hash2
    Register<bit<32>,bit<32>>(32w5120) reg_for_flow_size_2;
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_size_2) size_count_2={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			register_data = register_data + len;
			flow_size = register_data;
		}
	};
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_size_2) size_check_2={
		void apply(inout bit<32> register_data, out bit<32> flow_size){
			flow_size = register_data;
		}
	};
        // reg for key: vni, ip_pair
    Register<bit<32>,bit<32>>(32w5120) reg_for_flow_key_vni_2;
    RegisterAction<bit<32>,bit<32>,bit<32>>(reg_for_flow_key_vni_2) key_vni_update_2={
		void apply(inout bit<32> register_data){
            register_data = ig_md.key.vni;	
		}
	};
    RegisterAction<bit<32>,bit<32>,bit<1>>(reg_for_flow_key_vni_2) key_vni_check_2={
		void apply(inout bit<32> register_data,out bit<1> out_flag){
            if(register_data == ig_md.key.vni){
                out_flag = 1;
            }
            else{
                out_flag = 0;                
            }			
		}
	};

    Register<ip_pair_t,bit<32>>(32w5120) reg_for_flow_key_ip_pair_2;
    RegisterAction<ip_pair_t,bit<32>,bit<32>>(reg_for_flow_key_ip_pair_2) key_ip_pair_update_2={
		void apply(inout ip_pair_t register_data){
            register_data.src_addr = ig_md.key.pair.src_addr;
            register_data.dst_addr = ig_md.key.pair.dst_addr;	
		}
	};
    RegisterAction<ip_pair_t,bit<32>,bit<1>>(reg_for_flow_key_ip_pair_2) key_ip_pair_check_2={
		void apply(inout ip_pair_t register_data,out bit<1> out_flag){
            if(register_data.src_addr == ig_md.key.pair.src_addr && register_data.dst_addr == ig_md.key.pair.dst_addr){
                out_flag = 1;
            }
            else{
                out_flag = 0;                
            }			
		}
	};
    // get 2 sketch value by 3 tuple
    action get_hash_value_1(){
        ig_md.hash_value1 = hash1.get({ig_md.key.vni, ig_md.key.pair.src_addr, ig_md.key.pair.dst_addr});
    }
    action get_hash_value_2(){
        ig_md.hash_value2 = hash2.get({ig_md.key.vni, ig_md.key.pair.src_addr, ig_md.key.pair.dst_addr});

    }
    action get_reg_value_1(){
        ig_md.hash_size1 = size_check_1.execute(ig_md.hash_value1);
    }
    action get_reg_value_2(){
        ig_md.hash_size2 = size_check_2.execute(ig_md.hash_value2);
    }
    action get_sketch_value_1(){
        get_hash_value_1();
        get_reg_value_1();
    }
    action get_sketch_value_2(){
        get_hash_value_2();
        get_reg_value_2();
    }
    table get_sketch_value_table_1{ //获取数据包的sketch信息
        actions = {
            get_sketch_value_1;
        }
        size = 1;
        const default_action = get_sketch_value_1();
    }
    table get_sketch_value_table_2{
        actions = {
            get_sketch_value_2;
        }
        size = 1;
        const default_action = get_sketch_value_2();
    }
    
    // check key and update key
    action update_hash_key_vni_1(){
        key_vni_update_1.execute(ig_md.hash_value1);
    }
    action update_hash_key_ip_pair_1(){
        key_ip_pair_update_1.execute(ig_md.hash_value1);
    }
    action update_hash_key_vni_2(){
        key_vni_update_2.execute(ig_md.hash_value2);
    }
    action update_hash_key_ip_pair_2(){
        key_ip_pair_update_2.execute(ig_md.hash_value2);
    }
    table update_hash_key_vni_table_1{ // 更新hash值对应的key
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            update_hash_key_vni_1;
            NoAction;
        }
        const entries = {
            0 : update_hash_key_vni_1();
        }
        size = 1;
        default_action = NoAction;
    }
    table update_hash_key_ip_pair_table_1{ // 更新hash值对应的key
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            update_hash_key_ip_pair_1;
            NoAction;
        }
        const entries = {
            0 : update_hash_key_ip_pair_1();
        }
        size = 1;
        default_action = NoAction;
    }
    table update_hash_key_vni_table_2{ // 更新hash值对应的key
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            update_hash_key_vni_2;
            NoAction;
        }
        const entries = {
            1 : update_hash_key_vni_2();
        }
        size = 1;
        default_action = NoAction;
    }
    table update_hash_key_ip_pair_table_2{ // 更新hash值对应的key
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            update_hash_key_ip_pair_2;
            NoAction;
        }
        const entries = {
            1 : update_hash_key_ip_pair_2();
        }
        size = 1;
        default_action = NoAction;
    }

    action check_hash_key_vni_1(){
        ig_md.key_vni_flag = key_vni_check_1.execute(ig_md.hash_value1);
    }
    action check_hash_key_ip_pair_1(){
        ig_md.key_ip_pair_flag =  key_ip_pair_check_1.execute(ig_md.hash_value1);
    }
    action check_hash_key_vni_2(){
        ig_md.key_vni_flag = key_vni_check_2.execute(ig_md.hash_value2);
    }
    action check_hash_key_ip_pair_2(){
        ig_md.key_ip_pair_flag =  key_ip_pair_check_2.execute(ig_md.hash_value2);
    }
    table check_hash_key_vni_table_1{ //检查key值是否相同，会输出一个标记值
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            check_hash_key_vni_1;
            NoAction;
        }

        const entries = {
            0 : check_hash_key_vni_1();
        }
        size = 1;
        default_action = NoAction;
    }
    table check_hash_key_ip_pair_table_1{ //检查key值是否相同，会输出一个标记值
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            check_hash_key_ip_pair_1;
            NoAction;
        }

        const entries = {
            0 : check_hash_key_ip_pair_1();
        }
        size = 1;
        default_action = NoAction;
    }
    table check_hash_key_vni_table_2{ //检查key值是否相同，会输出一个标记值
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            check_hash_key_vni_2;
            NoAction;
        }

        const entries = {
            1 : check_hash_key_vni_2();
        }
        size = 1;
        default_action = NoAction;
    }
    table check_hash_key_ip_pair_table_2{ //检查key值是否相同，会输出一个标记值
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            check_hash_key_ip_pair_2;
            NoAction;
        }

        const entries = {
            1 : check_hash_key_ip_pair_2();
        }
        size = 1;
        default_action = NoAction;
    }
    // update reg
    action count_size_1(){
        ig_md.hash_size1 = size_count_1.execute(ig_md.hash_value1);
    }
    action count_size_2(){
        ig_md.hash_size2 = size_count_2.execute(ig_md.hash_value2);
    }
    table count_size_table_1{ //更新hash值对应的size
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            count_size_1;
            NoAction;
        }
        const entries = {
            0 : count_size_1();
        }
        size = 1;
        default_action = NoAction;
    }
    table count_size_table_2{ //更新hash值对应的size
        key = {
            ig_md.cmp_flag : exact;
        }
        actions = {
            count_size_2;
            NoAction;
        }
        const entries = {
            1 : count_size_2();
        }
        size = 1;
        default_action = NoAction;
    }

    //   ---------------在apply中实现size比较-----------------
    // bit<1> flag;  // ?
    // select the min_size and hash_fun then compute it's probability
    Register<bit<32>,bit<1>>(1) num_32;
	MathUnit<bit<32>>(true,0,9,{68,73,78,85,93,102,113,128,0,0,0,0,0,0,0,0}) prog_64K_div_mu;
	RegisterAction<bit<32>,bit<1>,bit<32>>(num_32) prog_64K_div_x = {
		void apply(inout bit<32> register_data, out bit<32> mau_value){
            // if(ig_md.hash_size1 > ig_md.hash_size2){
            //     register_data = ig_md.hash_size2;
            //     flag = 0;
            // }
            // else{
            //     register_data = ig_md.hash_size1;
            //     flag = 1;
            // }
            // ig_md.hash_min_size = register_data;
            register_data = prog_64K_div_mu.execute(ig_md.hash_size);
			mau_value = register_data; 
		}
	};

    Random<bit<16>>() random_generator;
	action generate_random_number(){
		ig_md.rnd = random_generator.get();
	}
    action calc_cond(){
        ig_md.hash_cond = (bit<16>)prog_64K_div_x.execute(0);
    }
    action cond_rnd(){
        generate_random_number();
        calc_cond();
    }
	table cond_rnd_table{ //根据最小的size计算概率值，并生成随机数，以进行概率比较
		actions = {
			cond_rnd;
		}
		size = 1;
		const default_action = cond_rnd();
	}
    action hash_size_cmp(){
        ig_md.hash_size_m = ig_md.hash_size1 - ig_md.hash_size2;
    }
    table hash_size_cmp_table{
        actions = {
            hash_size_cmp;
        }
        size = 1;
        const default_action = hash_size_cmp();
    }

    action calc_cond_cmp(){
        ig_md.hash_cond = ig_md.hash_cond - ig_md.rnd;
	}
	table calc_cond_cmp_table{
		actions = {
			calc_cond_cmp;
		}
		size = 1;
		const default_action = calc_cond_cmp();
	}

    apply {
        count_all_size_table.apply();

        get_sketch_value_table_1.apply();
        get_sketch_value_table_2.apply();
        hash_size_cmp_table.apply();
        if(ig_md.hash_size_m > 0){
            ig_md.cmp_flag = 1;
            // ig_md.hash_value = ig_md.hash_value2;
            ig_md.hash_size = ig_md.hash_size2;
        }
        else{
            ig_md.cmp_flag = 0;
            // ig_md.hash_value = ig_md.hash_value1;
            ig_md.hash_size = ig_md.hash_size1;
        }

        count_size_table_1.apply();
        count_size_table_2.apply();
        
        check_hash_key_vni_table_1.apply();
        check_hash_key_ip_pair_table_1.apply();
        check_hash_key_vni_table_2.apply();
        check_hash_key_ip_pair_table_2.apply();
        if(ig_md.key_vni_flag == 1 && ig_md.key_ip_pair_flag ==1){
            ig_md.key_flag = 1;
        }
        else{
            ig_md.key_flag = 0;
        }

        cond_rnd_table.apply();

        if(ig_md.key_flag == 0){ //key不同，需要更新
            calc_cond_cmp_table.apply();
            if(ig_md.hash_cond > 0){ //概率更新
                ig_md.key_change = 1; //若更新，key_change位置1
                update_hash_key_vni_table_1.apply();
                update_hash_key_ip_pair_table_1.apply();
                update_hash_key_vni_table_2.apply();
                update_hash_key_ip_pair_table_2.apply();
            }
        }
        if(ig_md.hash_size > 2000){ //threshold = 2000;
            ig_md.size_flag = 1;
        }
        // if(ig_md.key_flag == 1 && ig_md.hash_min_size > bit<32>(ig_md.all_flow_size >> 3)){
        //     ig_md.e_flow = 1;
        // }
        // else{
        //     ig_md.e_flow = 0;
        // }
    }

}


control ShcIngress( inout shc_header_t hdr,
                 inout shc_ingress_metadata_t ig_md,
                 
                 in    ingress_intrinsic_metadata_t               ig_intr_md,
                 in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
                 inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
                 inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md    )
{

    Sketch() sk;
    bit<32> len = (bit<32>)(hdr.inner_ipv4.total_len >> 3);
    nexthop_id_t nexthop_id = 0;
    bit<1> miss_flag;
    action set_nexthop(nexthop_id_t nexthop) {
        nexthop_id = nexthop;
        miss_flag = 0;
        // ig_tm_md.copy_to_cpu = 0; //为了防止ig_tm_md.copy_to_cpu无值
    }
    action report_and_to_nfv(){
        ig_tm_md.ucast_egress_port = 32; //指定端口去NFV
        miss_flag = 1;
        // ig_tm_md.copy_to_cpu = 1; //miss包上报，待完善-------考虑使用mirror
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
        sk.apply(len, ig_md);

        if (hdr.vxlan.isValid() && hdr.inner_ipv4.isValid()) {
            if(ig_md.size_flag == 1){
                eflow_forward.apply();
                if(miss_flag == 0){
                        nexthop.apply();
                    }
                if(ig_md.key_flag == 0 && ig_md.key_change == 0){
                    ig_tm_md.copy_to_cpu = 0;
                }
                else{
                    ig_tm_md.copy_to_cpu = 1;
                }
            }
            else{
                ig_tm_md.ucast_egress_port = 32; //指定端口去NFV
            }          
        }
    }
}

//ingress deparser
control ShcIngressDeparser(
    packet_out pkt,
    inout shc_header_t hdr,
    in shc_ingress_metadata_t ig_md,
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
    inout shc_egress_metadata_t                     meta,
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
control ShcEgressDeparser(
        packet_out pkt,
        inout shc_header_t hdr,
        in shc_egress_metadata_t eg_md,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr){

    apply{

    }
}

Pipeline(ShcIngressParser(),
        ShcIngress(),
        ShcIngressDeparser(),
        ShcEgressParser(),
        ShcEgress(),
        ShcEgressDeparser()) pipe;

Switch(pipe) main;