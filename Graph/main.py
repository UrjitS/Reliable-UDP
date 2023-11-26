"""
Main file for the graphing program
"""




import matplotlib.pyplot as plt

def main():
    """
    Main function for the graphing program
    """
    packet_sequence_numbers = []
    timestamps = []

    with open('output.txt', 'r') as f:
        for line in f:
            packet_sequence_number, timestamp = map(int, line.split(','))
            packet_sequence_numbers.append(packet_sequence_number)
            timestamps.append(timestamp)

    plt.plot(timestamps, packet_sequence_numbers)
    plt.xlabel('Timestamp')
    plt.ylabel('Packet Sequence Number')
    plt.title('Packets over Time')
    plt.show()

if __name__ == '__main__':
    main()