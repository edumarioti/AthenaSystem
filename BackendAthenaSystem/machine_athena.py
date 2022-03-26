from datetime import datetime
from typing import Callable
import serial
from time import sleep



SLOW = 1200
FAST = 8000
DEFAULT_FEED_RATE = FAST

POSITION_IN = 'A'
POSITION_OUT = 'Z'

Z_STANDARD_ELEVATION = 80
Z_TO_PICK_UP = 9

WAKE_UP_GRBL = b" \r \n \r \n"

BASE_GO_TO_COORDINATE  = 'G21 G90 G1 '
BASE_INCREMENTAL_MOV = 'G21 G91 G1 '
CMD_SET_ZERO = 'G10 P0 L20 X0  Y0  Z104'
CMD_PICK_UP = 'M3'
CMD_DROP = 'M5'
CMD_HOME = '$H'
CMD_GET_STATUS = '?'
CMD_RESET_ALARMS = '$X'
CMD_SOFT_RESET = '^X'
CMD_GET_STATE = '$G'
CMD_RETURN_ZERO = 'G21 G90 G1 X0 Y0 X104'

SPEED_CONVERSION_FACTOR = 1/60000 #mm/minuto to mm/ms

MANUAL = 0
AUTOMATICO = 1

UNDEFINED = 0
IDLE = 1
RUN = 2
ALARM = 3

DEFAUL_MANUAL_MOVIMENT_DISTANCE = 10


X = 0
Y = 1
Z = 2

