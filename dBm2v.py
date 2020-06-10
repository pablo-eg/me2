# -*- coding: utf-8 -*-
#!/usr/bin/env python3

# Para los calculos
import numpy as np

import  math

import scipy                  # http://scipy.org/
from scipy import constants

def dBmtoV(dBm):

    v = 10**(dBm/20)

    print('\nV = {:.5f} mV\n\n'.format(v))

while 1:
    dBm = float(input("Enter dBm: "))
    dBmtoV(dBm)
    pass
