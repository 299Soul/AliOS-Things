from driver import ADC

class FIRE(object):

    def __init__(self, adcObj):
        self.adcObj = None
        if not isinstance(adcObj, ADC):
            raise ValueError("parameter is not an ADC object")
        self.adcObj = adcObj

    def getVoltage(self):
        if self.adcObj is None:
            raise ValueError("invalid ADC object")
        value = self.adcObj.read()
        return value
