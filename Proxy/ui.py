"""
Generic UI to set the proxy settings live.
"""
import tkinter as tk
import tkinter.messagebox
import options

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

        self.sender_drop_chance = tk.StringVar()
        tk.Label(self.window, text="Sender Drop Chance").grid(row=0)
        tk.Entry(self.window, textvariable=self.sender_drop_chance).grid(row=0, column=1)

        self.receiver_drop_chance = tk.StringVar()
        tk.Label(self.window, text="Receiver Drop Chance").grid(row=1)
        tk.Entry(self.window, textvariable=self.receiver_drop_chance).grid(row=1, column=1)

        self.delay_range_low = tk.StringVar()
        self.delay_range_high = tk.StringVar()
        tk.Label(self.window, text="Delay Range Lower Bound").grid(row=2)
        tk.Entry(self.window, textvariable=self.delay_range_low).grid(row=2, column=1)
        tk.Label(self.window, text="Delay Range Upper Bound").grid(row=3)
        tk.Entry(self.window, textvariable=self.delay_range_high).grid(row=3, column=1)

        tk.Button(self.window, text="Save Changes", command=self.save_changes).grid(row=4, column=1)

        self.set_defaults()

    def set_defaults(self):
        """
        Set the default values for the form fields from options.py.
        """
        self.sender_drop_chance.set(str(options.SENDER_DROP_CHANCE))
        self.receiver_drop_chance.set(str(options.RECEIVER_DROP_CHANCE))
        self.delay_range_low.set(str(options.DELAY_RANGE[0]))
        self.delay_range_high.set(str(options.DELAY_RANGE[1]))

    def save_changes(self):
        """
        Save the changes made in the form to options.py.
        """
        try:
            sender_drop_chance = float(self.sender_drop_chance.get())
            receiver_drop_chance = float(self.receiver_drop_chance.get())
            delay_range_low = int(self.delay_range_low.get())
            delay_range_high = int(self.delay_range_high.get())

            if not 0 <= sender_drop_chance <= 1:
                raise ValueError("Sender Drop Chance must be a decimal between 0 and 1.")
            if not 0 <= receiver_drop_chance <= 1:
                raise ValueError("Receiver Drop Chance must be a decimal between 0 and 1.")
            if not 0 <= delay_range_low <= delay_range_high <= 1000:
                raise ValueError("Delay Range must be two integers between 0 and 1000, with LOWER_BOUND <= UPPERBOUND.")

            options.SENDER_DROP_CHANCE = sender_drop_chance
            options.RECEIVER_DROP_CHANCE = receiver_drop_chance
            options.DELAY_RANGE = (delay_range_low, delay_range_high)

            print(f"SENDER_DROP_CHANCE set to {sender_drop_chance}")
            print(f"RECEIVER_DROP_CHANCE set to {receiver_drop_chance}")
            print(f"DELAY_RANGE set to ({delay_range_low}, {delay_range_high})")
        except ValueError as e:
            tkinter.messagebox.showerror("Input Error", str(e))
