import serial
import serial.tools.list_ports
import threading
import queue
import time
import csv
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

class SerialReader(threading.Thread):
    def __init__(self, port, baud, data_queue, error_cb):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.data_queue = data_queue
        self.error_cb = error_cb
        self.running = False

    def run(self):
        try:
            ser = serial.Serial(self.port, self.baud, timeout=1)
            self.running = True
        except Exception as e:
            self.error_cb(str(e))
            return

        while self.running:
            try:
                line = ser.readline().decode(errors="ignore").strip()
                if not line:
                    continue

                parts = line.split(",")
                if len(parts) < 4:
                    continue

                t = int(parts[0])
                ac = int(parts[1])
                amp = int(parts[2])
                finger = int(parts[3])

                self.data_queue.put((t, ac, amp, finger))
            except Exception:
                pass

        ser.close()

    def stop(self):
        self.running = False

class PPGApp:
    def __init__(self, root):
        self.root = root
        root.title("PPG Scope")

        self.data_queue = queue.Queue()
        self.reader = None
        self.data_log = []

        self.ac_data = []
        self.amp_data = []

        self.create_widgets()
        self.update_plot()

    def create_widgets(self):
        top = ttk.Frame(self.root)
        top.pack(fill=tk.X, padx=5, pady=5)

        ttk.Label(top, text="COM Port:").pack(side=tk.LEFT)
        self.port_box = ttk.Combobox(top, values=self.get_ports(), width=10)
        self.port_box.pack(side=tk.LEFT, padx=5)

        ttk.Button(top, text="Refresh", command=self.refresh_ports).pack(side=tk.LEFT)

        ttk.Button(top, text="Start", command=self.start).pack(side=tk.LEFT, padx=5)
        ttk.Button(top, text="Stop", command=self.stop).pack(side=tk.LEFT)

        ttk.Button(top, text="Save CSV", command=self.save_csv).pack(side=tk.RIGHT)

        # Figure
        fig = plt.Figure(figsize=(7, 4), dpi=100)
        self.ax1 = fig.add_subplot(211)
        self.ax2 = fig.add_subplot(212)

        self.ax1.set_title("PPG AC")
        self.ax1.set_ylim(-40, 40)

        self.ax2.set_title("Amplitude EMA")
        self.ax2.set_ylim(0, 30)

        self.line_ac, = self.ax1.plot([], [])
        self.line_amp, = self.ax2.plot([], [])

        self.canvas = FigureCanvasTkAgg(fig, master=self.root)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def get_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def refresh_ports(self):
        self.port_box["values"] = self.get_ports()

    def start(self):
        if self.reader and self.reader.running:
            return

        port = self.port_box.get()
        if not port:
            messagebox.showerror("Error", "No COM port selected")
            return

        self.data_log.clear()
        self.ac_data.clear()
        self.amp_data.clear()

        self.reader = SerialReader(
            port,
            115200,
            self.data_queue,
            lambda e: messagebox.showerror("Serial error", e),
        )
        self.reader.start()

    def stop(self):
        if self.reader:
            self.reader.stop()

    def save_csv(self):
        if not self.data_log:
            messagebox.showwarning("No data", "Nothing to save")
            return

        path = filedialog.asksaveasfilename(
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv")]
        )
        if not path:
            return

        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["time_ms", "ac", "amp", "finger"])
            w.writerows(self.data_log)

    def update_plot(self):
        updated = False

        while not self.data_queue.empty():
            t, ac, amp, finger = self.data_queue.get()
            self.data_log.append((t, ac, amp, finger))

            self.ac_data.append(ac)
            self.amp_data.append(amp)

            if len(self.ac_data) > 500:
                self.ac_data.pop(0)
                self.amp_data.pop(0)

            updated = True

        if updated:
            self.line_ac.set_data(range(len(self.ac_data)), self.ac_data)
            self.ax1.set_xlim(0, len(self.ac_data))

            if len(self.ac_data) > 5:
                ymin = min(self.ac_data)
                ymax = max(self.ac_data)
                margin = max(2, int((ymax - ymin) * 0.15))
                self.ax1.set_ylim(ymin - margin, ymax + margin)

            self.line_amp.set_data(range(len(self.amp_data)), self.amp_data)
            self.ax2.set_xlim(0, len(self.amp_data))

            if len(self.amp_data) > 5:
                ymin = min(self.amp_data)
                ymax = max(self.amp_data)
                margin = max(1, int((ymax - ymin) * 0.15))
                self.ax2.set_ylim(ymin - margin, ymax + margin)

            self.canvas.draw()


        self.root.after(30, self.update_plot)

if __name__ == "__main__":
    root = tk.Tk()
    app = PPGApp(root)
    root.mainloop()
