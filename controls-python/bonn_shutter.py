"""
Docstring for repositories.COO-Workbench.controls-python.bonn_shutter
"""
import errno
import time
import socket
import struct
import serial
import serial.tools.list_ports

class BonnShutterCommands:
    """
    Class for Bonn Shutter Commands
    bonn shutter 100m has 2 blades: A and B
    0 corresponds to blade A
    1 corresponds to blade B
    bonn shutter command format:
    <command> <value>\n
    """
    # Command Constants
    OPEN = "os"
    CLOSE = "cs"
    STANDARD_CMDS = "s?"
    SPECIAL_CMDS = "s!"
    INTERACTIVE_MODE = "ia"         # value after->ex: "ia 1"
    BLADE_PROFILE_PARAMS = "sh"     # value after->ex: "sh 0/1" = A/B
    HOST_COM_PARAMS = "pp"
    MSEC_EXPOSE_TIME = "ex"         # value after->ex: "ex 500" for 500 milliseconds
    SHUTTER_IN_APERTURE = "ss"
    ACCEL_PARAMS = "ac"             # value after->ex: "ac 25000"
    CHECK_STATUS = "sv"             # value after->ex: "sv 0/1" = A/B
    FACTORY_RESET = "fd"



class BonnShutterResponses:
    """Class for Bonn Shutter Responses"""
    # Response Constants
    RESP_OK = 0
    RESP_ERROR = 1
    RESP_INVALID_CMD = 2
    RESP_INVALID_PARAM = 3
    RESP_BUSY = 4

class BonnShutterController():
    """
    Docstring for BonnShutterController
    """

    Commands = BonnShutterCommands
    Responses = BonnShutterResponses
    state = {
        'is_open': False,
        'is_busy': False,
        'last_command': None,
        'error_code': None,
        'connection_type': None,  # 'rj45' or 'usb'
        'is_connected': False
    }

    def __init__(self):
        self.socket = None
        self.host = None
        self.port = None
        self.timeout = 5  # Default timeout in seconds
        self._usb_ports = []
        self.ftdi_ports = []
        self.dev = None

    def connect_rj45(self, host: str, port: int):
        """Connect to the Bonn Shutter device."""
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(self.timeout)
        try:
            self.socket.connect((self.host, self.port))
            self.state['connection_type'] = 'rj45'
            self.state['is_connected'] = True
        except socket.error as e:
            raise ConnectionError(f"Could not connect to {self.host}:{self.port} - {e}") from e

    def connect_usb(self, device_path: str):
        """Connect to the Bonn Shutter device via USB (placeholder)."""
        try:
            self.dev = serial.Serial(port=device_path, baudrate=19200, bytesize=8,
                                         stopbits=1, parity=serial.PARITY_NONE)
            self.state['connection_type'] = 'usb'
            self.state['is_connected'] = True
        except serial.SerialException as e:
            raise ConnectionError(f"Could not connect to USB device at {device_path} - {e}") from e

    def list_devices(self):
        """List available USB devices (Bonn Shutter devices typically FTDI)."""
        self._usb_ports=serial.tools.list_ports.comports()

        for p in self._usb_ports:
            print('--------------------')
            print(f'p.pid: {p.pid}')
            print(f'p.device: {p.device}')
            print(f'p.name: {p.name}')
            print(f'p.description: {p.description}')
            print(f'p.manufacturer: {p.manufacturer}')
            if 'FTDI' in p.manufacturer:
                self.ftdi_ports.append(p.device)

    def disconnect(self):
        """Disconnect from the Bonn Shutter device."""
        if self.socket:
            self.socket.close()
            self.socket = None
        if self.dev:
            self.dev.close()
            self.dev = None
        self.state['is_connected'] = False

    def send_command(self, command: str) -> str:
        """Send a command to the Bonn Shutter device and return the response."""
        if not self.socket and not self.dev:
            raise ConnectionError("Not connected to the Bonn Shutter device.")

        try:
            if self.socket:
                self.socket.sendall(command.encode('utf-8') + b'\n')
                response = self.socket.recv(1024).decode('utf-8').strip()
            else:
                self.dev.write((command + '\n').encode('utf-8'))
                response = self.dev.readline().decode('utf-8').strip()
            return response
        except socket.error as e:
            raise IOError(f"Error sending command '{command}': {e}") from e

    def open_shutter(self):
        """Open the Bonn Shutter."""
        response = self.send_command(self.Commands.OPEN)
        if response == "OK":
            self.state['is_open'] = True
            self.state['last_command'] = self.Commands.OPEN
        else:
            self.state['error_code'] = response
            raise RuntimeError(f"Failed to open shutter: {response}")

    def close_shutter(self):
        """Close the Bonn Shutter."""
        response = self.send_command(self.Commands.CLOSE)
        if response == "OK":
            self.state['is_open'] = False
            self.state['last_command'] = self.Commands.CLOSE
        else:
            self.state['error_code'] = response
            raise RuntimeError(f"Failed to close shutter: {response}")
