import socket

PDM_PORT = 6768
POD_PORT = 6767

def main():
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

                # This is where we would do the malicious packet injection stuff >:)

                data = podsock.recv(1024)
                pdmsock.sendall(data)


if __name__ == "__main__":
    main()
