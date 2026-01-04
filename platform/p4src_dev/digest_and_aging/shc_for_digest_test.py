# 在bfrt_python中执行 $load shc_for_digest_test.py
import os
os.environ['SDE'] = "/".join(os.environ['PATH'].split(":")[0].split("/"))
os.environ['SDE_INSTALL'] = "/".join([os.environ['SDE'], 'install'])
print("%env SDE         {}".format(os.environ['SDE']))
print("%env SDE_INSTALL {}".format(os.environ['SDE_INSTALL']))

p4 = bfrt.shc_for_digest.pipe
p4_learn =  bfrt.shc_for_digest.pipe.ShcIngressDeparser

def my_learning_cb(dev_id, pipe_id, direction, parser_id, session, msg):
    global p4
    
    # smac = p4.Ingress.smac
    # dmac = p4.Ingress.dmac

    for digest in msg:
        vrf  = digest["vrf_digest"]
        src_ip = digest["src_ip_digest"]
        dst_ip = digest["dst_ip_digest"]

        print("vrf=%d src_ip=0x%08X dst_ip=0x%08X" % (vrf, src_ip, dst_ip))

    return 0

try:
    p4_learn.flowkey_digest.callback_deregister()
except:
    pass
finally:
    print("Deregistering old learning callback (if any)")
          
p4_learn.flowkey_digest.callback_register(my_learning_cb)
print("Learning callback registered")

# def my_aging_cb(dev_id, pipe_id, direction, parser_id, entry):
#     global p4

#     smac = p4.Ingress.smac
#     dmac = p4.Ingress.dmac
    
#     mac = entry.key[b'hdr.ethernet.src_addr']
#     vid = entry.key[b'meta.vid']

#     print("Aging out: VID=%d, MAC=0x%012X"%(vid, mac))
    
#     entry.remove() # from smac
#     try:
#         dmac.delete(vid=vid, dst_addr=mac)
#     except:
#         print("WARNING: Could not find the matching DMAC entry")

# p4.Ingress.smac.idle_table_set_notify(enable=False)
# print("Deregistering old aging callback (if any)")

# p4.Ingress.smac.idle_table_set_notify(enable=True, callback=my_aging_cb,
#                                       interval=10000,
#                                       min_ttl=10000, max_ttl=60000)
# print("Aging callback registered")
