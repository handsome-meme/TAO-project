    nexthop_id_t    nexthop_id = 0;

    action send(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
        // ig_tm_md.bypass_egress     = 1;//改为 = true
    }


    action drop() {
        ig_dprsr_md.drop_ctl = 1;
    }

    action set_nexthop(nexthop_id_t nexthop) {
        nexthop_id = nexthop;
    }
    action report_and_to_nfv(){
        ig_tm_md.ucast_egress_port = 32; //指定端口去NFV
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
        default_action = mirror_and_to_nfv();
        size    = SZIE;
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
        hdr.ethernet.src_mac = src_mac;
        hdr.ethernet.dst_mac = dst_mac;
    }
    table nexthop{
        key = { nexthop_id : exact; }
        actions = { rewrite_outer_header;}
        size = NEXTHOP_TABLE_SIZE;
    }

    apply{
        if (hdr.ipv4.isValid()) {
            eflow_forward.apply();
            nexthop.apply();
        }

    }