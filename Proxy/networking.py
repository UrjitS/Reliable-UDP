"""
Implements the networking for the proxy
"""

import socket
import options

def print_ip():
    """
    Prints and returns the IP address of the networking interface on this machine.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        ip = s.getsockname()[0]
    except Exception: # pylint: disable=broad-except
        ip = '127.0.0.1'
    finally:
        s.close()

    print(ip)
    return ip

def create_udp(bind_port: int):
    """
    Creates a UDP socket and binds it to this machine's IP address.
    """
    # Create a UDP socket
    ip_address = print_ip()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Bind the socket to this machine's IP address and the specified port
    sock.bind((ip_address, bind_port))

    return sock

def forward_data(receiver_ip: str, receiver_port: int, bind_port: int):
    """
    Forwards data from the sender to the receiver
    """
    first_packet = False
    client_addr = None

    print("Forwarding data")

    socket_fd = create_udp(bind_port)

    while options.RUNNING:
        data, addr = socket_fd.recvfrom(1024)  # buffer size is 1024 bytes

        if not first_packet:
            client_addr = addr
            first_packet = True

        if b'\x03\x03' in data:  # ETX character is 0x03 in ASCII
            if (addr is not None) and (addr[0] != receiver_ip):
                socket_fd.sendto(data, (receiver_ip, receiver_port))
                data = None
            elif addr is not None:
                socket_fd.sendto(data, client_addr)
                data = None
    