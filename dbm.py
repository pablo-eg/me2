# -*- coding: utf-8 -*-
#!/usr/bin/env python3

# Para los calculos
import numpy as np

import  math

import scipy                  # http://scipy.org/
from scipy import constants

def dbm(voltage):

    dbm = 20 * math.log10(v)
    dbu = 20 * math.log10(v*10**3)

    print('\ndbm = {:f}'.format(dbm))
    print('\ndbu = {:f}\n\n'.format(dbu))

while 1:
    v = float(input("Enter Voltage in mV: "))
    dbm(v)
    pass
