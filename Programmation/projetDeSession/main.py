import time

import customtkinter
import serial
import threading
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
        self.plot_widget.setXRange(-12, 12)
        self.plot_widget.setYRange(-12,12)
        self.layout.addWidget(self.plot_widget)

        self.plotGraph = self.plot_widget
        self.setCentralWidget(self.plotGraph)
        self.pen = pg.mkPen(color=(255, 0, 0), width=15) #permet de donner un style au ligne de graphique

        self.data = deque(maxlen=4)  # Liste de data contenant les angles
        self.x = 0
        self.y = 0

        self.serial_port = serial.Serial('COM12', 9600)

        self.timer = pg.QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(200)  # Update plot every ___ milliseconds





    def update_plot(self):
        self.plotGraph.clear()
        try:
            if self.serial_port.in_waiting > 0:

                #Permet de lire les angles de l'arduino et les placer dans un tableau
                stringAngles = self.serial_port.readline().decode().strip().split()
                angle = float(stringAngles[0])
                angle2 = float(stringAngles[1])
                angle3 = float(stringAngles[2])

                #Longueur entre les points d'imu
                l1 = 6
                l2 = 4
                l3 = 2

                #Ajoute un point à (1,1)
                self.data.append((1, 1))

                point1x = l1*np.cos(angle)
                point1y = l1*np.sin(angle)
                point1 = (point1x, point1y)
                self.data.append(point1)

                point2x = l1*np.cos(angle) + l2*np.cos(angle2)
                point2y = l1*np.sin(angle) + l2*np.sin(angle2)
                point2 = (point2x, point2y)
                self.data.append(point2)

                point3x = l1*np.cos(angle) + l2*np.cos(angle2) + l3*np.cos(angle3)
                point3y = l1*np.sin(angle) + l2*np.sin(angle2) + l3*np.cos(angle3)
                point3 = (point3x, point3y)
                self.data.append(point3)

                self.plotGraph.plot([x for x, _ in self.data], [y for _, y in self.data], pen=self.pen)
                self.data.clear()


        except Exception as e:
            print(e)


class Application(customtkinter.CTk):

    def __init__(self, *args, **kwargs):

        super().__init__(*args, **kwargs)

        self.title('Exo')
        self.geometry('700X500')

        #Initialisation des différentes affaires de l'interface
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

        #Initialisation de la géométrie des items dans l'interface
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

        #Possibilité d'ajout d'un menu
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

#Permet de lançer le graph dans un autre thread
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








