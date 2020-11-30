#!/usr/bin/env python3

# todo:
# agregar un handler para SIGINT (Ctrl + c)

from multiprocessing import Process, Pipe, Queue
import socket
#from PyQt5 import QtCore, QtGui, QtWidgets, uic
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.uic import loadUi
import sys, traceback, os, time, glob
from datetime import datetime
from matplotlib.backends.backend_qt5agg import FigureCanvas
from matplotlib.backends.backend_qt5agg import (NavigationToolbar2QT as NavigationToolbar)
from matplotlib.figure import Figure
import numpy as np
import random
from matplotlib.animation import FuncAnimation

p_plot = []
seconds = []


class getValuesThread(QRunnable):
    def __init__(self, q_values, printValues, q_power):
        super(getValuesThread, self).__init__()
        self.q_values = q_values
        self.q_power = q_power
        self.printValues = printValues
        # crear funcion para lo siguiente
        try:
            os.mkdir("logs")
        except OSError as error:
            print(" ")


    @pyqtSlot()
    def run(self):
        Vef = 0.0
        Ief = 0.0
        P = 0.0
        IP = "0.0.0.0"
        voltageScaleFactor = 279.95
        currentScaleFactor = 606

        now = datetime.now()
        fileName = "logs/" + str(now.day) + "_" + str(now.month) + "_" + str(now.year) + "_" + str(now.hour) + "_" + str(now.minute) + "_" + str(now.second) + ".txt"

        time.sleep(1)

        while True:
            now = datetime.now()

            if not self.q_values.empty(): # si la cola de valores recibidos no esta vacia
                payload = self.q_values.get()

                if(payload[0] == 'V' and payload[6] == 'I' and payload[12:] != "0.0.0.0"):

                    Vef = round(float(payload[1:6]) * voltageScaleFactor, 1)
                    if Vef < 50: # si la tension es menor a 50 Vac se considera ruido
                        Vef = 0.0
                        Ief = 0.0
                        P = 0.0
                    else:
                        Ief = round(float(payload[7:12]) * 606)
                        if Ief <= 10: # si es menor o igual 10 mA (2.2 W), se considera ruido
                            Ief = 0
                        P = round(Vef*Ief/1000, 1)

                    self.q_power.put(str(P) + "_" + str(Vef))

                    IP = payload[12:]

                    newLine = str(now.day) + "_" + str(now.month) + "_" + str(now.year) + "_" + str(now.hour) + "_" + str(now.minute) + "_" + str(now.second) + "," + str(P) + "\n"

                    log = open(fileName, "a")
                    log.write(newLine)
                    log.close()

                    self.printValues(IP, str(Vef), str(Ief), str(P))

            time.sleep(0.5)

class MyWindow(QMainWindow):
    def __init__(self, q_buttons, q_values, q_power):
        super(MyWindow,self).__init__()
        loadUi('botones.ui', self)
        self.setWindowTitle("iPDU - UTN")
        self.setIcon()
        self.pushButton_on.clicked.connect(lambda: self.pushOn(q_buttons))
        self.pushButton_off.clicked.connect(lambda: self.pushOff(q_buttons))

        #self.addToolBar(NavigationToolbar(self.MplWidget.canvas, self))

        self.threadpool = QThreadPool()
        self.getValues(q_values, q_power)

    def pushOn(null, q_buttons):
        q_buttons.put("On")

    def pushOff(null, q_buttons):
        q_buttons.put("Off")

    def getValues(self, q_values, q_power):
        getvaluest = getValuesThread(q_values, self.printValues, q_power)
        self.threadpool.start(getvaluest)

    def printValues(self, ipPort, Vef1, Ief1, P1):
        self.host.setText(ipPort)
        self.V1.setText(Vef1)
        self.I1.setText(Ief1)
        self.P1.setText(P1)

        list_of_files = glob.glob('logs/*')
        latest_file = max(list_of_files, key=os.path.getctime)

        pullData = open(latest_file, "r").read()
        dataList = pullData.split('\n')
        yList = []
        for eachLine in dataList:
            if len(eachLine) > 1:
                x, y = eachLine.split(',')
                yList.append(float(y))

        self.MplWidget.canvas.axes.clear()
        self.MplWidget.canvas.axes.plot(yList)
        self.MplWidget.canvas.axes.set_title('Potencia (W)')
        self.MplWidget.canvas.draw()

    def setIcon(self):
        appIcon = QIcon("UTN_logo.png")
        self.setWindowIcon(appIcon)

