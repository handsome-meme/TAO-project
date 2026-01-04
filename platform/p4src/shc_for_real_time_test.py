# 在bfrt_python中执行 $load shc_for_digest_test.py
import os
os.environ['SDE'] = "/".join(os.environ['PATH'].split(":")[0].split("/"))
os.environ['SDE_INSTALL'] = "/".join([os.environ['SDE'], 'install'])
print("%env SDE         {}".format(os.environ['SDE']))
print("%env SDE_INSTALL {}".format(os.environ['SDE_INSTALL']))

p4 = bfrt.shc_real_time_sketch.pipe
p4_learn =  bfrt.shc_real_time_sketch.pipe.ShcIngressDeparser

# ---------- learn_digest ----------

def my_learning_cb(dev_id, pipe_id, direction, parser_id, session, msg):
    global p4
    
    # smac = p4.Ingress.smac
    # dmac = p4.Ingress.dmac
    route_sele_tbl = p4.ShcIngress.route_sele_tbl
    tunnel_map_vrf_tbl = p4.ShcIngress.tunnel_map_vrf_tbl

    for digest in msg:
        port = digest["port"]
        dtype = digest["type"]
        vni  = digest["vni_digest"]
        src_ip = digest["src_ip_digest"]
        dst_ip = digest["dst_ip_digest"]
        time_stamp = digest["time_stamp"]

        if dtype == 1 : 
            print("port=%d dtype=%d vni=%d src_ip=0x%08X dst_ip=0x%08X time_stamp=%d ------counting" % (port, dtype, vni, src_ip, dst_ip, time_stamp))
        
        else : 
            tunnel_map_vrf_tbl.entry_with_tunnel_map_vrf(
                type=1, vni=vni, vrf=vni
            ).push()
            print("type=%d vni=%d vrf=0x%d ------learning and insert" % (dtype, vni, vni))

            route_sele_tbl.entry_with_route_sele_for_nexthop(
                vrf=vni, src_addr=src_ip, dst_addr=dst_ip, nhop_id=vni,
                ENTRY_TTL=10000
            ).push()
            print("port=%d dtype=%d vni=%d src_ip=0x%08X dst_ip=0x%08X time_stamp=%d ------learning and insert" % (port, dtype, vni, src_ip, dst_ip, time_stamp))
        
    return 0

try:
    p4_learn.flowkey_digest.callback_deregister()
except:
    pass
finally:
    print("Deregistering old learning callback (if any)")
          
p4_learn.flowkey_digest.callback_register(my_learning_cb)
print("Learning callback registered")


def my_aging_cb(dev_id, pipe_id, direction, parser_id, entry):
    global p4

    route_sele_tbl = p4.ShcIngress.route_sele_tbl
    tunnel_map_vrf_tbl = p4.ShcIngress.tunnel_map_vrf_tbl

    vrf = entry.key[b'ig_md.vrf']
    src_ip = entry.key[b'hdr.inner_ipv4.src_addr']
    dst_ip = entry.key[b'hdr.inner_ipv4.dst_addr']

    print("vrf=%d src_ip=0x%08X dst_ip=0x%08X ------aging and delete" % (vrf, src_ip, dst_ip))
    
    entry.remove() # from route_sele_tbl
    try:
        tunnel_map_vrf_tbl.delete(type=1, vni=vrf) # vni = vrf
        print("cb to delete")
    except:
        print("WARNING: Could not find the matching tunnel_map_vrf_tbl entry")

p4.ShcIngress.route_sele_tbl.idle_table_set_notify(enable=False)
print("Deregistering old aging callback (if any)")

p4.ShcIngress.route_sele_tbl.idle_table_set_notify(enable=True, callback=my_aging_cb,
                                      interval=1000,
                                      min_ttl=1000, max_ttl=10000)
print("Aging callback registered")