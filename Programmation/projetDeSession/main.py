import time

import customtkinter
import serial
import threading
import matplotlib

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np


import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
import pyqtgraph as pg
from collections import deque

customtkinter.set_appearance_mode("dark")
customtkinter.set_default_color_theme("dark-blue")

class RealTimePlot(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Real-Time Plot")
        self.setGeometry(100, 100, 800, 600)

        self.widget = QWidget(self)
        self.setCentralWidget(self.widget)

        self.layout = QVBoxLayout()
        self.widget.setLayout(self.layout)

        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setXRange(0, 25)
        self.plot_widget.setYRange(0,8)
        self.layout.addWidget(self.plot_widget)

        self.curve = self.plot_widget.plot()

        self.data = deque(maxlen=10000)  # Store data points
        self.x = 0  # Initial x value
        self.y = 0

        # Serial port initialization (change the port and baud rate accordingly)
        self.serial_port = serial.Serial('COM12', 9600)

        self.timer = pg.QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(50)  # Update plot every 50 milliseconds


    def update_plot(self):
        try:
            if self.serial_port.in_waiting > 0:
                value = float(self.serial_port.readline().decode().strip())
                self.data.append((self.x, value))
                self.y += 1
                self.x += 1
                #le problème est ici quand tu pognes
                self.curve.setData([x for x, _ in self.data], [y for _, y in self.data])
                if(self.y % 25 == 0):
                    self.plot_widget.setXRange(self.y, self.y+25)

        except Exception as e:
            print(e)


class Application(customtkinter.CTk):

    def __init__(self, *args, **kwargs):

        super().__init__(*args, **kwargs)

        self.title('Exo')
        self.geometry('700X500')

        self.labelTitre = customtkinter.CTkLabel(master=self, font=("Arial Black", 32), text="Info Exosquelette")
        self.labelMoteurPoignet = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Poignet")
        self.labelMoteurCoude = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Coude")
        self.labelMoteurEpaule = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Épaule")

        self.entreePoignet = customtkinter.CTkEntry(master=self)
        self.entreeCoude = customtkinter.CTkEntry(master=self)
        self.entreeEpaule = customtkinter.CTkEntry(master=self)

        self.boutonPoignet = customtkinter.CTkButton(master=self, text="Envoie commande poignet",
                                                     command=self.commandeBoutonPoignet)
        self.boutonCoude = customtkinter.CTkButton(master=self, text="Envoie commande coude",
                                                   command=self.commandeBoutonCoude)
        self.boutonEpaule = customtkinter.CTkButton(master=self, text="Envoie commande épaule",
                                                    command=self.commandeBoutonEpaule)

        ##########################################ENDROIT QUI PLACE LES SHIT######################################################################################3
        self.labelTitre.grid(row=0, column=1, pady=40)

        self.labelMoteurPoignet.grid(row=2, column=0, padx=5, pady=15)
        self.labelMoteurCoude.grid(row=3, column=0, padx=5, pady=15)
        self.labelMoteurEpaule.grid(row=4, column=0, padx=5, pady=15)

        self.entreePoignet.grid(row=2, column=1, padx=5, pady=15)
        self.entreeCoude.grid(row=3, column=1, padx=5, pady=15)
        self.entreeEpaule.grid(row=4, column=1, padx=5, pady=15)

        self.boutonPoignet.grid(row=2, column=2, pady=5)
        self.boutonCoude.grid(row=3, column=2, pady=5)
        self.boutonEpaule.grid(row=4, column=2, pady=5)

        self.menu = customtkinter.CTkOptionMenu(master=self, values=["Option 1", "Option 2"])
        self.menu.set("menu")
        self.mainloop()

    # menu.grid(row=1, column=3, padx=5, pady=5)

    def commandeBoutonPoignet(self):
        if self.entreeCoude.get() == "" and self.entreeEpaule.get() == "":
            print(self.entreePoignet.get())
        else:
            print("Veuiller vider les zones de textes")

    def commandeBoutonCoude(self):
        if self.entreePoignet.get() == "" and self.entreeEpaule.get() == "":
            print(self.entreeCoude.get())
        else:
            print("Veuiller vider les zones de textes")

    def commandeBoutonEpaule(self):
        if self.entreeCoude.get() == "" and self.entreePoignet.get() == "":
            print(self.entreeEpaule.get())
        else:
            print("Veuiller vider les zones de textes")

def Graph():
    app = QApplication(sys.argv)
    window = RealTimePlot()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":

    # threadLecture = threading.Thread(target=lirePortSerie)
    # threadLecture.start()

    threadApp = threading.Thread(target=Application).start()
    threadGraph = threading.Thread(target=Graph).start()








