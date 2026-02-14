import socket

def print_hex(b: bytes):
    for i in b:
        print("%02x" % i, end="")
    print("")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(("", 6767))
    s.listen(5)
    conn, addr = s.accept()
    try:
        with conn:
            print("Connection accepted: ", addr)
            while True:
                input()
                conn.sendall(b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00\xef\xbe\xad\xde\x02\x00\x00\x00")
                print_hex(conn.recv(32))
                conn.sendall(b"\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\xef\xbe\xad\xde\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
                print_hex(conn.recv(32))

    finally:
        conn.close()
        s.close()
