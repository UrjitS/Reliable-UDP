"""
Main file for the graphing program
"""
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import argparse
import os

FILE_NAME = ''
last_mod_time = None

def update(num):
    """
    Update the plot with data from the file.
    """
    global last_mod_time
    file_path = './output.txt'
    
    current_mod_time = os.path.getmtime(file_path)
    if last_mod_time == current_mod_time:
        return  # File hasn't been modified, so don't update the graph.
    last_mod_time = current_mod_time

    packet_sequence_numbers = []
    timestamps = []

    with open(file_path, 'r', encoding="utf-8") as f:
        for line in f:
            packet_sequence_number, timestamp = map(int, line.split(','))
            packet_sequence_numbers.append(packet_sequence_number)
            timestamps.append(timestamp)

    plt.cla()  # Clear the current axes.
    plt.scatter(packet_sequence_numbers, timestamps)
    plt.xlabel('Packet Sequence Number')
    plt.ylabel('Time (s)')
    plt.title('Time vs. Packet Sequence Number')

def main():
    """
    Main function for the graphing program
    """
    global FILE_NAME
    global last_mod_time
    
    parser = argparse.ArgumentParser(description='Graph Program.')
    parser.add_argument('-s',    type=str, required=False, help='Server Flag')
    parser.add_argument('-c',    type=str, required=False, help='Client Flag')
    parser.add_argument('-p',    type=str, required=False, help='Proxy Flag')
    args = parser.parse_args()
    
    if args.s:
        print("Server")
        last_mod_time = os.path.getmtime(args.s)
        FILE_NAME = args.s
        plt.title('Server Time vs. Packet Sequence Number')
    elif args.c:
        print(f"Client File: {args.c}")
        last_mod_time = os.path.getmtime(args.c)
        FILE_NAME = args.c
        plt.title('Client Time vs. Packet Sequence Number')
        try:
            anim = FuncAnimation(plt.gcf(), update, interval=1000, cache_frame_data=False)  # Update every 1000ms.
            plt.show()
        except KeyboardInterrupt:
            print("Interrupted by user. Exiting...")
            return
    elif args.p:
        last_mod_time = os.path.getmtime(args.p)
        FILE_NAME = args.p
        print("Proxy")
        plt.title('Proxy Time vs. Packet Sequence Number')
    else:
        print("Invalid arguments.")
        return

if __name__ == '__main__':
    main()