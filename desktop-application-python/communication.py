"""
Author: Rafael Remo - The Gaitkeepers

TML PLATFORM APPLICATION: Communication classes
- These classes handles all functions related to serial communication
"""

# Imports
import queue
import serial
import serial.tools.list_ports
import threading as th


class Buffer:
    """
    Contains:
    - Receive buffer
    - Send buffer (Do I even need a buffer for this?)
    - Methods to add/remove from buffers
    """

    BUFSIZE = 0

    def __init__(self):
        self.receive_buffer = queue.Queue(maxsize=Buffer.BUFSIZE)
        self.send_buffer = queue.Queue(maxsize=Buffer.BUFSIZE)  # if needed in the future

    def receive_enqueue(self, item):
        self.receive_buffer.put(item)  # blocking

    def receive_dequeue(self):
        try:
            _item = self.receive_buffer.get()  # Blocking (adding timeout still has error of thread not ending on disconnect)
            return _item
        except Exception:  # can remove this try except block (since .get() is blocking)
            return str.encode('.')

    def receive_is_waiting(self):
        return not self.receive_buffer.empty()

    def receive_clear(self):
        self.receive_buffer = queue.Queue(maxsize=Buffer.BUFSIZE)


class COM:
    """
    Contains:
    - Serial to buffer thread
    - Buffer to serial commands (thread? Or straight from GUI?)
    - Serial connection state
    - Methods to connect/disconnect serial
    """
    comport_esp = "USB Serial Device"

    connected_message = "Connected to Serial!"
    disconnect_message = "Disconnected from Serial!"
    sent_message = "Info sent through Serial!"
    not_connected_message = "Serial is not connected!"
    already_connected_message = "Already connected!"
    already_disconnected_message = "Already disconnected!"

    # Make sure in firmware, commands are uninterrupted by data stream
    stream_start = '>'
    command_start = '~'  # Includes commands and messages
    command_end = '<'  # Packed architecture: {start} {cmd} {data} {end}
    separator = '|'

    def __init__(self, rbuf: Buffer):  # One of the inputs is the receive buffer
        self.byte_decoding_method = "utf-8"
        self.com_port = None
        self.baud = 115200

        self.rbuf = rbuf

        self.connection = None  # Will initialize when connecting
        self.serial_connected = False

        self.serial_read_thread = th.Thread(target=self.serial_read_loop, daemon=True)  # Thread ready to start

        self.lockout = False

    # Finds the correct com port for ESP connection, then returns it as a string
    def find_comport(self) -> str:
        valid_ports = [port.device for port in serial.tools.list_ports.comports() if
                       COM.comport_esp in port.description]

        if not valid_ports:
            return None

        self.com_port = valid_ports[0]
        return self.com_port  # This only works if there's not multiple options to connect to (only connect 1 microcontroller)

    def get_port(self):  # Mostly for debugging
        return self.com_port

    def serial_send_packet(self, msg: str):  # Unused for now
        # Use packet in case we need to send data over serial
        # {command_start} {1 digit commang} {command_end}
        # Might need to change command packet to accomodate for more commands to MCU. Messages can be done similarly to desktop code? Do I have to use pointers?
        if (
                self.serial_send(COM.command_start) == COM.sent_message and
                self.serial_send(msg) == COM.sent_message and
                self.serial_send(COM.command_end) == COM.sent_message
        ):
            return COM.sent_message

    def serial_send(self, msg: str) -> str:  # return success/failure
        # Assumes sending buffer is not needed
        if not self.lockout:  # if not locked out
            if self.serial_connected:
                try:
                    self.connection.write(str.encode(msg))
                    return COM.sent_message
                except Exception as e:
                    return e
            else:
                return COM.not_connected_message

    def serial_read_loop(self):
        while self.serial_connected:  # Will automatically terminate when serial disconnects
            try:
                in_waiting = self.connection.in_waiting
            except Exception:
                continue
            if self.connection != None and in_waiting > 0:
                try:
                    data = self.connection.read()
                    self.rbuf.receive_enqueue(data)
                except Exception:
                    continue

    def connect(self) -> str:  # return message on status (string)
        # print("Start connecting....")
        if self.serial_connected is False:
            try:
                self.connection = serial.Serial(self.com_port, self.baud)
                self.serial_connected = True
#                 print("Connecting!")

                self.serial_read_thread = th.Thread(target=self.serial_read_loop, daemon=True)  # Thread ready to start
                self.serial_read_thread.start()

                return COM.connected_message
            except Exception as e:
                self.serial_connected = False
#                 print("Couldn't Connect :(")
                return f"Error... {e}"
        else:
            return COM.already_connected_message

    def disconnect(self) -> str:  # return message on status (string)
        if self.serial_connected:
            try:
                self.serial_connected = False
                self.connection.close()

                return COM.disconnect_message
            except Exception as e:
                self.serial_connected = True
#                 print("Couldn't Disconnect :(")
                return f"Error... {e}"
        else:
            return COM.already_disconnected_message