def server(q_buttons, q_values, q_power):
    try:
        now = datetime.now()
        print("\n{}/{}/{}".format(now.day, now.month, now.year))
        # el Toma 1 se inicializa apagado
        T1 = '0'
        pString = "000.0_000.0"
        lp = 0
        lt = 0
        # se crea un socket UDP de con IPv4
        sock = socket.socket( family=socket.AF_INET, type=socket.SOCK_DGRAM )
        # se elige el puerto
        serverPort = 12345
        # se obtiene la dirección actual
        serverIP = socket.gethostbyaddr(socket.gethostname())
        # se muestra la dirección ip y puerto del serrver UDP
        print("UDP server IP: {} at {}".format(serverIP[2][0], serverPort))
        # tamaño maximo de los paquetes
        bufferSize  = 1024
        # se vincula el socket con la direccion IP y el puerto
        sock.bind(( '', serverPort ))
        print("UDP server up and listening...")
        print(" ")
        # payload por defecto: rele apagado
        payload1 = 'T1_0_000.0_000.0'

        # se pone al server a escuchar para siempre
        while True:
            # los paquetes consisten de mensaje y (IP, puerto)
            bytesAddressPair = sock.recvfrom( bufferSize )
            message = bytesAddressPair[ 0 ]
            address = bytesAddressPair[ 1 ]

            # se arma el payload de la respuesta que consiste en el estado
            # del relé, segun los botones de la UI
            if not q_power.empty() or not q_buttons.empty():
                if not q_power.empty():
                    pString = str(q_power.get())
                    lString = len(pString)
                    e = pString.find("_")
                    if e != -1:
                        lt = lString - e - 1
                        lp = lString - lt - 1
                    if lp == 3:
                        pString = "00" + pString
                    if lp == 4:
                        pString = "0" + pString

                    if lt == 3:
                        pString = pString[:6] + "00" + pString[6:]
                    if lt == 4:
                        pString = pString[:6] + "0" + pString[6:]

                if not q_buttons.empty():
                    toma = q_buttons.get([1])
                    if toma == 'On':
                        T1 = '1'
                    elif toma == 'Off':
                        T1 = '0'
                # se arma el payload de toma 1

                payload1 = 'T1_' + T1 + "_" + pString

            # se responde con el estado que debe tener el rele y la potencia
            sock.sendto( str.encode(payload1), (address[0], 1337) )
            # se pone en la cola el mensaje y la dirección
            print(message.decode())
            msg = message.decode() + str(address[0]) + ':' + str(address[1])
            q_values.put(msg)


    except KeyboardInterrupt:
        print (" ")
        print ("Bajando server...")
        sock.close()
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)

def interfazGrafica(q_buttons, q_values, q_power):
    app = QApplication(sys.argv)
    window = MyWindow(q_buttons, q_values, q_power)
    window.show()

    sys.exit(app.exec_())

if __name__ == '__main__':
    try:
        q_buttons = Queue()
        q_values = Queue()
        q_power = Queue()

        UDPServerp = Process(target=server, args=(q_buttons, q_values, q_power, ))

        gui = Process(target=interfazGrafica, args=(q_buttons, q_values, q_power ))

        gui.start()
        time.sleep(1)
        UDPServerp.start()

        gui.join()
        UDPServerp.join()

    except KeyboardInterrupt:
        print ("Saliendo...")
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
