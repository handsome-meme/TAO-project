# 在bfrt_python中执行 $load shc_for_digest_test.py
import os
os.environ['SDE'] = "/".join(os.environ['PATH'].split(":")[0].split("/"))
os.environ['SDE_INSTALL'] = "/".join([os.environ['SDE'], 'install'])
print("%env SDE         {}".format(os.environ['SDE']))
print("%env SDE_INSTALL {}".format(os.environ['SDE_INSTALL']))

p4 = bfrt.shc_for_aging.pipe
p4_learn =  bfrt.shc_for_aging.pipe.ShcIngressDeparser

def my_learning_cb(dev_id, pipe_id, direction, parser_id, session, msg):
    global p4
    
    # smac = p4.Ingress.smac
    # dmac = p4.Ingress.dmac
    route_tbl = p4.ShcIngress.route_tbl

    for digest in msg:
        vrf  = digest["vrf_digest"]
        src_ip = digest["src_ip_digest"]
        dst_ip = digest["dst_ip_digest"]

        print("vrf=%d src_ip=0x%08X dst_ip=0x%08X ------learning and insert" % (vrf, src_ip, dst_ip))

        route_tbl.entry_with_route_for_nexthop(
            vrf=vrf, src_addr=src_ip, dst_addr=dst_ip, nhop_id=vrf,
            ENTRY_TTL=10000
        ).push()

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

    # smac = p4.Ingress.smac
    # dmac = p4.Ingress.dmac
    route_tbl = p4.ShcIngress.route_tbl

    vrf = entry.key[b'ig_md.vrf']
    src_ip = entry.key[b'hdr.inner_ipv4.src_addr']
    dst_ip = entry.key[b'hdr.inner_ipv4.dst_addr']

    print("vrf=%d src_ip=0x%08X dst_ip=0x%08X ------aging and delete" % (vrf, src_ip, dst_ip))
    
    # entry.remove() # from route_tbl
    try:
        route_tbl.delete(vrf=vrf, src_addr=src_ip, dst_addr=dst_ip)
        print("cb to delete")
    except:
        print("WARNING: Could not find the matching DMAC entry")

p4.ShcIngress.route_tbl.idle_table_set_notify(enable=False)
print("Deregistering old aging callback (if any)")

p4.ShcIngress.route_tbl.idle_table_set_notify(enable=True, callback=my_aging_cb,
                                      interval=1000,
                                      min_ttl=1000, max_ttl=10000)
print("Aging callback registered")
