"""
Generic UI to set the proxy settings live.
"""
import tkinter as tk
import tkinter.messagebox
import options
import signal

class UI:
    """
    Basic UI for the proxy server.
    """
    def __init__(self):
        """
        Initialize the UI window and create the form fields.
        """
        self.window = tk.Tk()
        self.window.title("Proxy Server Settings")
        self.window.protocol("WM_DELETE_WINDOW", self.on_close)

        self.sender_drop_chance = tk.StringVar()
        tk.Label(self.window, text="Sender Drop Chance").grid(row=0)
        tk.Entry(self.window, textvariable=self.sender_drop_chance).grid(row=0, column=1)

        self.receiver_drop_chance = tk.StringVar()
        tk.Label(self.window, text="Receiver Drop Chance").grid(row=1)
        tk.Entry(self.window, textvariable=self.receiver_drop_chance).grid(row=1, column=1)

        self.delay_ack_range_low = tk.StringVar()
        self.delay_data_range_high = tk.StringVar()
        tk.Label(self.window, text="Delay DATA Upper Bound (ms)").grid(row=3)
        tk.Entry(self.window, textvariable=self.delay_data_range_high).grid(row=3, column=1)

        tk.Label(self.window, text="Delay ACK Upper Bound (ms)").grid(row=4)
        tk.Entry(self.window, textvariable=self.delay_ack_range_low).grid(row=4, column=1)


        self.status = tk.StringVar()
        tk.Label(self.window, textvariable=self.status).grid(row=6)

        tk.Button(self.window, text="Save Changes", command=self.save_changes).grid(row=7, column=1)

        self.set_defaults()
        self.update_status()
        
        signal.signal(signal.SIGINT, self.handle_sigint)


    def handle_sigint(self, signum, frame):
        self.on_close()
    
    def update_status(self):
        """
        Updates the status every 500ms.
        """
        self.status.set(options.STATUS)
        self.window.after(500, self.update_status)

    def set_defaults(self):
        """
        Set the default values for the form fields from options.py.
        """
        self.sender_drop_chance.set(str(options.SENDER_DROP_CHANCE))
        self.receiver_drop_chance.set(str(options.RECEIVER_DROP_CHANCE))
        self.delay_data_range_high.set(str(options.DELAY_DATA_UPPER_BOUND))
        self.delay_ack_range_low.set(str(options.DELAY_ACK_UPPER_BOUND))

    def save_changes(self):
        """
        Save the changes made in the form to options.py.
        """
        try:
            sender_drop_chance    = int(self.sender_drop_chance.get())
            receiver_drop_chance  = int(self.receiver_drop_chance.get())
            delay_data_range_high = int(self.delay_data_range_high.get())
            delay_ack_range_high  = int(self.delay_ack_range_low.get())

            if not 0 <= sender_drop_chance <= 100:
                raise ValueError("Sender Drop Chance must be an integer between 0 and 100.")
            if not 0 <= receiver_drop_chance <= 100:
                raise ValueError("Receiver Drop Chance must be an integer between 0 and 100.")
            if not 0 <= delay_data_range_high <= 10000:
                raise ValueError("Delay Range must be two integers between 0 and 10000.")
            if not 0 <= delay_ack_range_high <= 10000:
                raise ValueError("Delay Range must be two integers between 0 and 10000.")

            options.SENDER_DROP_CHANCE = sender_drop_chance
            options.RECEIVER_DROP_CHANCE = receiver_drop_chance
            options.DELAY_DATA_UPPER_BOUND = delay_data_range_high
            options.DELAY_ACK_UPPER_BOUND = delay_ack_range_high

            print(f"SENDER_DROP_CHANCE set to {sender_drop_chance}")
            print(f"RECEIVER_DROP_CHANCE set to {receiver_drop_chance}")
            print(f"DELAY_DATA_RANGE set to {delay_data_range_high}")
            print(f"DELAY_ACK_RANGE set to {delay_ack_range_high}")
            options.STATUS = "Changes saved."

        except ValueError as e:
            tkinter.messagebox.showerror("Input Error", str(e))


    def on_close(self):
        """
        Set options.RUNNING to False and close the window.
        """
        print("Closing UI")
        options.RUNNING = False
        self.window.destroy()
