"""
Yes
"""

import argparse
import ipaddress

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

def main ():
    """
    Main function for the proxy server
    """
    parser = argparse.ArgumentParser(description='Proxy server program.')
    parser.add_argument('-r', type=str, required=True, help='Receiver IP Address')
    parser.add_argument('-a', type=int, required=True, help='Receiver Port Number')
    parser.add_argument('-p', type=int, required=True, help='Port to bind to')

    args = parser.parse_args()

    if not check_ip(args.r) or not check_port(args.a) or not check_port(args.p):
        print("Invalid arguments.")
        return

    print(f"Receiver IP Address: {args.r}")
    print(f"Receiver Port Number: {args.a}")
    print(f"Port to bind to: {args.p}")

    
    
if __name__ == '__main__':
    main()
    