class Machine:
    
    def __init__(self, port:str):
        self.__buffer_commands = []
        self._update_products = False
        self._current_position = [0,0,0]
        self._machine_status = 0
        self._pick_up = False

        self._points_busy = {'B': False, 'C': False, 'D': False, 'E': False,
                             'F': False, 'G': False, 'H': False, 'I': False,
                             'J': False, 'K': False, 'L': False, 'M': False,
                             'N': False, 'O': False, 'P': False, 'Q': False,
                             'R': False, 'S': False, 'T': False, 'U': False
                            }

        self.__coordinate_for_points = {'A': ( 11, 140), 'B': ( 71, 187), 'C': ( 71, 140), 'D': ( 71,  93),
                                        'E': ( 71,  46), 'F': ( 72,   0), 'G': (132, 187), 'H': (132, 140),
                                        'I': (132,  93), 'J': (131,  45), 'K': (131,   0), 'L': (192, 187),
                                        'M': (192, 141), 'N': (192,  93), 'O': (192,  46), 'P': (191,   0),
                                        'Q': (251, 188), 'R': (251, 140), 'S': (251,  92), 'T': (251,  45),
                                        'U': (251,   0), 'Z': (311,  45)
                                        }

        self.__state = MANUAL
        self.__serial = serial.Serial(port, 115200)
        
        self.change_manual_movement_distance()

        self.__serial.write(WAKE_UP_GRBL)
        self.__serial.write(WAKE_UP_GRBL)
        sleep(0.5)
        self.__serial.read_all()


    def __create_gcode_for_coordinate_increment(self, x=0, y=0, z=0, f=DEFAULT_FEED_RATE):
        
        command = BASE_INCREMENTAL_MOV + f'X{x} Y{y} Z{z} F{f}'

        self.__add_buffer_of_commands(command)


    def __create_gcode_for_go_to_coordinate(self, x=None, y=None, z=None, f=DEFAULT_FEED_RATE):
        
        command = BASE_GO_TO_COORDINATE
        
        if x != None:
            command += f'X{x} '

        if y != None:
            command += f'Y{y} '
        
        if z != None:
            command += f'Z{z} '
            
        command += f'F{f}'

        self.__add_buffer_of_commands(command)


    def __send_gcode_for_serial(self, command=""):
        
        command_formated = "\r\n" + command + "\r\n"

        self.__serial.write(command_formated.encode('utf-8'))        
        sleep(0.1)

        #print(command)
        feed_back = self.__serial.read_all().decode('utf-8')
        
        #print(feed_back)
        return feed_back


    def go_to_position(self, position:str):

        position = position.upper()
        x, y = self.__coordinate_for_points[position]

        self.__create_gcode_for_go_to_coordinate(x=x, y=y)


    def go_to_home_position(self):

        self.__send_gcode_for_serial(CMD_HOME)
        self.__send_gcode_for_serial(CMD_SET_ZERO)


    def create_manual_movement(self, direction):
        
        command = BASE_INCREMENTAL_MOV

        if direction > 0:

            for key, moviment in self.__base_for_movement_manual:

                if key == direction:
                    command += moviment

        command += f"F{DEFAULT_FEED_RATE}"
        
        self.__send_gcode_for_serial(command)


    def increment_coordinate(self, value):
        self.__create_gcode_for_coordinate_increment(value[X], value[Y], value[Z])


    def set_manual_auto(self, state):
        self.__state = state


    def change_manual_movement_distance(self, value=DEFAUL_MANUAL_MOVIMENT_DISTANCE):
        
        self.__manual_movement_distance = value

        x = str(self.__manual_movement_distance)
        y = str(self.__manual_movement_distance)
        self.__base_for_movement_manual =  ((1, f"Y {y} "), (2, f"X-{x} Y {y} "),
                                            (3, f"X-{x} "), (4, f"X-{x} Y-{y} "),
                                            (5, f"Y-{y} "), (6, f"X {x} Y-{y} "),
                                            (7, f"X {x} "), (8, f"X {x} Y {y} "))   


    def manual_z_axis(self, up):
        if up:
            self.__create_gcode_for_coordinate_increment(z=self.__manual_movement_distance)
        else:
            self.__create_gcode_for_coordinate_increment(z= -self.__manual_movement_distance)


    def get_state(self):
        
        state = (int(self.__state),
                 self._current_position[X],
                 self._current_position[Y],
                 self._current_position[Z],
                 self._machine_status,
                 int(self._pick_up),
                 int(self._update_products)
        )
        
        return state


    def get_state_grbl(self):

        state = self.__send_gcode_for_serial(CMD_GET_STATE)

        position = state.find(CMD_PICK_UP)

        self._pick_up = position > 0


    def pick_up(self, value:bool):
        
        if value:
            command = CMD_PICK_UP
        else:
            command = CMD_DROP
        
        self.__add_buffer_of_commands(command)


    def get_feed_back_grbl(self):

        feed_back = self.__send_gcode_for_serial(CMD_GET_STATUS)

        #<Alarm,MPos:0.000,0.000,0.000,WPos:309.000,189.000,1.000>

        position_init = feed_back.find('WPos:')
        position_finish = feed_back.find('>')

        if position_init > 0:

            position_init += 5

            values = feed_back[position_init:position_finish]

            values = list(eval(values))
            
            self._current_position = values


        state_init = feed_back.find('<')
        state_finish = feed_back.find(',')
        
        if state_init > 0:

            state_init += 1

            values = feed_back[state_init:state_finish]

            if values == 'Idle':
                self._machine_status = IDLE
                self.__execute_buffer_commmands()

            elif values == 'Run':
                self._machine_status = RUN

            elif values == 'Home':
                self._machine_status = RUN
            
            elif values == 'Alarm':
                self._machine_status = ALARM

        else:
            self._machine_status = UNDEFINED
        
        self.get_state_grbl()


    def reset_machine(self):
        
        self.__send_gcode_for_serial(CMD_RESET_ALARMS)
        self.__serial.write(b"\030")

        
    def remove_in_position(self, position):
        self._points_busy[position] = False
        self.pick_up(value=False)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self.go_to_position(position)
        self.__create_gcode_for_go_to_coordinate(z=Z_TO_PICK_UP)
        self.pick_up(value=True)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self.go_to_position(POSITION_OUT)
        self.__create_gcode_for_go_to_coordinate(z=Z_TO_PICK_UP)
        self.pick_up(value=False)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self._update_products = True


    def insert_in_position(self, position):
        self._points_busy[position] = True
        self.pick_up(value=False)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self.go_to_position(POSITION_IN)
        self.__create_gcode_for_go_to_coordinate(z=Z_TO_PICK_UP)
        self.pick_up(value=True)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self.go_to_position(position)
        self.__create_gcode_for_go_to_coordinate(z=Z_TO_PICK_UP)
        self.pick_up(value=False)
        self.__create_gcode_for_go_to_coordinate(z=Z_STANDARD_ELEVATION)
        self._update_products = True
        

    def get_free_point(self):

        for point, busy in self._points_busy.items():
            if not busy:
                return point
    

    def current_products_updated(self):
        self._update_products = False


    def __add_buffer_of_commands(self, command):
        self.__buffer_commands.append(command)


    def __execute_buffer_commmands(self):

        if self.__buffer_commands != []:
            self.__send_gcode_for_serial(self.__buffer_commands[0])
            self.__buffer_commands.pop(0)
