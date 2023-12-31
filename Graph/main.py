"""
Main file for the graphing program
pyinstaller --onefile main.py
"""
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import argparse
import re
import os


FILE_NAME = ''
last_mod_time = None
def update_client(num):
    """
    Update the plot with data from the file.
    """
    global last_mod_time
    
    current_mod_time = os.path.getmtime(FILE_NAME)
    if last_mod_time == current_mod_time:
        return  # File hasn't been modified, so don't update the graph.
    last_mod_time = current_mod_time

    packet_sequence_numbers = []
    timestamps = []

    with open(FILE_NAME, 'r', encoding="utf-8") as f:
        for line in f:
            try:
                packet_sequence_number, timestamp = map(int, line.split(','))
                packet_sequence_numbers.append(packet_sequence_number)
                timestamps.append(timestamp)
            except ValueError:
                print(f"Invalid data in file: {line}. Skipping this line.")

    plt.cla()  # Clear the current axes.
    plt.scatter(x=timestamps, y=packet_sequence_numbers)
    plt.title('Packet Sequence Number vs Time (s)')
    plt.xlabel('Time (s)')
    plt.ylabel('Packet Sequence Number')


def update_proxy(num):
    """
    Update the plot with data from the file.
    """
    global last_mod_time
    
    current_mod_time = os.path.getmtime(FILE_NAME)
    if last_mod_time == current_mod_time:
        return
    last_mod_time = current_mod_time

    sender_seq_nums = []
    sender_delays = []
    sender_dropped = []
    receiver_seq_nums = []
    receiver_delays = []
    receiver_dropped = []

    with open(FILE_NAME, 'r', encoding="utf-8") as f:
        for line in f:
            try:
                if "Sender packet dropped" in line:
                    seq_num = int(re.findall(r'\d+', line)[0])
                    sender_seq_nums.append(seq_num)
                    sender_delays.append(0)
                    sender_dropped.append(seq_num)
                elif "Sender packet delayed" in line:
                    delay, seq_num = map(int, re.findall(r'\d+', line))
                    sender_seq_nums.append(seq_num)
                    sender_delays.append(delay)
                elif "Receiver packet dropped" in line:
                    seq_num = int(re.findall(r'\d+', line)[0])
                    receiver_seq_nums.append(seq_num)
                    receiver_delays.append(0)
                    receiver_dropped.append(seq_num)
                elif "Receiver packet delayed" in line:
                    delay, seq_num = map(int, re.findall(r'\d+', line))
                    receiver_seq_nums.append(seq_num)
                    receiver_delays.append(delay)
            except ValueError:
                print(f"Invalid data in file: {line}. Skipping this line.")

    plt.clf()
    
    # Plot for sender
    plt.subplot(2, 1, 1)
    plt.plot(sender_seq_nums, sender_delays, label='Delays')
    plt.scatter(sender_dropped, [0]*len(sender_dropped), color='red', label='Dropped')
    plt.title(f'Client Packets')
    plt.xlabel('Sequence Number')
    plt.ylabel('Delay (ms)')
    plt.legend(loc='upper right')

    # Plot for receiver
    plt.subplot(2, 1, 2)
    plt.plot(receiver_seq_nums, receiver_delays, label='Delays')
    plt.scatter(receiver_dropped, [0]*len(receiver_dropped), color='red', label='Dropped')
    plt.title(f'Receiver Packets')
    plt.xlabel('Sequence Number')
    plt.ylabel('Delay (ms)')
    plt.legend(loc='upper right')

    plt.subplots_adjust(hspace=0.5, right=0.85)

def update_server(num):
    """
    Update the plot with data from the file.
    """
    global last_mod_time
    
    current_mod_time = os.path.getmtime(FILE_NAME)
    if last_mod_time == current_mod_time:
        return
    last_mod_time = current_mod_time

    packet_sequence_numbers = []
    timestamps = []

    with open(FILE_NAME, 'r', encoding="utf-8") as f:
        for line in f:
            try:
                packet_sequence_number, timestamp = map(int, line.split(','))
                packet_sequence_numbers.append(packet_sequence_number)
                timestamps.append(timestamp)
            except ValueError:
                print(f"Invalid data in file: {line}. Skipping this line.")

    plt.cla()
    plt.scatter(x=timestamps, y=packet_sequence_numbers)
    plt.title('ACK Packet Sequence Number vs. Time (s)')
    plt.xlabel('Time (s)')
    plt.ylabel('ACK Packet Sequence Number')

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
    
    try:
        if args.s:
            FILE_NAME = args.s
            print(f"Server: {FILE_NAME}")
            last_mod_time = os.path.getmtime(FILE_NAME)
            plt.figure(num="Server Statistics")
            try:
                anim = FuncAnimation(plt.gcf(), update_server, interval=1000, cache_frame_data=False)  # Update every 1000ms.
                plt.show()
            except KeyboardInterrupt:
                print("Interrupted by user. Exiting...")
                return
        elif args.c:
            FILE_NAME = args.c
            print(f"Client File: {FILE_NAME}")
            last_mod_time = os.path.getmtime(FILE_NAME)
            plt.figure(num="Client Statistics")
            try:
                anim = FuncAnimation(plt.gcf(), update_client, interval=1000, cache_frame_data=False)  # Update every 1000ms.
                plt.show()
            except KeyboardInterrupt:
                print("Interrupted by user. Exiting...")
                return
        elif args.p:
            FILE_NAME = args.p
            last_mod_time = os.path.getmtime(FILE_NAME)
            plt.figure(num="Proxy Statistics")
            try:
                anim = FuncAnimation(plt.gcf(), update_proxy, interval=1000, cache_frame_data=False)  # Update every 1000ms.
                plt.show()
            except KeyboardInterrupt:
                print("Interrupted by user. Exiting...")
                return
            
        else:
            print("Invalid arguments.")
            return
    except FileNotFoundError:
        print(f"File {FILE_NAME} not found. Please check the file path and try again.")

if __name__ == '__main__':
    main()