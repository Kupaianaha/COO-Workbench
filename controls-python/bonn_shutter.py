"""
Docstring for repositories.COO-Workbench.controls-python.bonn_shutter
"""
import errno
import time
import socket
import struct

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
    }

    def __init__(self):
        self.socket = None
        self.host = None
        self.port = None
        self.timeout = 5  # Default timeout in seconds

    def connect(self, host: str, port: int):
        """Connect to the Bonn Shutter device."""
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(self.timeout)
        try:
            self.socket.connect((self.host, self.port))
        except socket.error as e:
            raise ConnectionError(f"Could not connect to {self.host}:{self.port} - {e}") from e

    def disconnect(self):
        """Disconnect from the Bonn Shutter device."""
        if self.socket:
            self.socket.close()
            self.socket = None

    def send_command(self, command: str) -> str:
        """Send a command to the Bonn Shutter device and return the response."""
        if not self.socket:
            raise ConnectionError("Not connected to the Bonn Shutter device.")

        try:
            self.socket.sendall(command.encode('utf-8') + b'\n')
            response = self.socket.recv(1024).decode('utf-8').strip()
            return response
        except socket.error as e:
            raise IOError(f"Error sending command '{command}': {e}") from e
