"""
Author: Rafael Remo - The Gaitkeepers

TML PLATFORM APPLICATION: recorder classes
- This class handles the Recorder class
"""
from tkinter import *
from tkinter import filedialog
import csv
import time


class Recorder:
    def __init__(self, root):
        self.root = root

        self.recording_state = IntVar()  # Keeps state of check button for recording

        self.clear_data()

        self.fields = ("Main_Angle", "Main_Heel", "Main_Fore", "Aux_Angle", "Aux_Heel", "Aux_Fore")

        self.record_speed_divider = 1  # Sets recording rate
        self.counter = 0

    def clear_data(self):
        self.data = list()
        self.data_length = 0

    def try_record(self, **kwargs):  # but this before saving new values to backend
        if self.recording_state.get():
            if self.counter == self.record_speed_divider - 1:
                field_values = [0, 0, 0, 0, 0, 0]

                # Record data
                for i, field in enumerate(self.fields):
                    if field in kwargs:
                        field_values[i] = kwargs[field]
                    else:
                        if len(self.data) > 0:
                            field_values[i] = self.data[-1][i]
                        else:
                            field_values[i] = 0
                data = [time.strftime("%Y-%m-%d %H:%M:%S")]
                for field in field_values:
                    data.append(field)
                self.data.append(tuple(data))
                self.data_length += 1

            self.counter = (self.counter + 1) % self.record_speed_divider

    def save_data(self):
        if len(self.data) > 0 and self.recording_state.get() == 0:
            # Prompt user to select a file location
            file_path = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV files", "*.csv")])
            if file_path:
                # Write data to CSV file
                with open(file_path, "w", newline="") as csvfile:
                    csv_writer = csv.writer(csvfile)
                    header = ["Timestamp"]
                    for field in self.fields:
                        header.append(field)
                    csv_writer.writerow(header)
                    csv_writer.writerows(self.data)
