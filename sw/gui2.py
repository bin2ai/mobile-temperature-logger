import csv
import datetime
from enum import Enum
import sys
import time
import tkinter as tk
from tkinter import ttk
import serial.tools.list_ports
import serial
import threading


class DeviceState(Enum):
    IDLE = 0
    COLLECTING = 1
    PARTIAL_FULL = 2
    FULL = 3


device_state = DeviceState.IDLE

# label that updates based on the variable values
value_vbus = 0.0
value_vbat = 0.0
value_charging_l = False
value_standby_l = False
value_state = 0
value_interval = 1
value_index = 0
value_timestamp = 0
value_estimated_time_remaining = 0
# Create a thread-safe queue for serial commands
# serial_queue = queue.Queue()
serial_list: list = []

selected_port = ""  # Variable to hold the selected COM port
serial_thread = None  # Variable to hold the serial thread
data = []  # Variable to hold the data received from the serial port
data_size = 500

for i in range(data_size):
    data.append(0)

is_cmd_priority = False

status_update_seconds = 5
last_status_update = 0


def reset_data():
    global data
    data = []
    global data_size
    for i in range(data_size):
        data[i] = 0

# Function to list available COM ports


def list_available_ports():
    ports = serial.tools.list_ports.comports()
    port_list = [port.device for port in ports]
    return port_list

# Function to handle the selection from the dropdown menu


def on_select(event):
    selected_port = port_var.get()
    print(f"Selected Port: {selected_port}")


def add_status_cmd_to_queue():
    serial_list.append('STATUS?STATE\n')
    serial_list.append('STATUS?VBUS\n')
    serial_list.append('STATUS?VBAT\n')
    serial_list.append('STATUS?TEMP\n')
    serial_list.append('STATUS?CHARGING_L\n')
    serial_list.append('STATUS?STANDBY_L\n')
    serial_list.append('STATUS?INTERVAL\n')
    serial_list.append('STATUS?INDEX\n')
    serial_list.append('STATUS?UTC\n')


