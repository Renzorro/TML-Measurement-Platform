"""
Author: Rafael Remo - The Gaitkeepers

TML PLATFORM APPLICATION: Backend classes
- This class handles:
    - the decoding of the receiving buffer
    - backend variable storage
    - number processing
    - messing around with numbers in the backend

The Decoder class, in one function (ideally)...
- Takes in a byte
- recognizes the byte and where in the stream/command it is
- Returns necessary data (or inputs it into the GUI backend)
"""

# Imports
from communication import COM


class Decoder:
    """
    Stream architecture: 0, 1, 2, 3, 4, 5, 6
        {stream_start} {main angle} {main heel} {main fore} {aux angle} {aux heel} {aux fore}
    Command Architecture: 0, ..., len(command)+1
        {command_start} {... command or message} {command_end}
    """

    NAN = '-'

    def __init__(self, bke):
        self.bke = bke

        # Parameters
        self.stream_max = 6  # 6 numbers per stream

        # Initialize device state variables
        self.entire_device_stopped()

        # Initialize communication state variables
        self.clear()

    def entire_device_stopped(self):
        self.main_moving_up = False
        self.main_moving_down = False
        self.aux_moving_up = False
        self.aux_moving_down = False
        self.main_moving_to_home = False
        self.aux_moving_to_home = False
        self.main_doing_auto = False
        self.aux_doing_auto = False
        self.both_calibrating = False

    def clear(self):
        # State machines
        self.stream_state = 0
        self.command_state = 0
        self.number_state = 0  # 0 = left of decimal, 1 = right of decimal
        self.decimal_state = 0

        # State buffers
        self.command_buffer = []
        self.int_buffer = []  # contains strings
        self.decimal_buffer = []  # contains strings

        # Buffer size indicator
        self.page_insert_queue_size("")

    def process_char(self, char: str):
        # Check for command stuff
        if char == COM.command_start:
            self.command_state = 1
            self.command_buffer = []  # clear buffer
            return None
        if char == COM.command_end:
            if len(self.command_buffer) > 2:  # Messages have to be larger than 2 characters!!!
                self.command_state = 0
                self.page_insert_message(''.join(self.command_buffer))
                self.page_insert_queue_size(str(self.bke.root.buffer.receive_buffer.qsize()))
                return None
            else:
                self.command_state = 0
                self.page_insert_queue_size(str(self.bke.root.buffer.receive_buffer.qsize()))
                return self._command(''.join(self.command_buffer))  # Commands should only be int values: '{int}'
        if self.command_state > 0:
            self.command_state += 1
            self.command_buffer.append(char)
            return None

        # Check for stream stuff
        if char == COM.stream_start:
            self.stream_state = 1  # start recording first number
            self.int_buffer = []
            self.decimal_buffer = []
            return
        if self.stream_state > 0:
            # Get digits
            if char == COM.separator:  # Assumes every number in the stream is followed by a separator, even the last one
                # Get number
                try:
                    big = float(''.join(self.int_buffer))
                    small = float(''.join(self.decimal_buffer)) / 10 ** len(self.decimal_buffer)
                    if self.int_buffer[0] == '-':  # Check for negative number
                        num = big - small
                    else:
                        num = big + small
                except Exception:
                    num = Decoder.NAN
                # Assign number to variable in bke (backend)
                if self.stream_state == 1:
                    self.bke.main_angle = num
                elif self.stream_state == 2:
                    self.bke.main_heel = num
                elif self.stream_state == 3:
                    self.bke.main_fore = num
                elif self.stream_state == 4:
                    self.bke.aux_angle = num
                elif self.stream_state == 5:
                    self.bke.aux_heel = num
                elif self.stream_state == 6:
                    self.bke.aux_fore = num
                    self.bke.root.recorder.try_record(
                        Main_Angle=self.bke.main_angle,
                        Main_Heel=self.bke.main_heel,
                        Main_Fore=self.bke.main_fore,
                        Aux_Angle=self.bke.aux_angle,
                        Aux_Heel=self.bke.aux_heel,
                        Aux_Fore=self.bke.aux_fore
                    )  # Record data
                    self.bke.root.page.insert_queue_size_text(f"Data points: {self.bke.root.recorder.data_length}")
                    self.bke.update_display()  # Update display
                    # self.page_insert_queue_size(str(self.bke.root.buffer.receive_buffer.qsize())) # this is for checking if buffer size is too big
                self.stream_state = (self.stream_state + 1) % (self.stream_max + 1)
                self.number_state = 0
                self.int_buffer = []
                self.decimal_buffer = []
                return  # return something to tell GUI thread that a number is completed/saved (now the display should update)
            if char == '.':
                self.number_state = 1
                return
            if self.number_state == 0:
                self.int_buffer.append(char)
                return
            elif self.number_state == 1:
                self.decimal_buffer.append(char)

    def _command(self, command: str):
        """
        Command List:
        0: Echo (for testing)
        1: Enable Desktop Control
        2: Disable Desktop Control
        3: Main moving up
        4: Main moving down
        5: Main stopping
        6: Main in home-ing procedure
        7: Main in auto procedure
        8: Aux moving up
        9: Aux moving down
        10: Aux Stopping
        11: Aux in home-ing procedure
        12: Aux in auto procedure
        13: Both in home-ing procedure
        14: Both in auto procedure
        15: Both calibrating
        """
        commands = [
            self._echo,  # 0
            self.enable_control,  # 1
            self.disable_control,  # 2 ...
            self.main_up,
            self.main_down,
            self.main_stop,
            self.main_home,
            self.main_auto,
            self.aux_up,
            self.aux_down,
            self.aux_stop,
            self.aux_home,
            self.aux_auto,
            self.both_home,
            self.both_auto,
            self.both_calibrate,
            self.both_calibrate_done
        ]
        command_dict = {str(index): command for index, command in enumerate(commands)}
        to_run = command_dict.get(command, self._no_command)
        return to_run()

    def generate_command_text(self) -> str:
        if self.both_calibrating:
            return "Calibrating..."

        if self.main_moving_to_home:
            main_message = "Main moving to home"
        elif self.main_doing_auto:
            main_message = "Main performing auto procedure"
        elif self.main_moving_up:
            main_message = "Main moving up"
        elif self.main_moving_down:
            main_message = "Main moving down"
        else:
            main_message = "Main platform stopped"

        if self.aux_moving_to_home:
            aux_message = "Aux moving to home"
        elif self.aux_doing_auto:
            aux_message = "Aux performing auto procedure"
        elif self.aux_moving_up:
            aux_message = "Aux moving up"
        elif self.aux_moving_down:
            aux_message = "Aux moving down"
        else:
            aux_message = "Aux platform stopped"

        return f"{main_message} | {aux_message}"

    def _no_command(self):  # a "do nothing" function
        pass

    def page_insert_message(self, string):
        self.bke.root.page.insert_message(string)

    def page_insert_command(self, string):
        self.bke.root.page.insert_command_text(string)

    def page_insert_queue_size(self, string):
        self.bke.root.page.insert_queue_size_text(string)

    def _echo(self):  # Use this to debug MCU communication with desktop
        self.page_insert_command("Echo...")

    def enable_control(self):
        # self.page_insert_command("Desktop Control Enabled!")
        self.bke.root.com.lockout = False

    def disable_control(self):
        # self.page_insert_command("Desktop control is disabled :(")
        self.bke.root.com.lockout = True

    def main_up(self):
        self.main_moving_up = True
        self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def main_down(self):
        self.main_moving_up = False
        self.main_moving_down = True
        self.main_moving_to_home = False
        self.main_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def main_stop(self):
        self.main_moving_up = False
        self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def main_home(self):
        # self.main_moving_up = False
        # self.main_moving_down = False
        self.main_moving_to_home = True
        self.main_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def main_auto(self):
        # self.main_moving_up = False
        # self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = True
        self.page_insert_command(self.generate_command_text())

    def aux_up(self):
        self.aux_moving_up = True
        self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def aux_down(self):
        self.aux_moving_up = False
        self.aux_moving_down = True
        self.aux_moving_to_home = False
        self.aux_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def aux_stop(self):
        self.aux_moving_up = False
        self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def aux_home(self):
        # self.aux_moving_up = False
        # self.aux_moving_down = False
        self.aux_moving_to_home = True
        self.aux_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def aux_auto(self):
        # self.aux_moving_up = False
        # self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = True
        self.page_insert_command(self.generate_command_text())

    def both_home(self):
        # self.main_moving_up = False
        # self.main_moving_down = False
        self.main_moving_to_home = True
        self.main_doing_auto = False
        # self.aux_moving_up = False
        # self.aux_moving_down = False
        self.aux_moving_to_home = True
        self.aux_doing_auto = False
        self.page_insert_command(self.generate_command_text())

    def both_auto(self):
        # self.main_moving_up = False
        # self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = True
        # self.aux_moving_up = False
        # self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = True
        self.page_insert_command(self.generate_command_text())

    def both_calibrate(self):
        self.main_moving_up = False
        self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = False
        self.aux_moving_up = False
        self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = False
        self.both_calibrating = True
        self.page_insert_command(self.generate_command_text())

    def both_calibrate_done(self):
        self.main_moving_up = False
        self.main_moving_down = False
        self.main_moving_to_home = False
        self.main_doing_auto = False
        self.aux_moving_up = False
        self.aux_moving_down = False
        self.aux_moving_to_home = False
        self.aux_doing_auto = False
        self.both_calibrating = False
        self.page_insert_command(self.generate_command_text())


