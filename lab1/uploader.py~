import urllib.parse
import urllib.request

import os
import os.path
import sys

size = os.stat(sys.argv[1]).st_size
size_str = "".join(map(lambda x:f"{x:#04x}"[2:], int.to_bytes(size, 4, "little")))
print(size, size_str)
data = urllib.parse.urlencode({
    "WINID": "0",
    "DATA": size_str,
    "CMD": "WriteHex",
}).encode("ascii")
urllib.request.urlopen("http://127.0.0.1:8081", data)

data = urllib.parse.urlencode({
    "WINID": "0",
    "DATA": os.path.abspath(sys.argv[1]),
    "CMD": "SendTextFile",
}).encode("ascii")
urllib.request.urlopen("http://127.0.0.1:8081", data)
