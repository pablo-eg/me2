# -*- coding: utf-8 -*-
#!/usr/bin/env python3

# Para los calculos
import numpy as np

import  math

import scipy                  # http://scipy.org/
from scipy import constants

def dButoV(dBu):

    v = 10**(dBu/20)

    print('\nV = {:.2f} uV\n\n'.format(v))

while 1:
    dBu = float(input("Enter dBu: "))
    dButoV(dBu)
    pass
