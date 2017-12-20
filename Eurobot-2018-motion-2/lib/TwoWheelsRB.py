import yaml

from lib.Wheel import DC_Wheel
from lib.Encoder import Decoder
from lib.I2Cslave import I2CSlave


class TwoWheelsRB(I2CSlave):
    def __init__(self, pi, file, i2c_addr=0x13):
        config = yaml.load(open(file))
#        I2CSlave.__init__(self,pi, i2c_addr)
        self.__ros_connection = None
        self.__left_wheel  = DC_Wheel(pi,
                                      config["L_DC_pin1"],
                                      config["L_DC_pwm_pin"],
                                      config["PWM_Frequency"],
                                      config["PWM_Range"])
        self.__right_wheel = DC_Wheel(pi,
                                      config["R_DC_pin1"],
                                      config["R_DC_pwm_pin"],
                                      config["PWM_Frequency"],
                                      config["PWM_Range"])
        self.__left_encoder = Decoder(pi,
                                       config["L_RD_pinA"],
                                       config["L_RD_pinB"],
                                       )
        self.__right_encoder = Decoder(pi,
                                       config["R_RD_pinA"],
                                       config["R_RD_pinB"],
                                       )

    def move(self,speed):
        self.right_wheel.move(speed)
        self.left_wheel.move(speed)

    def brake(self):
        self.right_wheel.brake()
        self.left_wheel.brake()


    @property
    def left_wheel(self):
        return self.__left_wheel

    @property
    def right_wheel(self):
        return self.__right_wheel

    @property
    def left_encoder(self):
        return self.__left_encoder

    @property
    def right_encoder(self):
        return self.__right_encoder
