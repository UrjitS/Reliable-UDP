"""
Main file for the graphing program
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
            packet_sequence_number, timestamp = map(int, line.split(','))
            packet_sequence_numbers.append(packet_sequence_number)
            timestamps.append(timestamp)

    plt.cla()  # Clear the current axes.
    plt.scatter(packet_sequence_numbers, timestamps)
    plt.xlabel('Packet Sequence Number')
    plt.ylabel('Time (s)')


# Initialize lists to store average delays
sender_avg_delays = []
receiver_avg_delays = []

def update_proxy(num):
    """
    Update the plot with data from the file.
    """
    # ... rest of your code ...

    sender_dropped = 0
    sender_delayed = 0
    sender_delay_time = 0
    receiver_dropped = 0
    receiver_delayed = 0
    receiver_delay_time = 0

    with open(FILE_NAME, 'r', encoding="utf-8") as f:
        for line in f:
            if "Sender packet dropped" in line:
                sender_dropped += 1
            elif "Sender packet delayed" in line:
                sender_delayed += 1
                sender_delay_time += int(re.findall(r'\d+', line)[0])
            elif "Receiver packet dropped" in line:
                receiver_dropped += 1
            elif "Receiver packet delayed" in line:
                receiver_delayed += 1
                receiver_delay_time += int(re.findall(r'\d+', line)[0])

    # Calculate average delays and append to lists
    sender_avg_delays.append(sender_delay_time / sender_delayed if sender_delayed != 0 else 0)
    receiver_avg_delays.append(receiver_delay_time / receiver_delayed if receiver_delayed != 0 else 0)

    plt.clf()  # Clear the entire figure.
    
    # Plot for sender
    plt.subplot(2, 1, 1)
    plt.plot(sender_avg_delays)
    plt.title('Client Packets (Dropped: {})'.format(sender_dropped))
    plt.ylabel('Average Delay (ms)')

    # Plot for receiver
    plt.subplot(2, 1, 2)
    plt.plot(receiver_avg_delays)
    plt.title('Receiver Packets (Dropped: {})'.format(receiver_dropped))
    plt.ylabel('Average Delay (ms)')

    plt.subplots_adjust(hspace=0.5)  # Adjust the space between plots

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
        try:
            anim = FuncAnimation(plt.gcf(), update_client, interval=1000, cache_frame_data=False)  # Update every 1000ms.
            plt.title('Client Time vs. Packet Sequence Number')
            plt.show()
        except KeyboardInterrupt:
            print("Interrupted by user. Exiting...")
            return
    elif args.p:
        last_mod_time = os.path.getmtime(args.p)
        FILE_NAME = args.p
        print("Proxy")
        try:
            anim = FuncAnimation(plt.gcf(), update_proxy, interval=1000, cache_frame_data=False)  # Update every 1000ms.
            plt.show()
        except KeyboardInterrupt:
            print("Interrupted by user. Exiting...")
            return
        
    else:
        print("Invalid arguments.")
        return

if __name__ == '__main__':
    main()