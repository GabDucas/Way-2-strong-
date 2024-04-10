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

temps = 0

angleEpaule = 0
angleCoude = 0
anglePoignet = 0

torqueEpaule = 0
torqueCoude = 0
torquePoignet = 0

mode_moteur = 42

commandeMoteur = ""
ancienneCommandeMoteur = "commande de moteur"
commandeAEnvoyer = False

lock = threading.Lock()

#Classe permettant de gérer la lecture du port série
class gestionPort():

    #Initialiser le port série
    def __init__(self):
        self.stringAngles = [0,0,0,0,0,0,0]
        self.serial_port = serial.Serial('COM8', 9600, timeout=1)
        self.serial_port.flush()
    def lireValeurs(self) -> []:
        if self.serial_port.in_waiting > 0:
            self.stringAngles = self.serial_port.readline().decode('utf-8').strip().split(',')

        return self.stringAngles

    def envoieCommande(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        with lock:
            commandeMoteur = (commandeMoteur +  ",\n")
            #if commandeMoteur != (ancienneCommandeMoteur+  ",\n"):
            self.serial_port.write(commandeMoteur.encode())
            ancienneCommandeMoteur = commandeMoteur

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
        self.curve = self.plotGraph
        self.pen = pg.mkPen(color=(255, 0, 127), width=15) #permet de donner un style au ligne de graphique

        self.data = deque(maxlen=1000)  # Liste de data contenant les angles
        self.x = 0
        self.y = 0

        self.timer = pg.QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(200)  # Update plot every ___ milliseconds

    def update_plot(self):

        self.plotGraph.clear()
        try:
            #Longueur entre les points d'imu
            l1 = 3
            l2 = 3
            l3 = 2


            #Ajoute un point à (1,1)
            self.data.append((0, 0))

            angleEpauleRad = (angleEpaule % 360) *2*np.pi/360
            angleCoudeRad = (angleCoude % 360) * 2 * np.pi / 360
            anglePoignetRad = (anglePoignet % 360) * 2 * np.pi / 360
            with lock:
                point1x = l1*np.cos(angleEpauleRad)
                point1y = l1*np.sin(angleEpauleRad)
                point1 = (point1x, -point1y)
            self.data.append(point1)

            with lock:
                point2x = point1x + l2*np.cos(angleCoudeRad + angleEpauleRad)
                point2y = point1y + l2*np.sin(angleCoudeRad + angleEpauleRad)
                point2 = (point2x, -point2y)
            self.data.append(point2)

            with lock:
                point3x = point2x + l3*np.cos(anglePoignetRad + angleCoudeRad + angleEpauleRad)
                point3y = point2y + l3*np.sin(anglePoignetRad + angleCoudeRad + angleEpauleRad)
                point3 = (point3x, -point3y)
            self.data.append(point3)


            self.plotGraph.plot([x for x, _ in self.data], [y for _, y in self.data], pen=self.pen)
            self.data.clear()


        except Exception as e:
            print(e)


class Application(customtkinter.CTk):
    global temps
    global angleEpaule
    global angleCoude
    global anglePoignet
    global torqueEpaule
    global torqueCoude
    global torquePoignet

    def __init__(self, *args, **kwargs):

        super().__init__(*args, **kwargs)

        back_color = "gray18"

        self.title('Exosquelette')
        self.geometry('1400X1000')

        self.grid_columnconfigure(0, weight=2)
        self.grid_rowconfigure(0, weight=2)

        self.LabelTitre = customtkinter.CTkLabel(master=self, font = ("Helvetica", 24), text="Panneau de contrôle")
        self.LabelTitre.grid(row=0, column=0, columnspan=2)

        self.frameGauche = customtkinter.CTkFrame(self, fg_color=back_color)
        self.frameGauche.grid(row=1, column=0, padx=10, pady=10)

        self.frameDroit = customtkinter.CTkFrame(self, fg_color=back_color)
        self.frameDroit.grid(row=1, column=1, padx=10, pady=10)

        ## Frame Gauche
        self.frameGauche_Haut = customtkinter.CTkFrame(self.frameGauche, fg_color=back_color)
        self.frameGauche_Haut.grid(row=0, column=0, padx=10, pady=10)

        self.frameGauche_Bas = customtkinter.CTkFrame(self.frameGauche, fg_color=back_color)
        self.frameGauche_Bas.grid(row=1, column=0, padx=10, pady=10)

        # Frame Gauche Haut (Choix des modes)
        self.labelMode = customtkinter.CTkLabel(master=self.frameGauche_Haut, font=("Helvetica", 16), text="Mode : ")

        #check_var_Manuel = customtkinter.StringVar(value="Off")
        self.checkManuel = customtkinter.CTkCheckBox(master=self.frameGauche_Haut, text="Manuel", onvalue=1, offvalue=2,
                                                     border_color="White",
                                                     hover_color="White", fg_color="White")

        self.boutonStatique = customtkinter.CTkButton(master=self.frameGauche_Haut, text="Statique", command=self.commandeStatique)
        self.boutonAntigrav = customtkinter.CTkButton(master=self.frameGauche_Haut, text="Anti-gravité", command=self.commandeAntigrav)
        self.boutonCurl = customtkinter.CTkButton(master=self.frameGauche_Haut, text="Curl", command=self.commandeCurl)

        #check_var_Anti = customtkinter.StringVar(value="Off")
        #self.checkAntigrav = customtkinter.CTkCheckBox(master=self.frameGauche_Haut, text="Anti-Gravité", onvalue=1, offvalue=0, border_color="White", hover_color="White", fg_color="White")

        #check_var_Stat = customtkinter.StringVar(value="Off")
        #self.checkStat = customtkinter.CTkCheckBox(master=self.frameGauche_Haut, text="Statique", onvalue=1, offvalue=0,
        #                                            border_color="White",
        #                                           hover_color="White", fg_color="White")

        self.boutonCalib = customtkinter.CTkButton(master=self.frameGauche_Haut, text="Calibration", command=self.commandeCalibration)

        self.labelMode.grid(row=0, column=0, padx=10, pady=10)
        self.checkManuel.grid(row=5, column=0, padx=7, pady=7)
        self.boutonCalib.grid(row=2,column=0,padx=5,pady=5)
        self.boutonAntigrav.grid(row=3, column=0, padx=5, pady=5)
        self.boutonCurl.grid(row=4, column=0, padx=5, pady=5)
        self.boutonStatique.grid(row=1, column=0, padx=5, pady=5)


        # Frame Gauche Bas (Envoi de commande)
        self.labelEnvoieCommande = customtkinter.CTkLabel(master=self.frameGauche_Bas, font=("Helvetica", 16), text="Envoie de commandes aux moteurs : ", pady=20)

        self.labelPoignetCommande = customtkinter.CTkLabel(master=self.frameGauche_Bas, font=("Helvetica", 16), text="Poignet :")
        self.labelCoudeCommande = customtkinter.CTkLabel(master=self.frameGauche_Bas, font=("Helvetica", 16),
                                                           text="Coude :")
        self.labelEpauleCommande = customtkinter.CTkLabel(master=self.frameGauche_Bas, font=("Helvetica", 16),
                                                           text="Épaule :")

        self.valeurPoignetEntry = customtkinter.CTkEntry(master=self.frameGauche_Bas, width=75, fg_color="White",
                                                         text_color="Black")
        self.valeurCoudeEntry = customtkinter.CTkEntry(master=self.frameGauche_Bas, width=75, fg_color="White",
                                                       text_color="Black")
        self.valeurEpauleEntry = customtkinter.CTkEntry(master=self.frameGauche_Bas, width=75, fg_color="White",
                                                        text_color="Black")

        self.boutonMoteur = customtkinter.CTkButton(master=self.frameGauche_Bas, text="Envoie Commande",  width=100, command=self.commandeBouton )

        self.labelEnvoieCommande.grid(row=0, column=0, columnspan=4)

        self.boutonMoteur.grid(row=2, column=3)

        self.labelPoignetCommande.grid(row=1, column=0, padx=5)
        self.labelCoudeCommande.grid(row=1, column=1, padx=5)
        self.labelEpauleCommande.grid(row=1, column=2, padx=5)

        self.valeurPoignetEntry.grid(row=2, column=0, padx=5)
        self.valeurCoudeEntry.grid(row=2, column=1, padx=5)
        self.valeurEpauleEntry.grid(row=2, column=2, padx=5)

        self.frameDroit_Haut = customtkinter.CTkFrame(self.frameDroit, fg_color=back_color)
        self.frameDroit_Haut.grid(row=0, column=0, padx=5, pady=5)

        self.frameDroit_bas = customtkinter.CTkFrame(self.frameDroit, fg_color=back_color)
        self.frameDroit_bas.grid(row=1, column=0, padx=5, pady=5)

        ## Frame Droit

        # Frame Droit Haut (Affichage des valeurs en temps réel)
        self.labelAngle = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="Angle")
        self.labelCouple = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="Couple")
        self.labelTemps = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="Temps")

        self.valeurLabelPoignet = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16),
                                                         text="Poignet : ")
        self.valeurLabelCoude = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16),
                                                         text="Coude : ")
        self.valeurLabelEpaule = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16),
                                                         text="Epaule : ")

        self.valeurAnglePoignet = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")
        self.valeurAngleCoude = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")
        self.valeurAngleEpaule = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")

        self.valeurCouplePoignet = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")
        self.valeurCoupleCoude = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")
        self.valeurCoupleEpaule = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0")

        self.valeurTemps = customtkinter.CTkLabel(self.frameDroit_Haut, font=("Helvetica", 16), text="0", pady=20)

        self.labelAngle.grid(row=0, column=1, padx=8)
        self.labelCouple.grid(row=0, column=2, padx=8)

        self.valeurLabelPoignet.grid(row=1, column=0, padx=8)
        self.valeurAnglePoignet.grid(row=1, column=1, padx=8)
        self.valeurCouplePoignet.grid(row=1, column=2, padx=8)

        self.valeurLabelCoude.grid(row=2, column=0, padx=10)
        self.valeurAngleCoude.grid(row=2, column=1, padx=10)
        self.valeurCoupleCoude.grid(row=2, column=2, padx=10)

        self.valeurLabelEpaule.grid(row=3, column=0, padx=10)
        self.valeurAngleEpaule.grid(row=3, column=1, padx=10)
        self.valeurCoupleEpaule.grid(row=3, column=2, padx=10)

        self.labelTemps.grid(row=4, column=0)
        self.valeurTemps.grid(row=4, column=1)

        # Frame Droit Bas (Bouton d'arrêt)
        self.boutonArret = customtkinter.CTkButton(self.frameDroit_bas, text="Bouton d'arrêt", font=("Helvetica", 12),command=self.commandeArret,
                                                   width=120, height=120, corner_radius=150,
                                                   fg_color="red2", hover_color="DarkRed")

        self.boutonArret.grid(row=0,column=0)

        threadUpdate = threading.Thread(target=self.set_valeur).start()


    def set_valeur(self):
        global temps
        global angleEpaule
        global angleCoude
        global anglePoignet
        global torqueEpaule
        global torqueCoude
        global torquePoignet
        global mode_moteur

        while 1:
            self.valeurAnglePoignet.configure(text=str(anglePoignet))
            self.valeurAngleCoude.configure(text=str(angleCoude))
            self.valeurAngleEpaule.configure(text=str(angleEpaule))
            self.valeurCouplePoignet.configure(text=str(torquePoignet))
            self.valeurCoupleCoude.configure(text=str(torqueCoude))
            self.valeurCoupleEpaule.configure(text=str(torqueEpaule))
            self.valeurTemps.configure(text=str(temps))
 

    def commandeArret(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer

        self.checkManuel.deselect()
        commandeMoteur = "0,0,0,0"
        commandeAEnvoyer = True
        ancienneCommandeMoteur = commandeMoteur

    def commandeAntigrav(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer

        self.checkManuel.deselect()
        commandeMoteur = "2,0,0,0"
        commandeAEnvoyer = True
        ancienneCommandeMoteur = commandeMoteur


    def commandeCalibration(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer

        self.checkManuel.deselect()
        commandeMoteur = "3,0,0,0"
        commandeAEnvoyer = True
        ancienneCommandeMoteur = commandeMoteur

    def commandeStatique(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer

        self.checkManuel.deselect()
        commandeMoteur = "4,0,0,0"
        commandeAEnvoyer = True
        ancienneCommandeMoteur = commandeMoteur

    def commandeCurl(self):
        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer

        self.checkManuel.deselect()
        commandeMoteur = "5,0,0,0"
        commandeAEnvoyer = True
        ancienneCommandeMoteur = commandeMoteur


    def commandeBouton(self):

        global commandeMoteur
        global ancienneCommandeMoteur
        global commandeAEnvoyer
        with lock:

            commandeAEnvoyer = True
            commandePoignetMoteur = self.valeurPoignetEntry.get()
            if commandePoignetMoteur == "":
                commandePoignetMoteur = 0
            commandeCoudeMoteur = self.valeurCoudeEntry.get()
            if commandeCoudeMoteur == "":
                commandeCoudeMoteur = 0
            commandeEpauleMoteur = self.valeurEpauleEntry.get()
            if commandeEpauleMoteur == "":
                commandeEpauleMoteur = 0
            commandeMoteur = "1" + "," + str(commandePoignetMoteur) + "," + str(commandeCoudeMoteur)+ "," + str(commandeEpauleMoteur)
            ancienneCommandeMoteur = commandeMoteur

#Permet de lançer le graph dans un autre thread
def Graph():
    app = QApplication(sys.argv)
    window = RealTimePlot()
    window.show()
    sys.exit(app.exec_())

def gestionPortSerie():
    global temps
    global angleCoude
    global angleEpaule
    global anglePoignet
    global torqueCoude
    global torqueEpaule
    global torquePoignet

    global commandeAEnvoyer

    objet = gestionPort()

    while 1:
        with lock:
            stringAngle = objet.lireValeurs()

            temps = float(stringAngle[0])
            anglePoignet = float(stringAngle[1])
            angleCoude = float(stringAngle[2])
            angleEpaule = float(stringAngle[3])
            torqueEpaule = float(stringAngle[4])
            torqueCoude = float(stringAngle[5])
            torquePoignet = float(stringAngle[6])


        if (commandeAEnvoyer == True):
            objet.envoieCommande()
            commandeAEnvoyer = False

        time.sleep(0.1)
        #print(stringAngle)
        #print(mode_moteur)

if __name__ == "__main__":
    app = Application()
    threadGraph = threading.Thread(target=Graph).start()

    threadLireAngle = threading.Thread(target=gestionPortSerie).start()
    app.mainloop()
