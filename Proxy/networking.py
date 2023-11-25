"""
Implements the networking for the proxy
"""

import socket
import random
import threading
import time
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

def random_drop(drop_chance: int):
    """
    Returns True if the packet should be dropped, False otherwise.
    """
    return random.randint(1, 100) <= drop_chance

def random_delay(delay_upper_bound: int):
    """
    Returns a random delay between the specified range.
    """
    return random.randint(0, delay_upper_bound)

def delayed_send(socket_fd, data, address, delay):
    """
    Sends data after a delay.
    """
    time.sleep(delay / 1000)  # convert delay from ms to seconds
    socket_fd.sendto(data, address)

def forward_receiver(socket_fd, data, address):
    """
    Forwards data from the receiver to the sender
    """
    print("Forwarding receiver")

    # Randomly drop packet
    if random_drop(options.RECEIVER_DROP_CHANCE):
        print("Dropped packet")
        options.STATUS = "Dropped Sender Packet"
        return

    # Randomly delay packet
    delay = random_delay(options.DELAY_UPPER_BOUND)
    options.STATUS = f"Delaying Receiver packet by {delay}ms"

    threading.Thread(target=delayed_send, args=(socket_fd, data, address, delay)).start()


def forward_sender(socket_fd, data, address):
    """
    Forwards data from the sender to the receiver
    """
    print("Forwarding sender")

    # Randomly drop packet
    if random_drop(options.SENDER_DROP_CHANCE):
        print("Dropped packet")
        options.STATUS = "Dropped Receiver Packet"
        return

    # Randomly delay packet
    delay = random_delay(options.DELAY_UPPER_BOUND)
    print(f"Delaying packet by {delay}ms")
    options.STATUS = f"Delaying Sender packet by {delay}ms"


    threading.Thread(target=delayed_send, args=(socket_fd, data, address, delay)).start()


def forward_data(receiver_ip: str, receiver_port: int, bind_port: int):
    """
    Forwards data from the sender to the receiver
    """
    first_packet = False
    client_addr = None

    print("Forwarding data")

    socket_fd = create_udp(bind_port)

    while options.RUNNING:
        data, addr = socket_fd.recvfrom(1024)

        if not first_packet:
            client_addr = addr
            first_packet = True

        if b'\x03\x03' in data:  # ETX character is 0x03 in ASCII
            if (addr is not None) and (addr[0] != receiver_ip):
                # Client -> Receiver
                forward_receiver(socket_fd, data, (receiver_ip, receiver_port))
                data = None
            elif addr is not None:
                # Receiver -> Client
                forward_sender(socket_fd, data, client_addr)
                data = None
    