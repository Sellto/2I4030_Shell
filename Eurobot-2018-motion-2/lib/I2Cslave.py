# SDA pin = 18
# SCL pin = 19
import pigpio


class I2CSlave:
    def __init__(self, pi, i2c_addr):
        self.__pi = pi
        self.__i2c_addr = i2c_addr
        self.__pi.set_pull_up_down(18, pigpio.PUD_UP)
        self.__pi.set_pull_up_down(19, pigpio.PUD_UP)
        self.__event = self.__pi.event_callback(pigpio.EVENT_BSC, self.__i2c)

    def __i2c(self):
        """
        The returned value is a tuple of : 
        the status as s
        the number of bytes read  as nbr
        bytearray containing the read bytes as data
        
        """
        status, nbr , data = self.__pi.bsc_i2c(self.__i2c_addr)
        print(status)
        print(nbr)
        print(data)