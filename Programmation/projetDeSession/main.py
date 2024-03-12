import time

import PyQt5
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

angleEpaule = 0
angleCoude = 0
anglePoignet = 0

commandeMoteur = ""
commandeAEnvoyer = False

lock = threading.Lock()

class gestionPort():
    def __init__(self):
        self.stringAngles = [0,0,0]
        self.serial_port = serial.Serial('COM12', 9600, timeout=1)
        self.serial_port.flush()
    def lireValeurs(self) -> []:
        if self.serial_port.in_waiting > 0:
            self.stringAngles = self.serial_port.readline().decode('utf-8').strip().split(',')

        return self.stringAngles
    def envoieCommande(self):
        global commandeMoteur
        with lock:
            commandeMoteur = commandeMoteur + "\n"
            print(commandeMoteur.encode("utf-8"))
            self.serial_port.write(commandeMoteur.encode("utf-8"))


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
<<<<<<< Updated upstream
        self.plot_widget.setXRange(-12, 12)
        self.plot_widget.setYRange(-12,12)
=======
        self.plot_widget.setXRange(0, 5)
        self.plot_widget.setYRange(0,8)
>>>>>>> Stashed changes
        self.layout.addWidget(self.plot_widget)

        self.plotGraph = self.plot_widget
        self.setCentralWidget(self.plotGraph)
        self.pen = pg.mkPen(color=(255, 0, 127), width=15) #permet de donner un style au ligne de graphique

        self.data = deque(maxlen=1000)  # Liste de data contenant les angles
        self.x = 0
        self.y = 0

<<<<<<< Updated upstream
=======
        # Serial port initialization (change the port and baud rate accordingly)
       # self.serial_port = serial.Serial('COM12', 9600)

>>>>>>> Stashed changes
        self.timer = pg.QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(200)  # Update plot every ___ milliseconds

    def update_plot(self):

        self.plotGraph.clear()
        try:
<<<<<<< Updated upstream

            #Permet de lire les angles de l'arduino et les placer dans un tableau

            #Longueur entre les points d'imu
            l1 = 6
            l2 = 4
            l3 = 2

            #Ajoute un point à (1,1)
            self.data.append((1, 1))

            with lock:
                point1x = l1*np.cos(angleEpaule)
                point1y = l1*np.sin(angleEpaule)
            point1 = (point1x, point1y)
            self.data.append(point1)

            with lock:
                point2x = point1x + l2*np.cos(angleCoude)
                point2y = point1y + l2*np.sin(angleCoude)
            point2 = (point2x, point2y)
            self.data.append(point2)

            with lock:
                point3x = point2x + l3*np.cos(anglePoignet)
                point3y = point2y + l3*np.cos(anglePoignet)
            point3 = (point3x, point3y)
            self.data.append(point3)

            self.plotGraph.plot([x for x, _ in self.data], [y for _, y in self.data], pen=self.pen)
            self.data.clear()
=======
            """
            if self.serial_port.in_waiting > 0:
                value = float(self.serial_port.readline().decode().strip())
            """

            self.data.append((self.x, 4))
            self.y += 1
            self.x += 1
            #le problème est ici quand tu pognes
            self.curve.setData([x for x, _ in self.data], [y for _, y in self.data])
>>>>>>> Stashed changes


        except Exception as e:
            print(e)


class Application(customtkinter.CTk):

    def __init__(self, *args, **kwargs):

        super().__init__(*args, **kwargs)

        self.title('Exo')
        self.geometry('700X500')

        #Initialisation des différentes affaires de l'interface
       # self.labelTitre = customtkinter.CTkLabel(master=self, font=("Arial Black", 32), text="Info Exosquelette")
        self.labelMoteurPoignet = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Poignet")
        self.labelMoteurCoude = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Coude")
        self.labelMoteurEpaule = customtkinter.CTkLabel(master=self, font=("Arial", 16), text="Moteur Épaule")

        self.entreePoignet = customtkinter.CTkEntry(master=self)
        #self.entreeCoude = customtkinter.CTkEntry(master=self)
        #self.entreeEpaule = customtkinter.CTkEntry(master=self)

        self.boutonMoteur = customtkinter.CTkButton(master=self, text="Envoie commande poignet",
                                                     command=self.commandeBouton)
        #self.boutonCoude = customtkinter.CTkButton(master=self, text="Envoie commande coude",
        #                                           command=self.commandeBoutonCoude)
        #self.boutonEpaule = customtkinter.CTkButton(master=self, text="Envoie commande épaule",
        #                                            command=self.commandeBoutonEpaule)

        #Initialisation de la géométrie des items dans l'interface
        #self.labelTitre.grid(row=0, column=1, pady=40)

        #self.labelMoteurPoignet.grid(row=2, column=0, padx=5, pady=15)
        #self.labelMoteurCoude.grid(row=3, column=0, padx=5, pady=15)
        #self.labelMoteurEpaule.grid(row=4, column=0, padx=5, pady=15)

        self.entreePoignet.grid(row=3, column=0, padx=5, pady=15)
        #self.entreeCoude.grid(row=3, column=1, padx=5, pady=15)
        #self.entreeEpaule.grid(row=4, column=1, padx=5, pady=15)

        self.boutonMoteur.grid(row=4, column=0, pady=5)
        #self.boutonCoude.grid(row=3, column=2, pady=5)
        #self.boutonEpaule.grid(row=4, column=2, pady=5)

        self.menu = customtkinter.CTkOptionMenu(master=self, values=["Poignet", "Coude", "Épaule", "Tous les moteurs"])
        self.menu.set("Choix moteur")
        self.menu.grid(row=2, column=0, padx=40, pady=40)
        self.mainloop()

        #Possibilité d'ajout d'un menu


    def commandeBouton(self):

        global commandeMoteur
        global commandeAEnvoyer
        with lock:
            commandeAEnvoyer = True
            #mettre des condition pour le get
            commandeMoteur = self.entreePoignet.get()



#Permet de lançer le graph dans un autre thread
def Graph():
    app = QApplication(sys.argv)
    window = RealTimePlot()
    window.show()
    sys.exit(app.exec_())

def gestionPortSerie():
    global angleCoude
    global angleEpaule
    global anglePoignet
    global commandeAEnvoyer

    objet = gestionPort()
    while 1:
        with lock:
            stringAngle = objet.lireValeurs()
            angleEpaule = float(stringAngle[0])
            angleCoude = float(stringAngle[1])
            anglePoignet = float(stringAngle[2])
        print(stringAngle)

        if(commandeAEnvoyer == True):
            objet.envoieCommande()
            with lock:
             commandeAEnvoyer = False

        time.sleep(0.2)

if __name__ == "__main__":

    # threadLecture = threading.Thread(target=lirePortSerie)
    # threadLecture.start()

    threadApp = threading.Thread(target=Application).start()
    threadGraph = threading.Thread(target=Graph).start()
    threadLireAngle = threading.Thread(target=gestionPortSerie).start()








