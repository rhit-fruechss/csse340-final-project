import socket

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(("", 6767))
    s.listen(5)
    conn, addr = s.accept()
    with conn:
        print("Connection accepted: ", addr)
        while True:
            input()
            conn.sendall(b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x1c\x00\x00\x00\x00\x00\x00\x00\xef\xbe\xad\xde\x02\x00\x00\x00")
