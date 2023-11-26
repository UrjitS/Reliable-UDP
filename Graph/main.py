"""
Main file for the graphing program
"""
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

def update(num):
    """
    Update the plot with data from the file.
    """
    packet_sequence_numbers = []
    timestamps = []

    with open('/home/wumbo/Documents/Reliable-UDP/Client/output.txt', 'r') as f:
        for line in f:
            packet_sequence_number, timestamp = map(int, line.split(','))
            packet_sequence_numbers.append(packet_sequence_number)
            timestamps.append(timestamp)

    plt.cla()  # Clear the current axes.
    plt.plot(packet_sequence_numbers, timestamps)
    plt.xlabel('Packet Sequence Number')
    plt.ylabel('Time (s)')
    plt.title('Time vs. Packet Sequence Number')

def main():
    """
    Main function for the graphing program
    """
    anim = FuncAnimation(plt.gcf(), update, interval=1000, cache_frame_data=False)  # Update every 1000ms.
    plt.show()

if __name__ == '__main__':
    main()
