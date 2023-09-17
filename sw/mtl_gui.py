import csv
import datetime
import os
import sys
import time
import tkinter as tk
from tkinter import ttk
import webbrowser
from matplotlib import pyplot as plt
import serial.tools.list_ports as list_ports
import serial
from tkinter import messagebox


class SerialConnectApp:

    root: tk.Tk
    serial_port: serial.Serial
    port_label: ttk.Label
    port_combo: ttk.Combobox
    connect_button: ttk.Button
    status_label: ttk.Label
    timer_interval: int
    timer: tk.Tk
    timer_label: ttk.Label
    timer_button: ttk.Button
    timer_running: bool
    delay_label: ttk.Label
    delay_slider: ttk.Scale
    period_label: ttk.Label
    period_slider: ttk.Scale
    samples_label: ttk.Label
    reset_button: ttk.Button

    is_connected: bool = False
    timer_interval_device_state = 5000

    device_states: dict = {
        0: "IDLE",
        1: "WAITING",
        2: "COLLECTING",
        3: "FULL",
        4: "UNKNOWN"

    }

    device_state: int = 0

    def __init__(self, root):
        self.root = root
        self.root.title("Mobile Temperature Logger (MTL)")

        # a ? button that opens up a url to the github page
        self.help_button = ttk.Button(
            root, text="?", command=self.open_github)
        self.help_button.pack(padx=10, pady=5)
        self.serial_port = None

        self.port_label = ttk.Label(root, text="Select COM Port:")
        self.port_label.pack(padx=10, pady=5)

        self.port_combo = ttk.Combobox(root, state="readonly")
        self.port_combo.pack(padx=10, pady=5)

        # Bind the refresh_ports function to the <FocusIn> event of the combobox
        self.port_combo.bind("<FocusIn>", self.refresh_ports)

        self.connect_button = ttk.Button(
            root, text="Connect", command=self.toggle_connection)
        self.connect_button.pack(padx=10, pady=5)
        # Initially disable the button
        self.connect_button["state"] = "disabled"

        self.status_label = ttk.Label(root, text="Status: Not Connected")
        self.status_label.pack(padx=10, pady=5)

        # horizontal line
        ttk.Separator(root, orient=tk.HORIZONTAL).pack(fill=tk.X)

        # settings text
        self.settings_label = ttk.Label(root, text="Settings:", font="bold")
        self.settings_label.pack(padx=10, pady=5)

        # text 'there are 1000 samples, so a period of 1000 seconds is ~16 minutes'
        self.samples_label = ttk.Label(
            root, text="There are 1000 samples,\nso a period of 1000 seconds is ~16 minutes")
        self.samples_label.pack(padx=10, pady=5)

        # i want sliders that set the delay seconds (0-255), period (1-65535), and samples (1-30)
        # i want a row where text, then slider, not text for one row, slider for next row
        self.delay_label = ttk.Label(root, text="Delay (seconds):")
        self.delay_label.pack(padx=10, pady=5)
        self.delay_slider = ttk.Scale(
            root, from_=0, to=255, orient=tk.HORIZONTAL, command=self.set_button_function)
        self.delay_slider.pack(padx=10, pady=5)
        self.delay_slider.set(60)
        self.delay_slider.bind("<ButtonPress-1>", self.on_slider_drag_start)
        self.delay_slider.bind("<ButtonRelease-1>", self.on_slider_drag_stop)

        self.period_label = ttk.Label(root, text="Period (seconds):")
        self.period_label.pack(padx=10, pady=5)
        self.period_slider = ttk.Scale(
            root, from_=1, to=100, orient=tk.HORIZONTAL, command=self.set_button_function)
        self.period_slider.pack(padx=10, pady=5)
        self.period_slider.set(60)
        self.period_slider.bind("<ButtonPress-1>", self.on_slider_drag_start)
        # estimated time to finish text (period * 1000 + delay in seconds)
        self.period_slider.bind("<ButtonRelease-1>", self.on_slider_drag_stop)
        # show hrs,min, seconds
        self.est_time_label = ttk.Label(root, text="Estimated Time:")
        self.est_time_label.pack(padx=10, pady=5)

        # set button and reset button on the same row
        self.reset_button = ttk.Button(
            root, text="Reset", command=self.reset_button_function)
        self.reset_button.pack(padx=10, pady=5)

        # horizontal line
        ttk.Separator(root, orient=tk.HORIZONTAL).pack(fill=tk.X)
        # battery voltage text
        self.battery_title = ttk.Label(
            root, text="Battery Voltage:", font="bold")
        self.battery_title.pack(padx=10, pady=5)
        self.battery_label = ttk.Label(
            root, text="Battery Voltage:")
        self.battery_label.pack(padx=10, pady=5)
        # battery info (chrg + stdby)
        self.battery_info_label = ttk.Label(
            root, text="Battery Info:")
        self.battery_info_label.pack(padx=10, pady=5)
        # progress bar (where 4.2 is full and 3.2 is empty)
        self.battery_progress = ttk.Progressbar(
            root, orient=tk.HORIZONTAL, length=200, mode="determinate", maximum=4.2, value=0)
        
        self.battery_progress.pack(padx=10, pady=5)

        # text for estimated time

        # horizontal line
        ttk.Separator(root, orient=tk.HORIZONTAL).pack(fill=tk.X)

        # Telemetry text
        self.telemetry_label = ttk.Label(root, text="Telemetry:", font="bold")
        # state text (0-idle, 1-waiting, 2-collecting, 4-full)
        self.state_title = ttk.Label(root, text="State:", font="bold")
        self.state_title.pack(padx=10, pady=5)
        self.state_label = ttk.Label(root, text="")
        self.state_label.pack(padx=10, pady=5)

        # start button
        self.start_button = ttk.Button(
            root, text="Start", command=self.start_button_function)
        self.start_button.pack(padx=10, pady=5)

        #progress bar (where 1000 is full and 0 is empty)
        self.progress_collection = ttk.Progressbar(
            root, orient=tk.HORIZONTAL, length=200, mode="determinate", maximum=1000, value=0)
        self.progress_collection.pack(padx=10, pady=5)

        #estimated time left text
        self.est_time_left_label = ttk.Label(root, text="Estimated Time Left:")
        self.est_time_left_label.pack(padx=10, pady=5)

        # divider
        ttk.Separator(root, orient=tk.HORIZONTAL).pack(fill=tk.X)

        # download data button
        self.download_button = ttk.Button(
            root, text="Download Data", command=self.download_button_function)
        self.download_button.pack(padx=10, pady=5)

        # Set the timer interval in milliseconds (e.g., check every 1000 ms)
        self.timer_interval = 1000  # 1 second

        self.update_device_state()
        self.check_and_refresh()

        # Start the timer to periodically check if connected and refresh COM ports
        self.root.after(self.timer_interval, self.check_and_refresh)

        self.root.after(5000, self.update_device_state)

    def open_github(self):
        webbrowser.open("https://github.com/bin2ai/mobile-temperature-logger")

    def start_button_function(self):
        self.start_button["state"] = "disabled"

        # if this button text says start, then send the start command ("AT+START\r\n")
        if self.start_button["text"] == "Start":
            try:
                # get utc time is seconds
                utc_time = int(time.time())
                # send +AT_SYNC_UTC=utc_time
                self.serial_port.write(f'AT+SYNC_UTC={utc_time}\r\n'.encode())
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error syncing time: {response}")

                self.serial_port.write(b'AT+START\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error starting: {response}")
                self.start_button["text"] = "Stop"
            except Exception as e:
                messagebox.showerror(
                    title="Error", message=f"Error starting: {e}")
        # if this button text says stop, then send the stop command ("AT+STOP\r\n")
        elif self.start_button["text"] == "Stop":
            try:
                self.serial_port.write(b'AT+STOP\r\n')
                response = self.serial_port.readline().decode()
                print(f"response: {response}")
                if "OK" not in response:
                    raise Exception(f"Error stopping: {response}")
                self.start_button["text"] = "Start"
            except Exception as e:
                messagebox.showerror(
                    title="Error", message=f"Error stopping: {e}")
        elif self.start_button["text"] == "Clear":
            try:
                self.serial_port.write(b'AT+CLEAR\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error resetting: {response}")
                self.start_button["text"] = "Start"
            except Exception as e:
                messagebox.showerror(
                    title="Error", message=f"Error resetting: {e}")
        else:
            messagebox.showerror(
                title="Error", message=f"Unknown button text: {self.start_button['text']}")

        if self.device_state == 0:
            self.start_button["text"] = "Start"
        elif self.device_state == 1:
            self.start_button["text"] = "Stop"
        elif self.device_state == 2:
            self.start_button["text"] = "Stop"
        elif self.device_state == 3:
            self.start_button["text"] = "Clear"
        else:
            self.start_button["text"] = "Unknown"
            self.start_button["state"] = "disabled"

    def download_button_function(self):
        if self.is_connected:
            # send AT+INDEX#\r\n
            try:
                # AT+GET_UTC?\r\n
                self.serial_port.write(b'AT+GET_UTC?\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error getting utc: {response}")
                # assume response is "OK+GET_UTC=[value]\r\n"
                utc = int(response.split("=")[1].split("\r")[0])
                # convert to string "temperature_logger_YYYY-MM-DD_HH-MM-SS.csv"
                filename = "temperature_logger_" + datetime.datetime.utcfromtimestamp(
                    utc).strftime("%Y-%m-%d_%H-%M-%S") + ".csv"

                self.serial_port.write(b'AT+TEMP#\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error getting index: {response}")
                # assume response is "OK+INDEX=[0-999]+\r\n"
                index = int(response.split("=")[1].split("\r")[0])

                if index == 0:
                    messagebox.showinfo(
                        title="Info", message=f"No data to download")

                data_bit = []
                for i in range(index):
                    data_bit.append(0)
                for i in range(index):
                    # send "AT+TELM@[i]\r\n"
                    self.serial_port.write(f'AT+TEMP@{i}\r\n'.encode())
                    response = self.serial_port.readline().decode()
                    try:
                        data_bit[i] = int(response.split("=")
                                          [1].split("\r")[0])
                    except:
                        pass

                # OPEN UP TEMP TO BIT MAP
                ''' temp_map.csv
                T_CAL,R_typ,R_min,R_max,V_typ,V_min,V_max,bit_typ,bit_min,bit_max,err_n,err_p,I max (mA),P max (mW),%-,%+
                -100,111312560,110199434,112425685,4.83,4.78,4.88,988,987,988,1,0,0.000,0.000,0.1%,0.0%
                -99,104938023,103888642,105987403,4.83,4.78,4.88,988,987,988,1,0,0.000,0.000,0.1%,0.0%
                -98,98960803,97971195,99950411,4.83,4.78,4.88,988,987,988,1,0,0.000,0.000,0.1%,0.0%
                '''

                '''
                temp_data = {
                "0": {
                }
                '''

                temp_map_data = {

                }
                # go through each data value, find the closest bit_typ, and convert to temp
                with open('temperature_bit_map.csv', newline='') as csvfile:
                    csvreader = csv.reader(csvfile)
                    # read header
                    header = next(csvreader)
                    index_tcal = header.index("T_CAL")
                    index_bit_typ = header.index("bit_typ")
                    for row in csvreader:
                        temp_map_data[row[index_tcal]] = row[index_bit_typ]

                # data_bit has the bit value for each index (time), convert to temp using temp_map_data
                data_temp = []
                for i in range(len(data_bit)):
                    data_temp.append(0)

                # find the closest bit for each data_bit, then map to temp
                for i in range(len(data_bit)):
                    bit = data_bit[i]
                    for j, temperature in enumerate(temp_map_data.keys()):
                        if j == 0:
                            closest_temperature = temperature
                        else:
                            if abs(int(temp_map_data[temperature]) - bit) < abs(int(temp_map_data[closest_temperature]) - bit):
                                closest_temperature = temperature
                    data_temp[i] = int(closest_temperature)

                # sort data_temp
                data_temp.sort()

                date_times = []
                for i in range(len(data_temp)):
                    date_times.append(0)

                # convert utc to date/time
                for i in range(len(data_temp)):
                    date_times[i] = datetime.datetime.utcfromtimestamp(
                        utc + i).strftime("%Y-%m-%d %H:%M:%S")

                # if this file already exists, append to it _1, _2, etc.
                # do not do _1_2, etc.
                if os.path.exists(filename):
                    i = 1
                    while os.path.exists(filename.split(".csv")[0] + "_" + str(i) + ".csv"):
                        i += 1
                    filename = filename.split(
                        ".csv")[0] + "_" + str(i) + ".csv"

                # write data to file
                with open(filename, 'w', newline='') as csvfile:
                    csvwriter = csv.writer(csvfile)
                    header = ["Date/Time", "Temperature (F)", "Bit Value"]
                    csvwriter.writerow(header)
                    for i in range(index):

                        csvwriter.writerow(
                            [date_times[i], data_temp[i], data_bit[i]])

                # create graph using matplotlib
                # make large
                x_size = 5
                y_size = 5
                plt.figure(figsize=(x_size, y_size))
                plt.plot(data_temp)
                plt.ylabel('Temperature (F)')
                plt.xlabel('Time (s)')
                # add title
                plt.title(filename.split(".csv")[0])
                min_ylim, max_ylim = plt.ylim()
                plt.ylim(min_ylim - 1, max_ylim + 1)
                min_val = min(data_temp)
                max_val = max(data_temp)
                # dashed not solid, 50% transparency
                plt.axhline(y=min_val, color='grey', linestyle='--', alpha=0.5)
                plt.axhline(y=max_val, color='grey', linestyle='--', alpha=0.5)
                avg_val = sum(data_temp) / len(data_temp)
                plt.axhline(y=avg_val, color='grey', linestyle='--', alpha=0.5)
                # add text for  date time, duration

                start_time = date_times[0]
                end_time = date_times[-1]
                # add text for  date time, duration, middle right
                plt.text(0.45, 0.15, f"Start: {start_time}\nEnd: {end_time}\nDuration (s): {len(data_temp)}\nMin (F): {min_val}\nMax (F): {max_val}\nAvg (F): {avg_val:.2f}",
                         horizontalalignment='left',
                         verticalalignment='center',
                         transform=plt.gca().transAxes, alpha=0.75)

                plt.savefig(filename.split(".csv")[0] + ".png")
                plt.clf()

                messagebox.showinfo(
                    title="Info", message=f"Data downloaded to {filename}")

            except Exception as e:
                messagebox.showerror(
                    title="Error", message=f"Error getting index: {e}")
                # print line error was on
                print("Error on line {}".format(sys.exc_info()[-1].tb_lineno))
                return

    def update_device_state(self):
        # send serial "AT+STATE?\r\n"
        if self.is_connected:
            try:
                self.serial_port.write(b'AT+STATE?\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error getting state: {response}")
                # assume response is "OK+STATE=[0-3]\r\n"
                state = int(response.split("=")[1].split("\r")[0])
                # look up state in dictionary
                self.device_state = state
                self.state_label["text"] = f"{self.device_states[self.device_state]}"

                # get battery voltage
                self.serial_port.write(b'AT+VBAT\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(
                        f"Error getting battery voltage: {response}")
                # assume response is "OK+VBAT=[0-3]\r\n"
                vbat_bit = float(response.split("=")[1].split("\r")[0])
                vbat_voltage = float(vbat_bit/1023*(10000+75000)/(10000)*3.3)
                # print vbat_bit, vbat_voltage

                self.battery_label["text"] = f"{vbat_voltage:.2f}V"
                # update progress bar
                self.battery_progress["value"] = vbat_voltage
                                

                # get AT+CHRG
                self.serial_port.write(b'AT+CHRG\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(
                        f"Error getting battery voltage: {response}")
                # assume response is "OK+CHRG=[0-1]\r\n"
                chrg_bit = int(response.split("=")[1].split("\r")[0])
                battery_info = "Info: "
                if chrg_bit == 0:
                    battery_info = "Charging"
                elif chrg_bit == 1:
                    battery_info = "Not Charging"

                # get AT+STDBY
                self.serial_port.write(b'AT+STBY\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(
                        f"Error getting battery voltage: {response}")
                # assume response is "OK+STDBY=[0-1]\r\n"
                stdby_bit = int(response.split("=")[1].split("\r")[0])
                if stdby_bit == 0:
                    battery_info += ", in standby"
                elif stdby_bit == 1:
                    battery_info += ", not in standby"

                self.battery_info_label["text"] = battery_info
                # Update the start button text based on the current state

                #get index. AT+INDEX#
                self.serial_port.write(b'AT+TEMP#\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error getting index: {response}")
                # assume response is "OK+INDEX=[0-999]\r\n"
                index = int(response.split("=")[1].split("\r")[0])
                #update progress_collection
                self.progress_collection["value"] = index
                self.progress_collection["maximum"] = 999
                self.progress_collection["length"] = 200
                self.progress_collection["mode"] = "determinate"
                self.progress_collection["orient"] = "horizontal"

                #get the period and multiply by 999-index to get the time remaining
                #est_time_left_label
                self.serial_port.write(b'AT+PERIOD?\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error getting index: {response}")
                # assume response is "OK+INDEX=[0-999]\r\n"
                period = int(response.split("=")[1].split("\r")[0])
                #update est_time_left_label
                est_time_left = (1000-index)*period
                #HH:MM:SS
                self.est_time_left_label["text"] = f"Estimated time left: {est_time_left//3600} Hr(s):{(est_time_left%3600)//60} Min(s):{est_time_left%60} Sec(s)"





            except Exception as e:
                messagebox.showerror(
                    title="Error", message=f"Error getting state: {e}")
                self.device_state = 4  # unknown state
                self.state_label["text"] = f"{self.device_states[self.device_state]}"
        else:
            self.device_state = 4  # unknown state
            self.state_label["text"] = f"{self.device_states[self.device_state]}"

        # update the start button text
        if self.device_state == 0:
            self.start_button["text"] = "Start"
        elif self.device_state in [1, 2]:
            self.start_button["text"] = "Stop"
        elif self.device_state == 3:
            self.start_button["text"] = "Clear"
        else:
            self.start_button["text"] = "Unknown"

        # if state is unknown disable start and download buttons
        if self.device_state == 4:
            self.start_button["state"] = "disabled"
            self.download_button["state"] = "disabled"
        else:
            # enable start button
            self.start_button["state"] = "normal"

        # Re-schedule the timer to check the device state again
        self.root.after(self.timer_interval_device_state,
                        self.update_device_state)

    def on_slider_drag_start(self, event):
        self.slider_dragging = True

    def on_slider_drag_stop(self, event):
        self.slider_dragging = False
        self.set_button_function()

    def set_button_function(self, _=None):
        if not self.is_connected:
            return
        if not self.slider_dragging:
            # This function will be called when the slider is released
            # send serial command to device
            try:
                self.serial_port.write(
                    b'AT+DELAY=' + str(self.delay_slider.get()).encode() + b'\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error setting delay: {response}")
                # split = and use the second part as delay
                delay = response.split("=")[1].split("\r")[0]
                # update delay label text
                self.delay_label["text"] = f"Delay: {delay} seconds"

                self.serial_port.write(
                    b'AT+PERIOD=' + str(self.period_slider.get()).encode() + b'\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error setting period: {response}")

                period = response.split("=")[1].split("\r")[0]

                self.period_label["text"] = f"Period: {period} seconds"

                self.serial_port.write(
                    b'AT+SMPLS=30\r\n')
                response = self.serial_port.readline().decode()
                if "OK" not in response:
                    raise Exception(f"Error setting samples: {response}")
            except Exception as e:
                # display pop up
                messagebox.showerror(
                    "Error", f"Error sending settings to device. Please try again.\n\n{e}")

            est_time = 1000*(int(period)) + int(delay)

            self.est_time_label["text"] = f"Estimated time\n(HH:MM:SS): {est_time//3600} Hr(s):{(est_time%3600)//60} min(s):{est_time%60} sec(s)"

    def update_state(self):
        if not self.is_connected:
            # disable start, set, and reset buttons and slides
            self.start_button["state"] = "disabled"
            self.reset_button["state"] = "disabled"
            self.delay_slider["state"] = "disabled"
            self.period_slider["state"] = "disabled"
            self.download_button["state"] = "disabled"
        else:
            # enable start, set, and reset buttons and slides
            self.start_button["state"] = "normal"
            self.reset_button["state"] = "normal"
            self.delay_slider["state"] = "normal"
            self.period_slider["state"] = "normal"
            self.download_button["state"] = "normal"

            self.state_label["text"] = f"{self.device_states[self.device_state]}"
            # if device is running, stopped, or cleared, only allow the user to edit in idle state
            if self.device_state in [1, 2, 3, 4]:
                self.reset_button["state"] = "disabled"
                self.delay_slider["state"] = "disabled"
                self.period_slider["state"] = "disabled"

            if self.device_state == 1:
                self.start_button["text"] = "Start"
            elif self.device_state == 2:
                self.start_button["text"] = "Stop"
            elif self.device_state == 3:
                self.start_button["text"] = "Clear"
            elif self.device_state == 4:
                self.start_button["text"] = "Unknown"
                self.start_button["state"] = "disabled"
                self.download_button["state"] = "disabled"

    def reset_button_function(self):
        # make sliders go back to their default values
        self.delay_slider.set(60)
        self.period_slider.set(60)

    def refresh_ports(self, event=None):
        # Get a list of available COM ports and update the dropdown
        available_ports = [port.device for port in list_ports.comports()]
        self.port_combo["values"] = available_ports

    def check_and_refresh(self):
        # Check if the serial port is connected
        if self.serial_port and self.serial_port.is_open:
            self.status_label["text"] = "Status: Connected"
            self.connect_button["text"] = "Disconnect"
            # Enable the button for disconnecting
            self.connect_button["state"] = "normal"
        else:
            self.status_label["text"] = "Status: Not Connected"
            self.connect_button["text"] = "Connect"
            # Enable the button for connecting
            self.connect_button["state"] = "normal"

        # Check if the selected COM port is still available
        selected_port = self.port_combo.get()
        available_ports = [port.device for port in list_ports.comports()]
        if selected_port not in available_ports:
            self.disconnect_serial()

        # Update the COM port list
        self.refresh_ports()

        # Re-schedule the timer to check again after the timer_interval
        self.root.after(self.timer_interval, self.check_and_refresh)

        self.update_state()

    def toggle_connection(self):
        if self.serial_port and self.serial_port.is_open:
            self.disconnect_serial()
        else:
            self.connect_serial()

    def connect_serial(self):
        selected_port = self.port_combo.get()

        if not selected_port:
            self.is_connected = False
            messagebox.showerror("Error", "Please select a COM port.")
            self.update_state()
            return

        try:
            # Try to open the selected COM port
            self.serial_port = serial.Serial(
                selected_port, baudrate=9600, timeout=1)
            self.is_connected = True
            messagebox.showinfo("Success", f"Connected to {selected_port}")
            self.check_and_refresh()  # Update status and refresh after successful connection
        except serial.SerialException as e:
            self.is_connected = False
            # Handle any exceptions that may occur
            messagebox.showerror("Error", f"Failed to connect: {str(e)}")
            self.check_and_refresh()  # Update status and refresh after failed connection

        self.update_state()
        self.update_device_state()

    def disconnect_serial(self):
        if self.serial_port and self.serial_port.is_open:
            self.is_connected = False
            self.serial_port.close()
            self.check_and_refresh()  # Update status and refresh after disconnection


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialConnectApp(root)
    root.mainloop()
