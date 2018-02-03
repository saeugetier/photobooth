from RPi import GPIO
import platform
if "x86" in platform.machine():
    GPIO.VERBOSE = True #show GPIO in command line

class Light:
    LED_PWM_PIN = 2
    LED_EN_PIN = 3

    def init(self):
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup([self.LED_EN_PIN, self.LED_PWM_PIN], GPIO.OUT)
        GPIO.output(self.LED_EN_PIN, GPIO.HIGH)
        self.pwm = GPIO.PWM(self.LED_PWM_PIN, 5000)
        self.pwm.start(0.0)

    def setBrigthness(self, brigthness):
        try:
            self.pwm.ChangeDutyCycle(brigthness)
        except:
            self.pwm.ChangeDutyCycle(0.0)

    def __del__(self):
        self.pwm.stop()
        GPIO.output(self.LED_EN_PIN, GPIO.LOW)
        GPIO.cleanup()