class Backend:
    def __init__(self, root):
        self.root = root
        self.decoder = Decoder(self)

        # Variables to be "stored" under **kwargs?
        self.main_angle = 0
        self.main_heel = 0
        self.main_fore = 0
        self.aux_angle = 0
        self.aux_heel = 0
        self.aux_fore = 0
        self._dump = 0  # Not really accessed anywhere

    def update_display(self):
        disp = []
        for measurement in [self.main_angle,
                            self.main_heel,
                            self.main_fore,
                            self.aux_angle,
                            self.aux_heel,
                            self.aux_fore
                            ]:
            if measurement != Decoder.NAN:
                disp.append(round(measurement, 2))
            else:
                disp.append(measurement)
        if self.root.page.name == "HomePage":
            self.root.page.angle_main_label.config(text=f"Angle: {disp[0]} deg")
            self.root.page.heel_main_label.config(text=f"Heel Force: {disp[1]}N")
            self.root.page.fore_main_label.config(text=f"Fore Force: {disp[2]}N")
            self.root.page.angle_aux_label.config(text=f"Angle: {disp[3]} deg")
            self.root.page.heel_aux_label.config(text=f"Heel Force: {disp[4]}N")
            self.root.page.fore_aux_label.config(text=f"Fore Force: {disp[5]}N")
