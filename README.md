# Way 2 Strong

Bienvenue dans le GitHub de l'équipe Way 2 Strong, composé de 6 futurs ingénieurs et qui étudient en ce moment en génie robotique à l'université de Sherbrooke. Ce projet est dans le cadre de la session 4 de génie robotique. Il a pour but de créer un robot possédant au minimum 3 joints. Les membres ont donc décidé de faire un exosquelette d'un modèle réduit d'un bras.

## Installation

Pour l'installation du projet, il est possible d'aller voir la page Wiki du projet. Il sera possible de suivre les instructions d'assemblage et les différentes fonctionnalités du programme. Voici le lien pour rejoindre le Wiki : https://github.com/GabDucas/Way-2-strong-.wiki.git 

## Brève description

Ce projet est donc un modèle réduit d'un exosquelette. Il possède un bras imprimé avec une imprimante 3D, un faux-bras en bois et PLA, ainsi qu'une interface graphique qui montre en temps réel l'emplacement du bras ainsi que différentes valeurs d'angles et de couple moteur. 

Le bras possède en tout 3 moteurs, soit pour l'épaule, le coude et le poignet. Ils peuvent être contrôlés par plusieurs modes de contrôle :

    Manuel -> Le mode manuel permet de contrôler les moteurs en angle (degré). Il est possible de bouger un ou plusieurs joints. Automatiquement, si un moteur ne reçoit pas de commande, l'angle est à 0.
    
    Antigravité -> Le mode antigravité évite à l'exosquelette de s'affaisser lorsqu'il n'y a pas de commandes envoyées au moteur. 
    
    Statique -> Le mode statique, s'il est couplé par exemple au mode antigravité, permet au robot, s'il est bougé, de revenir à la position enregistrée lorsque le bouton statique est activé. 
    
    Calibration -> Permet de déterminer lorsque le bras est à sa pire position de voir les limites de PWM des trois moteurs. À chaque démarrage, il faut que la calibration soit faite. Elle peut être refaite plus tard.

Libraries à télécharger :

- Sur Python:
      - Time,
        customtkinter,
        Serial,
        threading,
        Numpy,
        sys,
        pyqt5,
        pyqtgraph,
        collections,

- Sur Arduino:
      - RTOS.h,
        dynamixel2arduino.h

Auteurs :

Gabriel Aubut, Antoine Costa, Gabriel Ducas, Félix Duguay, Simon Lamontagne et Anaïs Mireault 



