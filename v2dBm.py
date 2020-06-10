# -*- coding: utf-8 -*-
#!/usr/bin/env python3

# Para los calculos
import numpy as np

import  math

import scipy                  # http://scipy.org/
from scipy import constants

def dBm(voltage):

    dBm = 20 * math.log10(v)
    dBu = 20 * math.log10(v*10**3)

    print('\ndbm = {:.2f}'.format(dBm))
    print('\ndbu = {:.2f}\n\n'.format(dBu))

while 1:
    v = float(input("Enter Voltage in mV: "))
    dBm(v)
    pass
