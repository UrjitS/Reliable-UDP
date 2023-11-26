"""
Main file for the proxy server
"""

import argparse
import ipaddress
import threading
from ui import UI
from networking import forward_data

def check_ip (ip):
    """
    Checks if the IP address is valid
    """
    try:
        ipaddress.ip_address(ip)
        return True
    except ValueError:
        return False


def check_port (port):
    """
    Checks if the port is valid
    """
    return bool(isinstance(port, int) and 0 <= port <= 65535)


def check_drop (drop):
    """
    Checks if the drop is valid
    """
    return bool(isinstance(drop, int) and 0 <= drop <= 100)


def main ():
    """
    Main function for the proxy server
    """
    parser = argparse.ArgumentParser(description='Proxy server program.')
    parser.add_argument('-rip',    type=str, required=True, help='Receiver IP Address')
    parser.add_argument('-rport',  type=int, required=True, help='Receiver Port Number')
    parser.add_argument('-port',   type=int, required=True, help='Port to bind to')
    parser.add_argument('-dropd',  type=int, required=True, help='% Chance to drop data')
    parser.add_argument('-dropa',  type=int, required=True, help='% Chance to drop ack')
    parser.add_argument('-delays', type=int, required=True, help='(ms) Delay for sending data')
    parser.add_argument('-delayr', type=int, required=True, help='(ms) Delay for sending ack')

    args = parser.parse_args()

    if not check_ip(args.rip) or not check_port(args.rport) or not check_port(args.port) or not check_drop(args.dropd) or not check_drop(args.dropa):
        print("Invalid arguments.")
        return

    print(f"Receiver IP Address:   {args.rip}")
    print(f"Receiver Port Number:  {args.rport}")
    print(f"Port to bind to:       {args.port}")
    print(f"% Chance to drop data: {args.dropd}")
    print(f"% Chance to drop ack:  {args.dropa}")


    forward_thread = threading.Thread(target=forward_data, args=(args.rip, args.rport, args.port))
    forward_thread.start()

    UI().window.mainloop()
    forward_thread.join()


if __name__ == '__main__':
    main()
    