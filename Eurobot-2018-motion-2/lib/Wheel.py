import pigpio


class DC_Wheel:
    def __init__(self,pi, pin1, pwd_pin, pwm_freq, pwm_range):
        self.__pi = pi
        self.__conn = {"cmd1": pin1, "pwm": pwd_pin}
        self.__pwm = {"freq": pwm_freq, "range": pwm_range}
        for pin_num in self.__conn.values():
            self.__pi.set_mode(pin_num, pigpio.OUTPUT)
        self.__pi.set_PWM_frequency(self.__conn["pwm"], self.__pwm["freq"])
        self.__pi.set_PWM_range(self.__conn["pwm"], self.__pwm["range"])

    # Try with the pwm on the enable and keep pin1 and pin2 in the same state during the movement
    # Advantage only 1 PWMs - Disadvantage 3 pins needed for each wheels.

    # Direction transform value into "Boolean"

    def move(self, value):
        direction = (value + abs(value))//(2*abs(value))
        print(direction)
        self.__pi.write(self.__conn["cmd1"], direction)
        self.__pi.set_PWM_dutycycle(self.__conn["pwm"], abs(value))

    def brake(self):
        self.__pi.set_PWM_dutycycle(self.__conn["pwm"],0)
