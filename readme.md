# Modélisateur de Propagation

Le Modélisateur de Propagation est une application C++ qui simule et visualise la propagation des signaux radio dans un environnement 2D, en tenant compte de divers obstacles.

Il est particulièrement utile pour planifier des réseaux Wi-Fi et comprendre comment les signaux interagissent avec différents types d'obstacles. 

Mais peut être également utilisé pour d'autres types de réseau, tant que l'antenne émettrice a une fréquence d'émission supérieure à 100Mhz, donc pour des antennes 4G ou 5G éventuellement, en indoor et outdoor. 

En effet le modèle que nous utilisons se base sur la FSPL (Free Space Path Loss) améliorée pour prendre en compte les obstacles. Des logiciels professionnels comme Remcom Wireless InSite ou
WinProp utilisent cette même méthode.

## Fonctionnalités

### Visualisation du signal

Génère des cartes thermiques montrant la force du signal dans une pièce, ou à l'extérieur

### Émetteurs multiples

Support pour plusieurs émetteurs avec niveaux de puissance configurables

### Modélisation d'obstacles

Différents types d'obstacles avec propriétés d'atténuation personnalisables :

- Murs rectangulaires avec épaisseur configurable

- Murs droits (verticaux/horizontaux)

- Obstacles circulaires

### Interface interactive

Construite avec SDL2 et SDL_ttf pour une expérience graphique interactive.

Chaque point est espacé de 1 cm, il y a un pixel pour un point, ceci est modulable, en changeant le RESOLUTION_FACTOR dans emitter.hpp, c'est un rapport de division.
Il est actuellement à 100, ce qui fait passer les distances du mètre au centimètre, pour rester en mètres par exemple, mettre 1.

Par exemple pour une pièce de 5m x 6m il y a 500 points par 600, puisque la résolution est de 1 cm la fenêtre aura une taille de 500 par 600 pixels.

## Prérequis

### Windows

<b>Make et g++</b>

### Linux

SDL 2 et SDL_ttf g++
installer avec


`sudo apt update
sudo apt install libsdl2-dev
sudo apt install libsdl2-ttf-dev`

## Compilation et exécution

Cloner le dépôt : `git clone https://github.com/Electromagnetic-Wave-Modeler/Propagation-Modeler`

S'assurer que les bibliothèques SDL2 et SDL2_ttf sont installées
Compiler et exécuter le projet avec le makefile :
`make` sous windows ou
`make linux`

La dernière version est la branche main.

## Utilisation

Ajouter des émetteurs : Créer des sources de signal avec des niveaux de puissance personnalisés. Vous pouvez placer des obstacles ou des sources en modifiant le code de main.cpp ou en ajoutant des murs via le bouton "ADD WALL".

Exporter les données : Sauvegarder les résultats de simulation pour une analyse ultérieure via la fonction ExportToCSV().

Obtenir les valeurs de puissance : cliquez sur un point de la pièce simulée, la valeur de puissance sera affichée en bas à droite.

Changer la position d'un émetteur : cliquer sur une source, le prochain endroit où vous cliquerez fixera la nouvelle position de la source !


## Structure du projet

- main.cpp Point d'entrée de l'application
- headers/ - Fichiers d'en-tête pour toutes les classes du projet
- src/ - Fichiers d'implémentation
- assets/ - Polices et autres ressources
- lib/ - Bibliothèques externes (SDL2)
