from ipaddress import ip_address

p4 = bfrt.test1.pipe

# This function can clear all the tables and later on other fixed objects
# once bfrt support is added.
def clear_all(verbose=True, batching=True):
    global p4
    global bfrt
    
    def _clear(table, verbose=False, batching=False):
        if verbose:
            print("Clearing table {:<40} ... ".
                  format(table['full_name']), end='', flush=True)
        try:    
            entries = table['node'].get(regex=True, print_ents=False)
            try:
                if batching:
                    bfrt.batch_begin()
                for entry in entries:
                    entry.remove()
            except Exception as e:
                print("Problem clearing table {}: {}".format(
                    table['name'], e.sts))
            finally:
                if batching:
                    bfrt.batch_end()
        except Exception as e:
            if e.sts == 6:
                if verbose:
                    print('(Empty) ', end='')
        finally:
            if verbose:
                print('Done')

        # Optionally reset the default action, but not all tables
        # have that
        try:
            table['node'].reset_default()
        except:
            pass
    
    # The order is important. We do want to clear from the top, i.e.
    # delete objects that use other objects, e.g. table entries use
    # selector groups and selector groups use action profile members
    

    # Clear Match Tables
    for table in p4.info(return_info=True, print_info=False):
        if table['type'] in ['MATCH_DIRECT', 'MATCH_INDIRECT_SELECTOR']:
            _clear(table, verbose=verbose, batching=batching)

    # Clear Selectors
    for table in p4.info(return_info=True, print_info=False):
        if table['type'] in ['SELECTOR']:
            _clear(table, verbose=verbose, batching=batching)
            
    # Clear Action Profiles
    for table in p4.info(return_info=True, print_info=False):
        if table['type'] in ['ACTION_PROFILE']:
            _clear(table, verbose=verbose, batching=batching)
    
#clear_all()

# ipv4_host = p4.Ingress.ipv4_host
# ipv4_host.add_with_send(dst_addr=ip_address('192.168.1.1'),   port=1)
# ipv4_host.add_with_send(dst_addr=ip_address('192.168.1.2'),   port=2)
# ipv4_host.add_with_drop(dst_addr=ip_address('192.168.1.3'))
# ipv4_host.add_with_send(dst_addr=ip_address('192.168.1.254'), port=64)

eflow_forward = p4.SwitchIngress.eflow_forward
eflow_forward.add_with_set_nexthop(vni=1,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=1)
eflow_forward.add_with_set_nexthop(vni=2,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=2)
eflow_forward.add_with_set_nexthop(vni=3,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=3)

# bfrt.test1.pipe.SwitchIngress.eflow_forward.add_with_set_nexthop(vni=1,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=1)
# bfrt.test1.pipe.SwitchIngress.eflow_forward.add_with_set_nexthop(vni=2,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=2)
# bfrt.test1.pipe.SwitchIngress.eflow_forward.add_with_set_nexthop(vni=3,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=3)
# bfrt.test1.pipe.SwitchIngress.eflow_forward.add_with_set_nexthop(vni=4,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=4)
# bfrt.test1.pipe.SwitchIngress.eflow_forward.add_with_set_nexthop(vni=5,dst_addr='10.0.0.11',src_addr='10.0.0.1',nexthop=5)



#  -----------------------for shc_v1_single_hash_with_dp.tunnel_map_vrf-------------
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=1,vrf=1)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=2,vrf=2)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=3,vrf=3)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=4,vrf=4)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=5,vrf=5)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=6,vrf=6)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=7,vrf=7)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.tunnel_map_vrf_tbl.add_with_tunnel_map_vrf(type=1,vni=8,vrf=8)

#  -----------------------for shc_v1_single_hash_with_dp.route-------------
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=1,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=1)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=7,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=2)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=2,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=3)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=3,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=4)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=4,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=5)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=5,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=6)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.route_tbl.add_with_route_for_nexthop(vrf=6,src_addr='10.0.0.1',dst_addr='10.0.0.11',nhop_id=7)

#  -----------------------for shc_v1_single_hash_with_dp.nhop-------------
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=1,vni=11,local='20.0.0.2',remote='20.0.0.22',neigh_id=1)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=2,vni=22,local='20.0.0.2',remote='20.0.0.22',neigh_id=2)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=3,vni=33,local='20.0.0.2',remote='20.0.0.22',neigh_id=3)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=4,vni=44,local='20.0.0.2',remote='20.0.0.22',neigh_id=4)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=5,vni=55,local='20.0.0.2',remote='20.0.0.22',neigh_id=5)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=6,vni=66,local='20.0.0.2',remote='20.0.0.22',neigh_id=6)
# bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.nhop_tbl.add_with_nhop(nhop_id=7,vni=77,local='20.0.0.2',remote='20.0.0.22',neigh_id=7)

#  -----------------------for shc_v1_single_hash_with_dp.neigh-------------
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=1,dmac="00:00:00:00:00:11")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=2,dmac="00:00:00:00:00:22")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=3,dmac="00:00:00:00:00:33")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=4,dmac="00:00:00:00:00:44")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=5,dmac="00:00:00:00:00:55")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=6,dmac="00:00:00:00:00:66")
bfrt.shc_v1_single_hash_with_dp.pipe.ShcIngress.neigh_tbl.add_with_neigh(neigh_id=7,dmac="00:00:00:00:00:77")





nexthop = p4.SwitchIngress.nexthop
nexthop.add_with_send(nexthop_id=0,port=0)
nexthop.add_with_send(nexthop_id=1,port=1)
nexthop.add_with_send(nexthop_id=2,port=2)
nexthop.add_with_send(nexthop_id=3,port=3)



# bfrt.test1.pipe.SwitchIngress.nexthop.add_with_rewrite_outer_header(nexthop_id=1,port=1,vni=11,src_addr="10.0.1.11",dst_addr="10.0.1.1",src_mac="11:11:11:11:11:11",dst_mac="11:11:11:11:11:12")
# bfrt.test1.pipe.SwitchIngress.nexthop.add_with_rewrite_outer_header(nexthop_id=2,port=2,vni=22,src_addr="10.0.2.11",dst_addr="10.0.2.1",src_mac="11:11:11:11:11:21",dst_mac="11:11:11:11:11:22")
# bfrt.test1.pipe.SwitchIngress.nexthop.add_with_rewrite_outer_header(nexthop_id=3,port=3,vni=33,src_addr="10.0.3.11",dst_addr="10.0.3.1",src_mac="11:11:11:11:11:31",dst_mac="11:11:11:11:11:32")
# bfrt.test1.pipe.SwitchIngress.nexthop.add_with_rewrite_outer_header(nexthop_id=4,port=4,vni=44,src_addr="10.0.4.11",dst_addr="10.0.4.1",src_mac="11:11:11:11:11:41",dst_mac="22:d0:2e:3d:42:c2")
# bfrt.test1.pipe.SwitchIngress.nexthop.add_with_send(nexthop_id=5,port=4)

bfrt.complete_operations()

# Final programming
print("""
******************* PROGAMMING RESULTS *****************
""")
print ("Table eflow_forward:")
eflow_forward.dump(table=True)
print ("Table nexthop:")
nexthop.dump(table=True)

                       