# Function to handle serial communication
def serial_worker():
    global selected_port, value_vbus, value_vbat, value_charging_l, value_standby_l, value_state, value_interval, value_index, value_timestamp, value_estimated_time_remaining, last_status_update
    global serial_list
    global data, data_size
    while True:
        try:
            try:
                command = ""
                # check list if in the list a command is available that is not a status command, then remove all status commands from the list and pop the first command
                is_status_only = True
                for cmd in serial_list:
                    if not cmd.startswith("STATUS?"):
                        is_status_only = False
                        break
                if not is_status_only:
                    for cmd in serial_list:
                        if cmd.startswith("STATUS?"):
                            serial_list.remove(cmd)
                # if the list is empty, then add a status command to the list
                if len(serial_list) == 0:
                    add_status_cmd_to_queue()
                command = serial_list.pop(0)
            except Exception as e:
                if time.time() - last_status_update > status_update_seconds:
                    add_status_cmd_to_queue()
                    last_status_update = time.time()
            if command == "":
                continue

            # if selected_port is "" then skip the rest of the loop
            if selected_port == "":
                continue
            # Open and configure the serial port
            with serial.Serial(selected_port, 115200, timeout=0.25) as ser:
                # Send the command over the serial port
                ser.write(command.encode())

                response = ""  # Initialize an empty response string

                # Read the response from the serial port until a newline character is received
                while True:
                    char = ser.read(1).decode(errors='ignore')
                    if char == '\n':
                        break  # Exit the loop when a newline character is received
                    response += char

                # Display the sent and received strings in the console
                print(f"Sent: {command}")
                print(f"Received: {response}")
                # update_console(f"Sent: {command}")
                # update_console(f"Received: {response}\n")
                # str

                # Parse and update labels based on the received response
                if command.startswith("STATUS?"):
                    if command == "STATUS?VBUS\n":
                        value_vbus = float(response)
                        label_vbus.config(
                            text=f"Vbus (5.0V nominal): {str(value_vbus)}V")
                    elif command == "STATUS?VBAT\n":
                        value_vbat = float(response)
                        label_vbat.config(
                            text=f"Vbat (4.2V full charge): {str(value_vbat)}V")
                        # set max to 4.2V
                        progress_vbat.config(maximum=4.2)
                        # set min to 3.0V
                        progress_vbat.config(value=value_vbat)
                    elif command == "STATUS?CHARGING_L\n":
                        value_charging_l = not bool(response)
                        label_charging_l.config(
                            text="Charging: " + str(value_charging_l))
                    elif command == "STATUS?STANDBY_L\n":
                        value_standby_l = not bool(response)
                        label_standby_l.config(
                            text="Standby: " + str(value_standby_l))
                    elif command == "STATUS?INTERVAL\n":
                        value_interval = int(response)
                        label_sampling_interval.config(
                            text="Interval: " + str(value_interval) + "s")
                        est_seconds = value_interval*(data_size-value_index)
                        # convert to HH:MM:SS
                        label_estimated_completion_time.config(
                            text="Estimated Time Remaining: " + str(datetime.timedelta(seconds=est_seconds)))
                        slider_sampling_interval.config(state=tk.NORMAL)
                        # update slider based on value
                        slider_sampling_interval.set(value_interval)
                    elif command == "STATUS?INDEX\n":
                        value_index = int(response)
                        label_index.config(
                            text=f"Index: {str(value_index)}/{str(data_size)}")
                    elif command == "STATUS?UTC\n":
                        print(response)
                        value_timestamp = int(response)
                        date_str = datetime.datetime.utcfromtimestamp(
                            value_timestamp).strftime('%Y-%m-%d %H:%M:%S')
                        label_timestamp.config(text="Timestamp: " + date_str)
                    elif command == "STATUS?STATE\n":
                        # set the state text to the device_state enum member that matches the value
                        state: str = DeviceState(int(response)).name
                        value_state = int(response)
                        label_state.config(text="State: " + state)
                        # use device_state enum
                        # if idle then set state_button text to start
                        if value_state == DeviceState.IDLE.value:
                            state_button.config(text="Start")
                            state_button.config(state=tk.NORMAL)
                            # save button enable
                            save_button.config(state=tk.NORMAL)
                        elif value_state == DeviceState.COLLECTING.value:
                            state_button.config(text="Stop")
                            state_button.config(state=tk.NORMAL)
                            # save button disable
                            save_button.config(state=tk.DISABLED)
                        elif value_state == DeviceState.PARTIAL_FULL.value or value_state == DeviceState.FULL.value:
                            state_button.config(text="Clear")
                            state_button.config(state=tk.NORMAL)
                            # save button enable
                            save_button.config(state=tk.NORMAL)
                        else:
                            state_button.config(text="Error")
                            state_button.config(state=tk.DISABLED)
                            # save button disable
                            save_button.config(state=tk.DISABLED)
                elif command.startswith("DUMP"):
                    # assume what follows is an index from 0-499, invalid values will be ignored
                    try:
                        command = command.replace("DUMP", "")
                        index = int(command)
                        # if the index is valid then update the data at that index
                        if index < 0 or index >= data_size:
                            raise ValueError
                        data[index] = float(response)
                        print(
                            f"Index: {index}\tData: {data[index]}")

                        print(f"Data[{index}] = {data[index]}")
                        # if the index is the last index then save the data to a csv file
                        if index == data_size - 1 or index == value_index:
                            print("Saving data to csv file...")
                            # if its the last index save as csv, where there are 3 columns; index, UTC + index * interval, data[index]
                            with open(f'data_{value_timestamp}.csv', 'w', newline='') as csvfile:
                                writer = csv.writer(csvfile, delimiter=',')
                                writer.writerow(['index', 'timestamp', 'data'])
                                # write the data to the csv file, from 0 to index
                                for i in range(value_index):
                                    writer.writerow(
                                        [i, value_timestamp + i * value_interval, data[i]])
                            print("Data saved to csv file.")
                    except Exception as e:
                        # print line where error occured
                        print("Error occured on line {}".format(
                            sys.exc_info()[-1].tb_lineno))
                        print(e)

                    # enable the button
                    state_button.config(state=tk.NORMAL)
                elif command.startswith("CLEAR"):
                    # enable the button
                    state_button.config(state=tk.NORMAL)
                    # clear the data
                    data = [0.0] * data_size
                elif command.startswith("START"):
                    # enable the button
                    state_button.config(state=tk.NORMAL)
                elif command.startswith("STOP"):
                    #   enable the button
                    state_button.config(state=tk.NORMAL)

        except Exception as e:
            # print line where error occured
            print("Error occured on line {}".format(
                sys.exc_info()[-1].tb_lineno))
            print(e)
# # Function to update the console in a thread-safe manner
# def update_console(text):
#     console.config(state=tk.NORMAL)  # Set the state to NORMAL to allow editing
#     console.insert(tk.END, text)
#     console.config(state=tk.DISABLED)  # Set the state back to DISABLED
#     console.see(tk.END)  # Autoscroll to the bottom of the console

# Function to send a serial command


def send_serial_command():
    command = command_entry.get()
    # if it doesnt end with a newline, add one
    if not command.endswith("\n"):
        command += "\n"
    # serial_queue.put(command)
    serial_list.append(command)


def update_state():
    # get current device state
    global value_state
    global value_interval
    # disable the button
    state_button.config(state=tk.DISABLED)
    # if idle then send start command
    if value_state == DeviceState.IDLE.value:
        utc_time = int(time.time())
        # serial_queue.put(f"UTC={utc_time}\n")
        # serial_queue.put(f"INTERVAL={value_interval}\n")
        # serial_queue.put("START\n")
        serial_list.append(f"UTC={utc_time}\n")
        serial_list.append(f"INTERVAL={value_interval}\n")
        serial_list.append("START\n")
    # if collecting then send stop command
    elif value_state == DeviceState.COLLECTING.value:
        # serial_queue.put("STOP\n")
        serial_list.append("STOP\n")
    # if partial or full then send clear command
    elif value_state == DeviceState.PARTIAL_FULL.value or value_state == DeviceState.FULL.value:
        # serial_queue.put("CLEAR\n")
        serial_list.append("CLEAR\n")
    # if error then do nothing
    else:
        pass

