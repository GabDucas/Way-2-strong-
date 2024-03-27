Way Too Strong

Bienvenue dans le GitHub de l'équipe Way Too Strong, composé de 6 futur ingénieurs robotique étudiant à l'université de Sherbrooke. Ce projet est dans la cadre de la session 4 de génie robotique. Il a pour but de créer un robot possèdant au minimum 3 joints. Les membres ont donc décider de faire un exosquelette d'un modèle réduit d'un bras.

Installation :

Pour installer le projet, la première étape à faire est l'impression 3D des différentes pièces. Pour l'assemblage des pièces, il est relativement facile. Il suffit de regarder l'assemblage SolidWork. Dans notre assemblage, le bois a été utilisé par soucis de temps et d'argent, mais il serait possible de faire des rails pour le rendre ajustable. 

Une fois le projet imprimé, il faut seulement brancher l'openCR (qui agit comme un Arduino) et de télécharger le code. Une fois le code téléchargé, deux fenêtres devraient s'ouvrir. La première fenêtre montre en temps réel la position du bras. La deuxième est la fenêtre de commande et montre quelques informations. Les informations affichées sont la valeur d'angles et de couples de chaques moteurs ainsi que le temps depuis le départ de l'openCR.

Liste des commandes :

    Manuel -> Le mode manuel permet de contrôlé les moteurs en angle (degré). Il est possible de bouger un joint ou plusieurs joints. Automatiquement, si un moteur ne reçoit pas de commande, l'angle est à 0.
    
    Antigravité -> Le mode antigravité évite à l'exosquelette de s'affaiser lorsqu'il n'y a pas de commandes envoyées au moteur. 
    
    Statique -> Le mode statique, s'il est couplé par exemple au mode antigravité, permet au robot, s'il est bougé, de revenir à la position enregistré lorsque le bouton statique est activé. 
    
    Calibration -> Permet de déterminer lorsque le bras est à sa pire position de voir les limites de PWM des trois moteurs. C'est la calibration de l'antigravité et don il faut que l'antigravité soit activée. La calibration se fait à chaque fois que le mode antigravité est commencé, mais il est possbile de la refaire.

Libraries à télécharger :


Auteurs :

Gabriel Aubut, Antoine Costa, Gabriel Ducas, Félix Duguay, Simon Lamontagne et Anaïs Mireault 




