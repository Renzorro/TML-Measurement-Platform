"""
Author: Rafael Remo - The Gaitkeepers

TML PLATFORM APPLICATION: GUI classes
- These classes handles all functions related to the GUI
"""

# Imports
from tkinter import *
from tkinter import ttk
from communication import Buffer, COM
import threading as th
from backend import Decoder, Backend
from recorder import Recorder


# GUI class
class GUI(Tk):
    # Important Values
    APP_TITLE = "TML Platform Desktop Application"
    BG_COLOR = "powder blue"  # default: grey94
    BUTTON_COLOR = "cadet blue"
    WIDTH = 560
    HEIGHT = 400
    UP_CURSOR = "sb_up_arrow"
    DOWN_CURSOR = "sb_down_arrow"
    STOP_CURSOR = "X_cursor"
    HOME_CURSOR = "bottom_side"
    AUTO_CURSOR = "top_side"
    TARE_CURSOR = "top_tee"

    def __init__(self):
        # Initialize Tk()
        Tk.__init__(self)

        # Configure window
        self.title(GUI.APP_TITLE)
        self.configure(bg=GUI.BG_COLOR)
        self.iconbitmap(default="icon.ico")

        # Set up data recorder
        self.recorder = Recorder(self)

        # Setup page
        self.page = HomePage(self)

        # Setup protocols
        self.protocol("WM_DELETE_WINDOW", self.__on_close)

        # Setup COM and Buffer
        self.buffer = Buffer()
        self.com = COM(self.buffer)

        # Setup GUI threads (for interpreting buffer)
        self.buffer_read_thread = th.Thread(target=self.buffer_read_loop, daemon=True)  # Thread ready to start

        # Setup backend for number processing
        self.backend = Backend(self)

    def buffer_read_loop(self):
        while self.com.serial_connected:
            data = self.buffer.receive_dequeue()
            data_char = data.decode(self.com.byte_decoding_method)

            response = self.backend.decoder.process_char(data_char)

            self.page.message_text.see(END)

    def serial_connection_change(self):  # Handles serial connecting and disconnecting
        if self.com.serial_connected:
            message = self.com.disconnect()
            if COM.disconnect_message in message:
                self.buffer.receive_clear()
                self.backend.decoder.clear()
                self.page.insert_command_text("")
                self.buffer_read_thread = th.Thread(target=self.buffer_read_loop,
                                                    daemon=True)  # Thread ready to start next time
                self.page.update_serial_button(self.com.serial_connected)
            self.page.insert_message(message)
        else:
            if self.com.find_comport() is None:
                self.page.insert_message("Couldn't find a valid COM port...")
                return
            connect_message = self.com.connect()
            message = f"Connecting to {self.com.get_port()}... {connect_message}"
            if COM.connected_message in message:
                self.buffer_read_thread.start()
                self.page.update_serial_button(self.com.serial_connected)
            self.page.insert_message(message)

    def __on_close(self):
        self.com.disconnect()
        self.destroy()  # Actually close the window