# Function to update the interval when the slider is released


def update_interval(event):
    # disable button
    global value_interval
    new_value = slider_sampling_interval.get()
    if new_value != value_interval:
        value_interval = new_value
        label_sampling_interval.config(
            text="Interval: " + str(value_interval) + "s")
        # send the interval command
        # serial_queue.put(f"INTERVAL={value_interval}\n")
        serial_list.append(f"INTERVAL={value_interval}\n")
    slider_sampling_interval.config(state=tk.DISABLED)


def save_data():
    # disable the button
    save_button.config(state=tk.DISABLED)
    # for 0 to 499 add "DUMP{index}?\n" to the queue
    for i in range(0, data_size):
        # serial_queue.put(f"DUMP{i}\n")
        serial_list.append(f"DUMP{i}\n")


# Function to start or restart the serial thread when a COM Port is selected
def update_serial_port(event):
    global selected_port
    selected_port = port_var.get()


serial_thread = threading.Thread(target=serial_worker)
serial_thread.daemon = True
serial_thread.start()


# Create the main GUI window
root = tk.Tk()
root.title("Serial Command Sender with Console")

# Label for COM Port selection
label_port = tk.Label(root, text="Select a COM Port:")
label_port.pack(pady=10)

# Dropdown menu to select COM port
ports = list_available_ports()
port_var = tk.StringVar(root)
port_var.set(ports[0])  # Set the default value
port_dropdown = ttk.Combobox(
    root, textvariable=port_var, values=ports, state="readonly")
port_dropdown.pack(pady=10)
port_dropdown.bind("<<ComboboxSelected>>", update_serial_port)


# label for vbus
label_vbus = tk.Label(root, text="Vbus: " + str(value_vbus))
label_vbus.pack(pady=10)
# label for vbat
label_vbat = tk.Label(root, text="Vbat: " + str(value_vbat))
label_vbat.pack(pady=10)
# add progress bar for vbat where 4.2v is 100% and 3.2v (or less) is 0%
progress_vbat = ttk.Progressbar(
    root, orient="horizontal", length=200, mode="determinate", maximum=4.2, value=value_vbat)
progress_vbat.pack(pady=10)
# label for charging
label_charging_l = tk.Label(root, text="Charging: " + str(value_charging_l))
label_charging_l.pack(pady=10)
# label for standby
label_standby_l = tk.Label(root, text="Standby: " + str(value_standby_l))
label_standby_l.pack(pady=10)

# INDEX
# label for index
label_index = tk.Label(root, text="Index: " + str(value_index))
label_index.pack(pady=10)
# label for timestamp, convert to human readable UTC (YYYY-MM-DD HH:MM:SS)
date_str = datetime.datetime.utcfromtimestamp(
    value_timestamp).strftime('%Y-%m-%d %H:%M:%S')
label_timestamp = tk.Label(root, text="Timestamp: " + date_str)
label_timestamp.pack(pady=10)

# label for state
label_state = tk.Label(root, text="State: " + str(value_state))
label_state.pack(pady=10)

# Label for sampling interval
label_sampling_interval = tk.Label(
    root, text="Interval: " + str(value_interval) + "s")
label_sampling_interval.pack(pady=10)
# disable the button

# Slider for sampling interval, only call command when slider is released
slider_sampling_interval = tk.Scale(
    root, from_=1, to=90, orient=tk.HORIZONTAL, length=200, showvalue=True)
slider_sampling_interval.pack(pady=10)

# Bind the ButtonRelease-1 event to the update_interval function
slider_sampling_interval.bind("<ButtonRelease-1>", update_interval)
# disable the slider
slider_sampling_interval.config(state=tk.DISABLED)

# label for estimated completion time (based on data_size x sampling interval seconds)
label_estimated_completion_time = tk.Label(
    root, text="Estimated Completion Time: " + str(value_estimated_time_remaining))
label_estimated_completion_time.pack(pady=10)

# Label for serial command entry
label_command = tk.Label(root, text="Enter a serial command:")
label_command.pack(pady=10)

# Entry widget to enter serial commands
command_entry = tk.Entry(root)
command_entry.pack(pady=10)

# # Button to send serial command
# send_button = tk.Button(root, text="Send Command", command=send_serial_command)
# send_button.pack(pady=10)

# multi-purpose button
# start off as button disabled, when enabled, it will be a "start" button
# when pressed, it will be a "stop" button
# when pressed again, it will be a "clear" button
# when pressed again, it will be a "start" button
state_button = tk.Button(root, text="Start", command=update_state)
state_button.config(state=tk.DISABLED)
state_button.pack(pady=10)

# save button
save_button = tk.Button(root, text="Save", command=save_data)
save_button.config(state=tk.DISABLED)
save_button.pack(pady=10)


# # Create a scrolled text widget for the serial console
# console = scrolledtext.ScrolledText(root, wrap=tk.WORD, height=15, state=tk.DISABLED)
# console.pack(pady=10)

# Start the Tkinter main loop
root.mainloop()