class HomePage(Frame):
    def __init__(self, root: GUI):
        Frame.__init__(self, root)
        self.root = root
        self.setup(self.root)  # Setup widgets
        self.name = "HomePage"

        # Values
        self.message_max_chars = 50

    def update_serial_button(self, connected):
        if connected:
            self.serial_button.config(text="Disconnect from serial")
        else:
            self.serial_button.config(text="Connect to serial")

    def insert_message(self, string):
        self.message_text.insert(END, "\n")  # Adds spacing before
        self.message_text.insert(END, string)
        self.message_text.insert(END, "\n")  # Adds spacing after
        self.message_text.see(END)

    def insert_command_text(self, string):
        self.command_text.delete('1.0', END)
        self.command_text.insert(END, string)

    def insert_queue_size_text(self, string):
        self.queue_size_text.delete('1.0', END)
        self.queue_size_text.insert(END, string)

    def clear_text(self, target_text: Text):
        target_text.delete('1.0', END)

    # Sets up widgets
    def setup(self, rt):
        # DEBUGGING placeholders
        DEBUG_reading = "0"

        # Grid parameters
        num_columns = 8
        # num_rows = 12

        # Styling
        style = ttk.Style()
        style.configure("title.TLabel", background=GUI.BG_COLOR)
        style.configure("label.TLabel", background=GUI.BG_COLOR)
        style.configure("number.TLabel", borderwidth=5, relief="sunken", padding=3, background="white", width=20)
        style.configure("button1.TButton", background=GUI.BUTTON_COLOR)

        title_font = ("Helvetica", 20, "bold")
        header_font = ("Helvetica", 14, "bold")
        label_font = ("Helvetica", 12)

        # Initialize Widgets
        self.title_label = ttk.Label(rt, text=f"{GUI.APP_TITLE}", font=title_font, style="title.TLabel")
        self.message_text = Text(rt, height=16, width=60)
        self.message_scroll = ttk.Scrollbar(rt, orient='vertical', command=self.message_text.yview)
        self.command_text = Text(rt, height=1, width=65)
        self.main_label = ttk.Label(rt, text="Main Platform", font=header_font, style="label.TLabel")
        self.angle_main_label = ttk.Label(rt, text=f"Angle: {DEBUG_reading} deg", font=label_font,
                                          style="number.TLabel")
        self.heel_main_label = ttk.Label(rt, text=f"Heel Force: {DEBUG_reading}N", font=label_font,
                                         style="number.TLabel")
        self.fore_main_label = ttk.Label(rt, text=f"Fore Force: {DEBUG_reading}N", font=label_font,
                                         style="number.TLabel")
        self.aux_label = ttk.Label(rt, text="Aux Platform", font=header_font, style="label.TLabel")
        self.angle_aux_label = ttk.Label(rt, text=f"Angle: {DEBUG_reading} deg", font=label_font, style="number.TLabel")
        self.heel_aux_label = ttk.Label(rt, text=f"Heel Force: {DEBUG_reading}N", font=label_font,
                                        style="number.TLabel")
        self.fore_aux_label = ttk.Label(rt, text=f"Fore Force: {DEBUG_reading}N", font=label_font,
                                        style="number.TLabel")
        self.main_up_button = ttk.Button(rt, text="Up", cursor=GUI.UP_CURSOR, takefocus=False, style="button1.TButton")
        self.main_home_button = ttk.Button(rt, text="Home", command=lambda: self.home_start("main"),
                                           cursor=GUI.HOME_CURSOR, takefocus=False, style="button1.TButton")
        self.main_down_button = ttk.Button(rt, text="Down", cursor=GUI.DOWN_CURSOR, takefocus=False,
                                           style="button1.TButton")
        self.main_stop_button = ttk.Button(rt, text="Stop", command=lambda: self.stop_moving("main"),
                                           cursor=GUI.STOP_CURSOR, takefocus=False, style="button1.TButton")
        self.up_label = ttk.Label(rt, text="Up")
        self.home_label = ttk.Label(rt, text="Home")
        self.down_label = ttk.Label(rt, text="Down")
        self.stop_both_button = ttk.Button(rt, text="Stop Both", command=lambda: self.stop_moving("both"),
                                           cursor=GUI.STOP_CURSOR, takefocus=False, style="button1.TButton")
        self.aux_up_button = ttk.Button(rt, text="Up", cursor=GUI.UP_CURSOR, takefocus=False, style="button1.TButton")
        self.aux_home_button = ttk.Button(rt, text="Home", command=lambda: self.home_start("aux"),
                                          cursor=GUI.HOME_CURSOR, takefocus=False, style="button1.TButton")
        self.aux_down_button = ttk.Button(rt, text="Down", cursor=GUI.DOWN_CURSOR, takefocus=False,
                                          style="button1.TButton")
        self.aux_stop_button = ttk.Button(rt, text="Stop", command=lambda: self.stop_moving("aux"),
                                          cursor=GUI.STOP_CURSOR, takefocus=False, style="button1.TButton")
        self.auto_button = ttk.Button(rt, text="Auto Both", command=lambda: self.auto_start("both"),
                                      cursor=GUI.AUTO_CURSOR, takefocus=False, style="button1.TButton")
        self.auto_main_button = ttk.Button(rt, text="Auto Main", command=lambda: self.auto_start("main"),
                                           cursor=GUI.AUTO_CURSOR, takefocus=False, style="button1.TButton")
        self.auto_aux_button = ttk.Button(rt, text="Auto Aux", command=lambda: self.auto_start("aux"),
                                          cursor=GUI.AUTO_CURSOR, takefocus=False, style="button1.TButton")
        self.serial_button = ttk.Button(rt, text="Connect to serial", command=rt.serial_connection_change, width=40,
                                        takefocus=False, style="button1.TButton")
        self.queue_size_text = Text(rt, height=1, width=17)
        self.tare_both_button = ttk.Button(rt, text="Tare Both", command=lambda: self.tare("both"),
                                           cursor=GUI.TARE_CURSOR, takefocus=False, style="button1.TButton")
        self.tare_main_button = ttk.Button(rt, text="Tare Main", command=lambda: self.tare("main"),
                                           cursor=GUI.TARE_CURSOR, takefocus=False, style="button1.TButton")
        self.tare_aux_button = ttk.Button(rt, text="Tare Aux", command=lambda: self.tare("aux"), cursor=GUI.TARE_CURSOR,
                                          takefocus=False, style="button1.TButton")
        self.actions_menubutton = ttk.Menubutton(rt, text="More Actions...", direction='above')
        self.actions_menubutton.menu = Menu(self.actions_menubutton, title="More Actions")
        self.actions_menubutton.menu.calibration = Menu(self.actions_menubutton.menu, title="Calibration")
        self.record_checkbutton = Checkbutton(rt, text="Record Data", variable=self.root.recorder.recording_state,
                                              bg=GUI.BG_COLOR)

        # I/O
        # self.input_entry.bind("<Return>", self.entry_to_serial)
        self.main_up_button.bind("<Button-1>", lambda event: self.move_button_press(event, "up", "main"))
        self.main_down_button.bind("<Button-1>", lambda event: self.move_button_press(event, "down", "main"))
        self.aux_up_button.bind("<Button-1>", lambda event: self.move_button_press(event, "up", "aux"))
        self.aux_down_button.bind("<Button-1>", lambda event: self.move_button_press(event, "down", "aux"))
        self.main_up_button.bind("<ButtonRelease-1>", lambda event: self.move_button_release(event, "main"))
        self.main_down_button.bind("<ButtonRelease-1>", lambda event: self.move_button_release(event, "main"))
        self.aux_up_button.bind("<ButtonRelease-1>", lambda event: self.move_button_release(event, "aux"))
        self.aux_down_button.bind("<ButtonRelease-1>", lambda event: self.move_button_release(event, "aux"))

        # Organize Widgets
        self.title_label.grid(column=0, row=0, columnspan=num_columns)
        self.message_text.grid(column=0, row=1, columnspan=3, rowspan=10, padx=5)
        self.message_scroll.grid(column=3, row=1, rowspan=10, sticky='ns')
        self.command_text.grid(column=4, row=1, columnspan=4, padx=5)
        self.main_label.grid(column=4, row=2, columnspan=2)
        self.angle_main_label.grid(column=4, row=3, columnspan=2)
        self.heel_main_label.grid(column=4, row=4, columnspan=2)
        self.fore_main_label.grid(column=4, row=5, columnspan=2)
        self.aux_label.grid(column=6, row=2, columnspan=2)
        self.angle_aux_label.grid(column=6, row=3, columnspan=2)
        self.heel_aux_label.grid(column=6, row=4, columnspan=2)
        self.fore_aux_label.grid(column=6, row=5, columnspan=2)
        self.main_up_button.grid(column=4, row=6, columnspan=2)
        self.main_home_button.grid(column=4, row=7, columnspan=2)
        self.main_down_button.grid(column=4, row=8, columnspan=2)
        self.main_stop_button.grid(column=4, row=9, columnspan=2)
        self.stop_both_button.grid(column=5, row=9, columnspan=2)
        self.aux_up_button.grid(column=6, row=6, columnspan=2)
        self.aux_home_button.grid(column=6, row=7, columnspan=2)
        self.aux_down_button.grid(column=6, row=8, columnspan=2)
        self.aux_stop_button.grid(column=6, row=9, columnspan=2)
        self.auto_button.grid(column=5, row=10, columnspan=2)
        self.auto_main_button.grid(column=4, row=10, columnspan=2)
        self.auto_aux_button.grid(column=6, row=10, columnspan=2)
        self.tare_both_button.grid(column=5, row=11, columnspan=2)
        self.tare_main_button.grid(column=4, row=11, columnspan=2)
        self.tare_aux_button.grid(column=6, row=11, columnspan=2)
        self.queue_size_text.grid(column=1, row=12)

        self.serial_button.grid(column=0, row=11, padx=5)
        self.record_checkbutton.grid(column=1, row=11)

        self.actions_menubutton.grid(column=2, row=11)
        self.actions_menubutton['menu'] = self.actions_menubutton.menu
        self.actions_menubutton.menu.add_cascade(label="Calibration", menu=self.actions_menubutton.menu.calibration)
        self.actions_menubutton.menu.calibration.add_command(label="Calibrate All", command=self.calibrate_both)
        self.actions_menubutton.menu.calibration.add_command(label="Calibrate Main: Heel",
                                                             command=lambda: self.calibrate('main', 'heel'))
        self.actions_menubutton.menu.calibration.add_command(label="Calibrate Main: Fore",
                                                             command=lambda: self.calibrate('main', 'fore'))
        self.actions_menubutton.menu.calibration.add_command(label="Calibrate Aux: Heel",
                                                             command=lambda: self.calibrate('aux', 'heel'))
        self.actions_menubutton.menu.calibration.add_command(label="Calibrate Aux: Fore",
                                                             command=lambda: self.calibrate('aux', 'fore'))
        self.actions_menubutton.menu.add_command(label="Clear Messages",
                                                 command=lambda: self.clear_text(self.message_text))
        self.actions_menubutton.menu.add_command(label="Save Data", command=self.save_data)
        self.actions_menubutton.menu.add_command(label="Clear Data", command=rt.recorder.clear_data)

    def foot_command_send(self, main: str, aux: str, foot: str):
        if foot == "main":
            self.root.com.serial_send(main)
        elif foot == "aux":
            self.root.com.serial_send(aux)
        elif foot == "both":
            self.root.com.serial_send(main)
            self.root.com.serial_send(aux)

    def save_data(self):
        self.root.recorder.save_data()

    # Button Commands
    def home_start(self, foot: str):
        main, aux = '6', '1'
        self.foot_command_send(main, aux, foot)

    def auto_start(self, foot: str):
        main, aux = '7', '2'
        self.foot_command_send(main, aux, foot)

    def stop_moving(self, foot: str):
        main, aux = '5', '0'
        self.foot_command_send(main, aux, foot)

    def move_up(self, foot: str):
        main, aux = '3', '8'
        self.foot_command_send(main, aux, foot)

    def move_down(self, foot: str):
        main, aux = '4', '9'
        self.foot_command_send(main, aux, foot)

    def calibrate_both(self):
        self.root.com.serial_send('!')

    def calibrate(self, foot: str, plate: str):
        """
        #define D2M_MAIN_HEEL_CALIBRATE 'w'
        #define D2M_MAIN_FORE_CALIBRATE 'v'
        #define D2M_AUX_HEEL_CALIBRATE 'u'
        #define D2M_AUX_FORE_CALIBRATE 't'
        """
        selection = {'main': {'heel': 'w', 'fore': 'v'}, 'aux': {'heel': 'u', 'fore': 't'}}
        self.root.com.serial_send(selection[foot][plate])

    def tare(self, foot: str):
        main, aux, both = 'y', 'x', 'z'
        if foot == "main":
            self.root.com.serial_send(main)
        elif foot == "aux":
            self.root.com.serial_send(aux)
        elif foot == "both":
            self.root.com.serial_send(both)

    # Key bind functions
    def entry_to_serial(self, event):
        text = self.input_entry.get()
        if text:
            if self.root.com.serial_connected:
                self.root.com.serial_send(text[0])
                self.input_entry.delete(0, END)

    def move_button_press(self, event, dir: str, foot: str):
        if dir == "up":
            self.move_up(foot)
        elif dir == "down":
            self.move_down(foot)

    def move_button_release(self, event, foot: str):
        self.stop_moving(foot)

    # DEBUGGING
    def DEBUG_filltext(self, event):  # What exactly does "event" do?
        text = self.input_entry.get()
        self.input_entry.delete(0, END)
        self.command_text.insert(END, text)

    def DEBUG_placeholder(self):
        pass
