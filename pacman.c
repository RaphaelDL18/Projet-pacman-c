#include <stdlib.h>
#include <stdio.h>
#include "libTableauNoir.h"

//Définition des constantes 

#define VITESSE 93
#define DUREE_GUM 9
#define DUREE_FRUIT 10
#define INTERVALLE 0.33
#define PI 3.1415 
#define DT_SPRITES 0.15

//Définition des types 

typedef struct
{
	double x;
	double y;
}coord_t;

typedef struct
{
	tStylo pacman;
	tStylo obstacle;
	tStylo dot;
	tStylo pacgum;
	tStylo porte_maison;
}stylos_t;

typedef struct
{
	char tab[27][21];
}modele_plateau_t;

typedef struct
{
	coord_t pos;
	/*******pos_case*****
	Contient la position de la case sur le plateau
	********************/
	coord_t pos_case;
	/***dot**************
	1 si la case contient un dot, 0 sinon.
	********************/
	int dot;
	/***pacgum***********
	1 si la case contient une pacgum, 0 sinon.
	********************/
	int pacgum;
	/***mur**************
	1 si la case est un mur, 0 sinon.
	********************/
	int mur;
}case_t;

typedef struct
{
	coord_t pos;
	/***comportement*****
	Chaque objet possède son propre comportement, désigné par un code:
	0=Pacman: contrôlé par le joueur
	1=Blinky(=fantôme rouge): Suit le joueur
	2=Pinky(=fantôme rose): Tend des embuscades au joueur;
	3=Inky(=fantôme bleu): Adopte le comportement d'un des 3 autres fantômes au hasard,
	                       en changeant de cible à intervalle régulier
	4=Clyde(=fantôme orange): Complètement aléatoire
	********************/
	int comportement;
	/****direction******
	contient un char indiquant la direction dans laquelle va l'objet
	'g'=gauche
	'h'=haut
	'd'=droite
	'b'=bas
	'n'=neutre
	********************/
	char direction;
	/*******pos_case*****
	Contient l'indice de la case du plateau à laquelle se situe l'objet.
	********************/
	coord_t pos_case;
	/***sur_case*********
	vaut 1 si l'objet est sur une case (et non entre deux cases), 0 sinon.
	********************/
	int sur_case;
	/***compteur_dots****
	(Utile que pour les fantômes) 
	Nombre de dots qu'a mangé pacman depuis sa dernière mort
	ou le début du niveau.Les fantômes sortent de
	la maison des fantômes au bout d'un certain nombre de
	dots mangées:
	-Blinky:Déjà dehors
	-Pinky:0
	-Inky:30
	-Clyde:60
	********************/
	int compteur_dots;
	/***sortie***********
	(Utile que pour les fantômes) 	
	vaut 1 si le fantôme peut quitter la pièce centrale, 0 sinon.
	********************/
	int sortie;
	/******compteur_temps
	(Utile que pour les fantômes)
	Si ce compteur conntient une valeur>3 et que le fantôme se dirige vers une
	cible aléatoire (Clyde en mode "Chase" ou n'importe quel fantôme en mode 
	"Frightened"), alors le faantôme change de cible.
	********************/
	double compteur_temps;
	/******compteur_inky*
	(Utile seulement pour Inky, le fantôme bleu)
	Si ce compteur>5, alors Inky copie le comportement d'un autre fantôme.
	********************/
	double compteur_inky;
	/***modele_inky******
	(Utile seulement pour Inky, le fantôme bleu)
	Contient la valeur correspondant au fantôme que copie Inky, c'est-à-dire:
	-0=Blinky
	-1=Pinky
	-2=Clyde
	********************/
	int modele_inky;
	/*******cible_alea***
	(Utile que pour les fantômes)
	Contient les coordonées du sommet ciblé si le fantômee se déplace aléatoirement.
	Permet, lorsque les fantômes se déplacent aléatoirement, qu'ils bougent vers un même sommet pendant 
	3 secondes.
	********************/
	coord_t cible_alea;
	/***mort*************
	(Utile que pour les fantômes)
	Vaut 1 si le fantôme est mort, 0 sinon.
	********************/
	int mort;
	/***alt_sprites******
	Permet l'animation des sprites des personnages
	********************/
	int alt_sprites;
	double compteur_alt_sprites;
}mobile_t;

typedef struct
{
	/*******pos**********
	Contient la position du sommet sur le plateau de jeu
	********************/
	coord_t pos;
	/****direction_a_prendre*
	Contient un char indiquant dans quelle direction l'objet doit se rendre
	pour atteindre la destination souhaitée.
	********************/
	char direction_a_prendre;
	int marque;
	/***pos_voisins******
	Tableau contenant la position relative des voisins de notre sommet
	indice 0=Bas
	indice 1=Gauche
	indice 2=Haut
	indice 3=Droite
	Vaut 0 s'il n'y a pas de voisins à la position correspondante, 1 sinon.
	********************/
	int pos_voisins[4];
	/******voisins*******
	Contient les cases voisines de notre sommet.
	NB: Les cases doivent être au même indice que dans pos_voisins. Par
	exemple, si la case[1][1] se situe à gauche de notre sommet, il
	se trouvera à l'indice 1 de pos_voisins et de voisins.
	********************/
	case_t voisins[4];
	int est_visite;
	/***depart***********
	Contient 1 si le sommet est choisi comme point de départ de 
	l'algorithme de Dijkstra, 0 sinon.
	********************/
	int depart;
	/***interdit*********
	Contient 0 si un objet peut se trouver sur cette case, 1 sinon.
	********************/
	int interdit;
}sommet_t;

typedef struct 
{
	sommet_t tab[27][21];
}graphe_t;

typedef struct
{
	int tab[3][8];
}alt_t;

typedef struct
{
	/***gain*************
	Nombre de points gagné lorsque l'on mange le fruit 
	cerises=100pts
	fraise=300pts
	orange=500pts
	pomme=700pts
	chou=1000pts
	vaisseau galaxien=2000pts
	cloche=3000pts
	clé=5000pts
	********************/
	int gain;
	/***affiche**********
	1 si le fruit est affiché, 0 sinon
	********************/
	int affiche;
	double compteur_fruit;
}fruit_t;

typedef struct
{
	mobile_t objets[5];
	/******plateau*******
	tableau contenant l'ensemble des cases composant le jeu pacman
	********************/
	case_t plateau[27][21];
	stylos_t stylos;
	/***etat_fantomes****
	Indique le mode de déplacement des fantômes. Vaut:
	-0 lorsque les fantômes sont en mode "Chase". Ils se 
	 déplacent alors normalement dans le labyrinthe
	-1 lorsque les fantômes sont en mode "Frightened".
	 Ils se déplacent alors aléatoirement dans le 
	 labyrinthe
	-2 lorsque les fantômes sont en mode "Scatter". Ils 
	 se déplacent alors chacun vers un coin du labyrinthe.
	********************/
	int etat_fantomes;
	/********graphe******
	Contient le graphe utilisé pour l'algorithme de Dijkstra
	********************/
	graphe_t graphe;
	/***nb_fantomes_manges
	nombre de fantômes mangés durant l'effet d'une gum
	********************/
	int nb_fantomes_manges;
	/***nbr_dots_manges*
	Nombre de dots qu'a mangé pacman.Une fois que le 
	compteur a atteint 244, la partie se termine
	********************/
	int nbr_dots_manges;
	/******duree_gum*****
	Contient le temps qui s'est écoulé depuis que le joueur 
	à manger une gum
	********************/
	double duree_gum;
	int vies;
	int score;
	int niveau;
	/***indice_alternance
	Contient l'indice dans alternance_chase_scatter correspondant au mode
	de comportement actuel des fantômes
	********************/
	int indice_alternance;
	double compteur_alternance;
	fruit_t fruits[8];
	/***num_fruit********
	Contient l'indice dans le tableau fruits du fruit courant.
	********************/
	int num_fruit;
	/*****images*********
	Contient l'ensemble des images à afficher dans le jeu
	0->9:numeros
	10->17:gain points fruits, par ordre croissant
	18->21:gain points kill fantômes
	22/23:sprites blinky bas
	24/25:sprites blinky droite
	26/27:sprites blinky gauche
	28/29:sprites blinky haut
	30/31:sprites pinky bas
	32/33:sprites pinky droite
	34/35:sprites pinky gauche
	36/37:sprites pinky haut
	38/39:sprites inky bas
	40/41:sprites inky droite
	42/43:sprites inky gauche
	44/45:sprites inky haut
	46/47:sprites clyde bas
	48/49:sprites clyde droite
	50/51:sprites clyde gauche
	52/53:sprites clyde haut
	54/64:sprites animation pacman mort
	65/68:sprites fantômes morts (bas-droite-gauche-haut)
	69/76:fruits par ordre croissant de gain
	77/78:sprites fantômes effrayés bleus
	79/80:sprites fantômes effrayés blancs
	81/82:pacman/pacman grand
	83:ready
	84:game over
	85:touches
	********************/
	Image images[86];
	double compteur_affichage_gain_fruit;
	/******compteur_score_vies
	Permet d'ajouter une vie au joueur quand il dépasse les 10 000 points
	********************/
	int compteur_score_vies;
	int fin;
}jeu_t;

//Prototypes des fonctions

//fonctions d'initialisation
stylos_t init_stylo(void);
void init_plateau_graphe(jeu_t*,modele_plateau_t);
mobile_t init_objet(jeu_t,int,int,int,int);
fruit_t init_fruit(int);
void init_images(jeu_t*);
jeu_t init_jeu(modele_plateau_t);
void init_terrain(jeu_t);
//Fonctions d'affichage
Image choix_image_fantome(jeu_t,int);
void affichage_niveau(jeu_t);
void affichage_score(jeu_t);
void affichage_gain(jeu_t);
void afficher(jeu_t);
void anim_mort(jeu_t);
//Fonctions utiles pour l'IA des fantômes
int tous_visites(jeu_t);
sommet_t plus_petite_marque_non_select(jeu_t);
void init_direction_dijkstra(jeu_t*,sommet_t);
char dijkstra(jeu_t,sommet_t,sommet_t);
void deplacement_objets(jeu_t*,modele_plateau_t,double);
sommet_t cible_probable(jeu_t,modele_plateau_t);
char ia_rouge(jeu_t*,modele_plateau_t,int,double);
char ia_rose(jeu_t*,modele_plateau_t,int,double);
char ia_bleu(jeu_t*,modele_plateau_t,double);
char ia_orange(jeu_t*,modele_plateau_t,int,double);
//Fonctions de mise à jour
void maj_compteurs_dots(jeu_t*);
void gestion_gum(jeu_t*,double);
void maj_compteurs_temps(jeu_t*,double);
void reset_mort(jeu_t*);
void reset_niveau(jeu_t*,modele_plateau_t);
void collisions(jeu_t*,double);
void fin_jeu(jeu_t*);
void respawn_fantomes(jeu_t*);
void maj_alternance(jeu_t*,alt_t);
void maj_fruits(jeu_t*,double);
void maj_sprites(jeu_t*);
void mettre_a_jour(jeu_t*,modele_plateau_t,alt_t);

//Fonction Principale

int main(void)
{
	jeu_t jeu;
	Image temp;
	/****modele_plateau**
        Modélisation du plateau de jeu par un tableau par un tableau de str
        Légende:
        x=bordure/obstacle
        o=case vide
        i=case avec un dot
        p=case avec une pacgum
        ********************/
        modele_plateau_t modele_plateau={.tab={
        					"xxxxxxxxxxxxxxxxxxxxx",
        					"xiiiiiiiiixiiiiiiiiix",
        					"xixxxixxxixixxxixxxix",
        					"xpxoxixoxixixoxixoxpx",
        					"xixxxixxxixixxxixxxix",
        					"xiiiiiiiiiiiiiiiiiiix",
        					"xixxxixixxxxxixixxxix",
        					"xixxxixixxxxxixixxxix",
        					"xiiiiixiiixiiixiiiiix",
        					"xxxxxixxxoxoxxxixxxxx",
        					"ooooxixoooooooxixoooo",
        					"ooooxixoxxxxxoxixoooo",
        					"xxxxxixoxoooxoxixxxxx",
        					"oooooiooxoooxooiooooo",
        					"xxxxxixoxxxxxoxixxxxx",
        					"ooooxixoooooooxixoooo",
        					"ooooxixoxxxxxoxixoooo",
        					"xxxxxixoxxxxxoxixxxxx",
        					"xiiiiiiiiixiiiiiiiiix",
        					"xixxxixxxixixxxixxxix",
        					"xpiixiiiiiiiiiiixiipx",
        					"xxxixixixxxxxixixixxx",
        					"xxxixixixxxxxixixixxx",
        					"xiiiiixiiixiiixiiiiix",
        					"xixxxxxxxixixxxxxxxix",
        					"xiiiiiiiiiiiiiiiiiiix",
        					"xxxxxxxxxxxxxxxxxxxxx"}
        				};
        alt_t alt_chase_scatter={.tab={
        					{7,20,7,20,5,20,5,9999},
        					{7,20,7,20,5,1033,1,9999},
        					{5,20,5,20,5,1037,1,9999}
        				     }
        				};
	creerTableau();
        fixerModeBufferisation(1);
        fixerTaille(1500,1000);
        jeu = init_jeu(modele_plateau);
        init_terrain(jeu);
        temp=rotozoomImage(jeu.images[83],0,2,2);
        afficherImage(temp,jeu.plateau[16][10].pos.x-tn_largeur(temp)/2,jeu.plateau[16][10].pos.y+tn_largeur(temp)/2);
        temp=rotozoomImage(jeu.images[85],0,0.5,0.5);
	afficherImage(temp,jeu.plateau[26][20].pos.x+200-tn_largeur(temp)/2,jeu.plateau[26][20].pos.y+tn_largeur(temp)/2);
        tamponner();
        attendreNms(1500);
        //Boucle d'animation
       	while(! jeu.fin)
       	{
       		afficher(jeu);
       		tamponner();
        	effacerTableau();
       		mettre_a_jour(&jeu,modele_plateau,alt_chase_scatter);
       	}
	return EXIT_SUCCESS;
}

//Définition des fonctions

stylos_t init_stylo(void)
{
	stylos_t stylos;
	choisirTypeStylo(27,255,247,0);
	stylos.pacman=stockerStylo();
	choisirTypeStylo(5,0,0,255);
	stylos.obstacle=stockerStylo();
	choisirTypeStylo(5,255,179,0);
	stylos.dot=stockerStylo();
	choisirTypeStylo(10,255,179,0);
	stylos.pacgum=stockerStylo();
	choisirTypeStylo(3,255,153,255);
	stylos.porte_maison=stockerStylo();
	return stylos;	
};

void init_plateau_graphe(jeu_t * jeu, modele_plateau_t mod)
{
	int i,j;
	case_t case_0={pos:{0,0},pos_case:{0,0},dot:0,pacgum:0,mur:0};
	sommet_t sommet_0={pos:{0,0},marque:999,pos_voisins:{0,0,0,0},voisins:{case_0,case_0,case_0,case_0},est_visite:0,direction_a_prendre:'n',depart:0,interdit:0};
	//Initialisation des cases du plateau et de leur position dans la fenêtre graphique
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			//initialisation du plateau
			jeu->plateau[i][j]=case_0;
			jeu->plateau[i][j].pos.x=(-310.0)+31*j;
			jeu->plateau[i][j].pos.y=(420.0)-31*i;
			jeu->plateau[i][j].pos_case.x=i;
			jeu->plateau[i][j].pos_case.y=j;
			if(mod.tab[i][j]=='x') jeu->plateau[i][j].mur=1;
			if(mod.tab[i][j]=='i') jeu->plateau[i][j].dot=1;
			if(mod.tab[i][j]=='p') jeu->plateau[i][j].pacgum=1;
		}
	}
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			//initialisation du graphe
			jeu->graphe.tab[i][j]=sommet_0;
			jeu->graphe.tab[i][j].pos.x=i;
			jeu->graphe.tab[i][j].pos.y=j;
			//On vérifie si le sommet(i,j) possède un voisin au dessus de lui
			if(mod.tab[(int)jeu->graphe.tab[i][j].pos.x-1][(int)jeu->graphe.tab[i][j].pos.y]!='x' && i!=0)
			{
				jeu->graphe.tab[i][j].pos_voisins[2]=1;
				jeu->graphe.tab[i][j].voisins[2]=jeu->plateau[i-1][j];
			}
			//On vérifie si le sommet(i,j) possède un voisin à droite
			if(mod.tab[(int)jeu->graphe.tab[i][j].pos.x][(int)jeu->graphe.tab[i][j].pos.y+1]!='x' && j!=20)
			{
				jeu->graphe.tab[i][j].pos_voisins[3]=1;
				jeu->graphe.tab[i][j].voisins[3]=jeu->plateau[i][j+1];
			}
			//On vérifie si le sommet(i,j) possède un voisin en dessous de lui
			if(mod.tab[(int)jeu->graphe.tab[i][j].pos.x+1][(int)jeu->graphe.tab[i][j].pos.y]!='x' && i!=26)
			{
				jeu->graphe.tab[i][j].pos_voisins[0]=1;
				jeu->graphe.tab[i][j].voisins[0]=jeu->plateau[i+1][j];
			}
			//On vérifie si le sommet(i,j) possède un voisin à gauche
			if(mod.tab[(int)jeu->graphe.tab[i][j].pos.x][(int)jeu->graphe.tab[i][j].pos.y-1]!='x' && j!=0)
			{
				jeu->graphe.tab[i][j].pos_voisins[1]=1;
				jeu->graphe.tab[i][j].voisins[1]=jeu->plateau[i][j-1];
			}
			//On vérifie si le sommet est accessible ou non
			if(mod.tab[i][j]=='x') jeu->graphe.tab[i][j].interdit=1;
		}
	}
	//Cas des raccourcis sur les côtés
	//Raccourci à gauche
	jeu->graphe.tab[13][0].pos_voisins[1]=1;
	jeu->graphe.tab[13][0].voisins[1]=jeu->plateau[13][20];
	//Racoourci à droite
	jeu->graphe.tab[13][20].pos_voisins[3]=1;
	jeu->graphe.tab[13][20].voisins[3]=jeu->plateau[13][0];
	//On marque comme interdit les sommets inaccessibles qui ne sont pas des murs
	jeu->graphe.tab[3][3].interdit=1;
	jeu->graphe.tab[3][7].interdit=1;
	jeu->graphe.tab[3][13].interdit=1;
	jeu->graphe.tab[3][17].interdit=1;
	for(i=0;i<4;i++) jeu->graphe.tab[10][i].interdit=1;
	for(i=0;i<4;i++) jeu->graphe.tab[11][i].interdit=1;
	for(i=17;i<21;i++) jeu->graphe.tab[10][i].interdit=1;
	for(i=17;i<21;i++) jeu->graphe.tab[11][i].interdit=1;
	for(i=0;i<4;i++) jeu->graphe.tab[15][i].interdit=1;
	for(i=0;i<4;i++) jeu->graphe.tab[16][i].interdit=1;
	for(i=17;i<21;i++) jeu->graphe.tab[15][i].interdit=1;
	for(i=17;i<21;i++) jeu->graphe.tab[16][i].interdit=1;
	for(i=9;i<12;i++) jeu->graphe.tab[12][i].interdit=1;
	for(i=9;i<12;i++) jeu->graphe.tab[13][i].interdit=1;	
};

mobile_t init_objet(jeu_t jeu,int indice,int posx,int posy,int sortie)
{
	mobile_t objet={pos:{jeu.plateau[posx][posy].pos.x,jeu.plateau[posx][posy].pos.y},comportement:indice,direction:'n',pos_case:{posx,posy},sur_case:1,compteur_dots:0,sortie:sortie,compteur_temps:0.0,compteur_inky:0.0,modele_inky:0,cible_alea:{10,10},mort:0,alt_sprites:0,compteur_alt_sprites:0.0};
	return objet;
};

fruit_t init_fruit(int gain)
{
	fruit_t fruit={gain:gain,affiche:0,compteur_fruit:0.0};
	return fruit;
};

void init_images(jeu_t* jeu)
{
	jeu->images[0]=chargerImage("../data/0.png");
	jeu->images[1]=chargerImage("../data/1.png");
	jeu->images[2]=chargerImage("../data/2.png");
	jeu->images[3]=chargerImage("../data/3.png");
	jeu->images[4]=chargerImage("../data/4.png");
	jeu->images[5]=chargerImage("../data/5.png");
	jeu->images[6]=chargerImage("../data/6.png");
	jeu->images[7]=chargerImage("../data/7.png");
	jeu->images[8]=chargerImage("../data/8.png");
	jeu->images[9]=chargerImage("../data/9.png");
	jeu->images[10]=chargerImage("../data/100.png");
	jeu->images[11]=chargerImage("../data/300.png");
	jeu->images[12]=chargerImage("../data/500.png");
	jeu->images[13]=chargerImage("../data/700.png");
	jeu->images[14]=chargerImage("../data/1000.png");
	jeu->images[15]=chargerImage("../data/2000.png");
	jeu->images[16]=chargerImage("../data/3000.png");
	jeu->images[17]=chargerImage("../data/5000.png");
	jeu->images[18]=chargerImage("../data/200.png");
	jeu->images[19]=chargerImage("../data/400.png");
	jeu->images[20]=chargerImage("../data/800.png");
	jeu->images[21]=chargerImage("../data/1600.png");
	jeu->images[22]=chargerImage("../data/blinky_bas_1.png");
	jeu->images[23]=chargerImage("../data/blinky_bas_2.png");
	jeu->images[24]=chargerImage("../data/blinky_droite_1.png");
	jeu->images[25]=chargerImage("../data/blinky_droite_2.png");
	jeu->images[26]=chargerImage("../data/blinky_gauche_1.png");
	jeu->images[27]=chargerImage("../data/blinky_gauche_2.png");
	jeu->images[28]=chargerImage("../data/blinky_haut_1.png");
	jeu->images[29]=chargerImage("../data/blinky_haut_2.png");
	jeu->images[30]=chargerImage("../data/pinky_bas_1.png");
	jeu->images[31]=chargerImage("../data/pinky_bas_2.png");
	jeu->images[32]=chargerImage("../data/pinky_droite_1.png");
	jeu->images[33]=chargerImage("../data/pinky_droite_2.png");
	jeu->images[34]=chargerImage("../data/pinky_gauche_1.png");
	jeu->images[35]=chargerImage("../data/pinky_gauche_2.png");
	jeu->images[36]=chargerImage("../data/pinky_haut_1.png");
	jeu->images[37]=chargerImage("../data/pinky_haut_2.png");
	jeu->images[38]=chargerImage("../data/inky_bas_1.png");
	jeu->images[39]=chargerImage("../data/inky_bas_2.png");
	jeu->images[40]=chargerImage("../data/inky_droite_1.png");
	jeu->images[41]=chargerImage("../data/inky_droite_2.png");
	jeu->images[42]=chargerImage("../data/inky_gauche_1.png");
	jeu->images[43]=chargerImage("../data/inky_gauche_2.png");
	jeu->images[44]=chargerImage("../data/inky_haut_1.png");
	jeu->images[45]=chargerImage("../data/inky_haut_2.png");
	jeu->images[46]=chargerImage("../data/clyde_bas_1.png");
	jeu->images[47]=chargerImage("../data/clyde_bas_2.png");
	jeu->images[48]=chargerImage("../data/clyde_droite_1.png");
	jeu->images[49]=chargerImage("../data/clyde_droite_2.png");
	jeu->images[50]=chargerImage("../data/clyde_gauche_1.png");
	jeu->images[51]=chargerImage("../data/clyde_gauche_2.png");
	jeu->images[52]=chargerImage("../data/clyde_haut_1.png");
	jeu->images[53]=chargerImage("../data/clyde_haut_2.png");
	jeu->images[54]=chargerImage("../data/mort1.png");
	jeu->images[55]=chargerImage("../data/mort2.png");
	jeu->images[56]=chargerImage("../data/mort3.png");
	jeu->images[57]=chargerImage("../data/mort4.png");
	jeu->images[58]=chargerImage("../data/mort5.png");
	jeu->images[59]=chargerImage("../data/mort6.png");
	jeu->images[60]=chargerImage("../data/mort7.png");
	jeu->images[61]=chargerImage("../data/mort8.png");
	jeu->images[62]=chargerImage("../data/mort9.png");
	jeu->images[63]=chargerImage("../data/mort10.png");
	jeu->images[64]=chargerImage("../data/mort11.png");
	jeu->images[65]=chargerImage("../data/mort_bas.png");
	jeu->images[66]=chargerImage("../data/mort_droite.png");
	jeu->images[67]=chargerImage("../data/mort_gauche.png");
	jeu->images[68]=chargerImage("../data/mort_haut.png");
	jeu->images[69]=chargerImage("../data/cerises.png");
	jeu->images[70]=chargerImage("../data/fraise.png");
	jeu->images[71]=chargerImage("../data/orange.png");
	jeu->images[72]=chargerImage("../data/pomme.png");
	jeu->images[73]=chargerImage("../data/chou.png");
	jeu->images[74]=chargerImage("../data/galaxien.png");
	jeu->images[75]=chargerImage("../data/cloche.png");
	jeu->images[76]=chargerImage("../data/cle.png");
	jeu->images[77]=chargerImage("../data/frightened_bleu_1.png");
	jeu->images[78]=chargerImage("../data/frightened_bleu_2.png");
	jeu->images[79]=chargerImage("../data/frightened_blanc_1.png");
	jeu->images[80]=chargerImage("../data/frightened_blanc_2.png");
	jeu->images[81]=chargerImage("../data/pacman.png");
	jeu->images[82]=chargerImage("../data/pacman_grand.png");
	jeu->images[83]=chargerImage("../data/ready.png");
	jeu->images[84]=chargerImage("../data/game_over.png");
	jeu->images[85]=chargerImage("../data/zqsd.png");
};

jeu_t init_jeu(modele_plateau_t mod)
{ 
	int i,alea_i,alea_j;
	int gain_fruit[8]={100,300,500,700,1000,2000,3000,5000};
	jeu_t jeu;
	init_plateau_graphe(&jeu,mod);
	//Initialisation de pacman
	jeu.objets[0]=init_objet(jeu,0,15,10,1);
	//Initialisation des fantômes
	//Blinky (rouge)
	jeu.objets[1]=init_objet(jeu,1,10,10,1);
	//Pinky (rose)
	jeu.objets[2]=init_objet(jeu,2,13,10,0);
	//Inky (bleu)
	jeu.objets[3]=init_objet(jeu,3,13,9,0);
	//Clyde (orange)
	jeu.objets[4]=init_objet(jeu,4,13,11,0);
	//Initialisation d'une cible aléatoire pour chaque fantôme
	for(i=1;i<5;i++)
	{
		alea_i=rand()%26;
		alea_j=rand()%20;
		while(jeu.graphe.tab[alea_i][alea_j].interdit)
		{
			alea_i=rand()%26;
			alea_j=rand()%20;
			if(!jeu.graphe.tab[alea_i][alea_j].interdit)
			{
				jeu.objets[i].cible_alea.x=alea_i;
				jeu.objets[i].cible_alea.y=alea_j;	
			}
		}
	}
	//Initialisation des fruits
	for(i=0;i<8;i++) jeu.fruits[i]=init_fruit(gain_fruit[i]);
	//Initialisation des autres variables
	jeu.stylos=init_stylo();
	jeu.etat_fantomes=2;
	jeu.nb_fantomes_manges=0;
	jeu.nbr_dots_manges=0;
	jeu.vies=3;
	jeu.score=0;
	jeu.niveau=1;
	jeu.indice_alternance=0;
	jeu.compteur_alternance=0.0;
	jeu.num_fruit=0;
	jeu.compteur_score_vies=0;
	jeu.compteur_affichage_gain_fruit=0.0;
	jeu.fin=0;
	//Initialisation des images
	init_images(&jeu);
	return jeu;
};

void init_terrain(jeu_t jeu)
{
	//Création des contours du terrain
	selectionnerStylo(jeu.stylos.obstacle);
	tracerSegment(jeu.plateau[0][0].pos.x,jeu.plateau[0][0].pos.y,jeu.plateau[0][20].pos.x,jeu.plateau[0][20].pos.y);
	tracerSegment(jeu.plateau[0][0].pos.x,jeu.plateau[0][0].pos.y,jeu.plateau[9][0].pos.x,jeu.plateau[9][0].pos.y);
	tracerSegment(jeu.plateau[9][0].pos.x,jeu.plateau[9][0].pos.y,jeu.plateau[9][4].pos.x,jeu.plateau[9][4].pos.y);
	tracerSegment(jeu.plateau[9][4].pos.x,jeu.plateau[9][4].pos.y,jeu.plateau[12][4].pos.x,jeu.plateau[12][4].pos.y);
	tracerSegment(jeu.plateau[12][0].pos.x,jeu.plateau[12][0].pos.y,jeu.plateau[12][4].pos.x,jeu.plateau[12][4].pos.y);
	tracerSegment(jeu.plateau[14][0].pos.x,jeu.plateau[14][0].pos.y,jeu.plateau[14][4].pos.x,jeu.plateau[14][4].pos.y);
	tracerSegment(jeu.plateau[14][4].pos.x,jeu.plateau[14][4].pos.y,jeu.plateau[17][4].pos.x,jeu.plateau[17][4].pos.y);
	tracerSegment(jeu.plateau[17][4].pos.x,jeu.plateau[17][4].pos.y,jeu.plateau[17][0].pos.x,jeu.plateau[17][0].pos.y);
	tracerSegment(jeu.plateau[26][0].pos.x,jeu.plateau[26][0].pos.y,jeu.plateau[17][0].pos.x,jeu.plateau[17][0].pos.y);
	tracerSegment(jeu.plateau[21][0].pos.x,jeu.plateau[21][0].pos.y,jeu.plateau[21][2].pos.x,jeu.plateau[21][2].pos.y);
	tracerSegment(jeu.plateau[21][2].pos.x,jeu.plateau[21][2].pos.y,jeu.plateau[22][2].pos.x,jeu.plateau[22][2].pos.y);
	tracerSegment(jeu.plateau[22][0].pos.x,jeu.plateau[22][0].pos.y,jeu.plateau[22][2].pos.x,jeu.plateau[22][2].pos.y);
	tracerSegment(jeu.plateau[26][0].pos.x,jeu.plateau[26][0].pos.y,jeu.plateau[26][20].pos.x,jeu.plateau[26][20].pos.y);
	tracerSegment(jeu.plateau[17][20].pos.x,jeu.plateau[17][20].pos.y,jeu.plateau[26][20].pos.x,jeu.plateau[26][20].pos.y);
	tracerSegment(jeu.plateau[21][20].pos.x,jeu.plateau[21][20].pos.y,jeu.plateau[21][18].pos.x,jeu.plateau[21][18].pos.y);
	tracerSegment(jeu.plateau[21][18].pos.x,jeu.plateau[21][18].pos.y,jeu.plateau[22][18].pos.x,jeu.plateau[22][18].pos.y);
	tracerSegment(jeu.plateau[22][20].pos.x,jeu.plateau[22][20].pos.y,jeu.plateau[22][18].pos.x,jeu.plateau[22][18].pos.y);
	tracerSegment(jeu.plateau[17][20].pos.x,jeu.plateau[17][20].pos.y,jeu.plateau[17][16].pos.x,jeu.plateau[17][16].pos.y);
	tracerSegment(jeu.plateau[14][16].pos.x,jeu.plateau[14][16].pos.y,jeu.plateau[17][16].pos.x,jeu.plateau[17][16].pos.y);
	tracerSegment(jeu.plateau[14][16].pos.x,jeu.plateau[14][16].pos.y,jeu.plateau[14][20].pos.x,jeu.plateau[14][20].pos.y);
	tracerSegment(jeu.plateau[12][20].pos.x,jeu.plateau[12][20].pos.y,jeu.plateau[12][16].pos.x,jeu.plateau[12][16].pos.y);
	tracerSegment(jeu.plateau[9][16].pos.x,jeu.plateau[9][16].pos.y,jeu.plateau[12][16].pos.x,jeu.plateau[12][16].pos.y);
	tracerSegment(jeu.plateau[9][16].pos.x,jeu.plateau[9][16].pos.y,jeu.plateau[9][20].pos.x,jeu.plateau[9][20].pos.y);
	tracerSegment(jeu.plateau[0][20].pos.x,jeu.plateau[0][20].pos.y,jeu.plateau[9][20].pos.x,jeu.plateau[9][20].pos.y);
	//Création des obstacles du terrain
	choisirCouleurPinceau(0,0,0);
	tracerRectangle(jeu.plateau[2][2].pos.x,jeu.plateau[2][2].pos.y,jeu.plateau[4][4].pos.x,jeu.plateau[4][4].pos.y);
	
	tracerRectangle(jeu.plateau[2][6].pos.x,jeu.plateau[2][6].pos.y,jeu.plateau[4][8].pos.x,jeu.plateau[4][8].pos.y);
	
	tracerSegment(jeu.plateau[0][10].pos.x,jeu.plateau[0][10].pos.y,jeu.plateau[4][10].pos.x,jeu.plateau[4][10].pos.y);
	
	tracerRectangle(jeu.plateau[2][12].pos.x,jeu.plateau[2][12].pos.y,jeu.plateau[4][14].pos.x,jeu.plateau[4][14].pos.y);
	
	tracerRectangle(jeu.plateau[2][16].pos.x,jeu.plateau[2][16].pos.y,jeu.plateau[4][18].pos.x,jeu.plateau[4][18].pos.y);
	
	tracerRectangle(jeu.plateau[6][2].pos.x,jeu.plateau[6][2].pos.y,jeu.plateau[7][4].pos.x,jeu.plateau[7][4].pos.y);
	
	tracerSegment(jeu.plateau[6][6].pos.x,jeu.plateau[6][6].pos.y,jeu.plateau[12][6].pos.x,jeu.plateau[12][6].pos.y);
	
	tracerSegment(jeu.plateau[9][6].pos.x,jeu.plateau[9][6].pos.y,jeu.plateau[9][8].pos.x,jeu.plateau[9][8].pos.y);
	
	tracerRectangle(jeu.plateau[6][8].pos.x,jeu.plateau[6][8].pos.y,jeu.plateau[7][12].pos.x,jeu.plateau[7][12].pos.y);
	tracerSegment(jeu.plateau[7][10].pos.x,jeu.plateau[7][10].pos.y,jeu.plateau[9][10].pos.x,jeu.plateau[9][10].pos.y);
	
	tracerSegment(jeu.plateau[6][14].pos.x,jeu.plateau[6][14].pos.y,jeu.plateau[12][14].pos.x,jeu.plateau[12][14].pos.y);
	
	tracerSegment(jeu.plateau[9][14].pos.x,jeu.plateau[9][14].pos.y,jeu.plateau[9][12].pos.x,jeu.plateau[9][12].pos.y);
	
	tracerRectangle(jeu.plateau[6][16].pos.x,jeu.plateau[6][16].pos.y,jeu.plateau[7][18].pos.x,jeu.plateau[7][18].pos.y);
	
	tracerRectangle(jeu.plateau[11][8].pos.x,jeu.plateau[11][8].pos.y,jeu.plateau[14][12].pos.x,jeu.plateau[14][12].pos.y);
	
	tracerSegment(jeu.plateau[14][6].pos.x,jeu.plateau[14][6].pos.y,jeu.plateau[17][6].pos.x,jeu.plateau[17][6].pos.y);
	
	tracerRectangle(jeu.plateau[16][8].pos.x,jeu.plateau[16][8].pos.y,jeu.plateau[17][12].pos.x,jeu.plateau[17][12].pos.y);
	tracerSegment(jeu.plateau[17][10].pos.x,jeu.plateau[17][10].pos.y,jeu.plateau[19][10].pos.x,jeu.plateau[19][10].pos.y);
	
	tracerSegment(jeu.plateau[14][14].pos.x,jeu.plateau[14][14].pos.y,jeu.plateau[17][14].pos.x,jeu.plateau[17][14].pos.y);
	
	tracerSegment(jeu.plateau[19][2].pos.x,jeu.plateau[19][2].pos.y,jeu.plateau[19][4].pos.x,jeu.plateau[19][4].pos.y);
	tracerSegment(jeu.plateau[22][4].pos.x,jeu.plateau[22][4].pos.y,jeu.plateau[19][4].pos.x,jeu.plateau[19][4].pos.y);
	
	tracerSegment(jeu.plateau[19][6].pos.x,jeu.plateau[19][6].pos.y,jeu.plateau[19][8].pos.x,jeu.plateau[19][8].pos.y);
	
	tracerSegment(jeu.plateau[19][12].pos.x,jeu.plateau[19][12].pos.y,jeu.plateau[19][14].pos.x,jeu.plateau[19][14].pos.y);
	
	tracerSegment(jeu.plateau[19][16].pos.x,jeu.plateau[19][16].pos.y,jeu.plateau[19][18].pos.x,jeu.plateau[19][18].pos.y);
	tracerSegment(jeu.plateau[19][16].pos.x,jeu.plateau[19][16].pos.y,jeu.plateau[22][16].pos.x,jeu.plateau[22][16].pos.y);
	
	tracerSegment(jeu.plateau[21][6].pos.x,jeu.plateau[21][6].pos.y,jeu.plateau[24][6].pos.x,jeu.plateau[24][6].pos.y);
	tracerSegment(jeu.plateau[24][8].pos.x,jeu.plateau[24][8].pos.y,jeu.plateau[24][2].pos.x,jeu.plateau[24][2].pos.y);
	
	tracerRectangle(jeu.plateau[21][8].pos.x,jeu.plateau[21][8].pos.y,jeu.plateau[22][12].pos.x,jeu.plateau[22][12].pos.y);
	tracerSegment(jeu.plateau[22][10].pos.x,jeu.plateau[22][10].pos.y,jeu.plateau[24][10].pos.x,jeu.plateau[24][10].pos.y);
	
	tracerSegment(jeu.plateau[21][14].pos.x,jeu.plateau[21][14].pos.y,jeu.plateau[24][14].pos.x,jeu.plateau[24][14].pos.y);
	tracerSegment(jeu.plateau[24][12].pos.x,jeu.plateau[24][12].pos.y,jeu.plateau[24][18].pos.x,jeu.plateau[24][18].pos.y);
};

Image choix_image_fantome(jeu_t jeu, int i)
{
	int premiere_image=0;
	Image temp;
	if(i==1) premiere_image=22;
	else if(i==2) premiere_image=30;
	else if(i==3) premiere_image=38;
	else if(i==4) premiere_image=46;
	if(jeu.objets[i].mort)
	{
		if(jeu.objets[i].direction=='g' || jeu.objets[i].direction=='n')
		{
			temp=rotozoomImage(jeu.images[67],0,2,2);
		}
		else if(jeu.objets[i].direction=='h')
		{
			temp=rotozoomImage(jeu.images[68],0,2,2);
		}
		else if(jeu.objets[i].direction=='b')
		{
			temp=rotozoomImage(jeu.images[65],0,2,2);
		}
		else if(jeu.objets[i].direction=='d')
		{
			temp=rotozoomImage(jeu.images[66],0,2,2);
		}
	}
	else if(jeu.etat_fantomes!=1)
	{
		if(jeu.objets[i].alt_sprites%2==0)
		{
			if(jeu.objets[i].direction=='g' || jeu.objets[i].direction=='n')
			{
				temp=rotozoomImage(jeu.images[premiere_image+4],0,2,2);
			}
			else if(jeu.objets[i].direction=='h')
			{
				temp=rotozoomImage(jeu.images[premiere_image+6],0,2,2);
			}
			else if(jeu.objets[i].direction=='d')
			{
				temp=rotozoomImage(jeu.images[premiere_image+2],0,2,2);
			}
			else if(jeu.objets[i].direction=='b')
			{
				temp=rotozoomImage(jeu.images[premiere_image],0,2,2);
			}
		}
		else
		{
			if(jeu.objets[i].direction=='g' || jeu.objets[i].direction=='n')
			{
				temp=rotozoomImage(jeu.images[premiere_image+5],0,2,2);
			}
			else if(jeu.objets[i].direction=='h')
			{
				temp=rotozoomImage(jeu.images[premiere_image+7],0,2,2);
			}
			else if(jeu.objets[i].direction=='d')
			{
				temp=rotozoomImage(jeu.images[premiere_image+3],0,2,2);
			}
			else if(jeu.objets[i].direction=='b')
			{
				temp=rotozoomImage(jeu.images[premiere_image+1],0,2,2);
			}
		}
	}
	else
	{
		if(jeu.objets[i].alt_sprites%2==0)
		{
			if(jeu.duree_gum>7.0 && jeu.duree_gum<9.0) temp=rotozoomImage(jeu.images[79],0,2,2);
			else temp=rotozoomImage(jeu.images[77],0,2,2);
		}
		else
		{
			if(jeu.duree_gum>7.0 && jeu.duree_gum<9.0) temp=rotozoomImage(jeu.images[80],0,2,2);
			temp=rotozoomImage(jeu.images[78],0,2,2);
		}		
	}
	return temp;
};

void affichage_score(jeu_t jeu)
{
	int i;
	int centaines_mil=jeu.score/100000;
	int dizaines_mil=(jeu.score-centaines_mil*100000)/10000;
	int milliers=(jeu.score-centaines_mil*100000-dizaines_mil*10000)/1000;
	int centaines=(jeu.score-centaines_mil*100000-dizaines_mil*10000-milliers*1000)/100;
	int dizaines=(jeu.score-centaines_mil*100000-dizaines_mil*10000-milliers*1000-centaines*100)/10;
	Image temp;
	for(i=0;i<10;i++)
	{
		if(i==centaines_mil) 
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][20].pos.x+100-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
		}
		if(i==dizaines_mil)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][20].pos.x+125-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
		}
		if(i==milliers)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][20].pos.x+150-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
		}
		if(i==centaines)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][20].pos.x+175-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
		}
		if(i==dizaines)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][20].pos.x+200-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
		}
	}
	temp=rotozoomImage(jeu.images[0],0,2,2);
	afficherImage(temp,jeu.plateau[1][20].pos.x+225-tn_largeur(temp)/2,jeu.plateau[1][20].pos.y+tn_largeur(temp)/2);
};

void affichage_gain(jeu_t jeu)
{
	Image temp;
	if(jeu.nb_fantomes_manges==1)
	{
		temp=rotozoomImage(jeu.images[18],0,2,2);
		afficherImage(temp,jeu.plateau[2][20].pos.x+100-tn_largeur(temp)/2,jeu.plateau[2][20].pos.y+tn_largeur(temp)/2);
	}
	else if(jeu.nb_fantomes_manges==2)
	{
		temp=rotozoomImage(jeu.images[19],0,2,2);
		afficherImage(temp,jeu.plateau[2][20].pos.x+100-tn_largeur(temp)/2,jeu.plateau[2][20].pos.y+tn_largeur(temp)/2);
	}
	else if(jeu.nb_fantomes_manges==3)
	{
		temp=rotozoomImage(jeu.images[20],0,2,2);
		afficherImage(temp,jeu.plateau[2][20].pos.x+100-tn_largeur(temp)/2,jeu.plateau[2][20].pos.y+tn_largeur(temp)/2);
	}
	else if(jeu.nb_fantomes_manges==4)
	{
		temp=rotozoomImage(jeu.images[21],0,2,2);
		afficherImage(temp,jeu.plateau[2][20].pos.x+100-tn_largeur(temp)/2,jeu.plateau[2][20].pos.y+tn_largeur(temp)/2);
	}
};

void affichage_niveau(jeu_t jeu)
{
	int i;
	Image temp;
	int dizaines=jeu.niveau/10;
	int unites=jeu.niveau-dizaines*10;
	for(i=0;i<10;i++)
	{
		if(i==dizaines)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][0].pos.x-125-tn_largeur(temp)/2,jeu.plateau[1][0].pos.y+tn_largeur(temp)/2);
		}
		if(i==unites)
		{
			temp=rotozoomImage(jeu.images[i],0,2,2);
			afficherImage(temp,jeu.plateau[1][0].pos.x-100-tn_largeur(temp)/2,jeu.plateau[1][0].pos.y+tn_largeur(temp)/2);
		}
	}
};

void afficher(jeu_t jeu)
{
	int i,j;
	Image temp;
	Image temp2;
	//affichage du terrain
	init_terrain(jeu);
	//affichage des dots et des gums
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			if(jeu.plateau[i][j].dot)
			{
				selectionnerStylo(jeu.stylos.dot);
				tracerPoint(jeu.plateau[i][j].pos.x,jeu.plateau[i][j].pos.y);
			}
			if(jeu.plateau[i][j].pacgum)
			{
				selectionnerStylo(jeu.stylos.pacgum);
				tracerPoint(jeu.plateau[i][j].pos.x,jeu.plateau[i][j].pos.y);
			}
		}
	}
	//On referme la maison des fantômes
	selectionnerStylo(jeu.stylos.porte_maison);
	tracerSegment(jeu.plateau[11][10].pos.x-15,jeu.plateau[11][10].pos.y,jeu.plateau[11][10].pos.x+15,jeu.plateau[11][10].pos.y);
	//Affichage du joueur 
	selectionnerStylo(jeu.stylos.pacman);
	if(jeu.objets[0].alt_sprites%3==0) tracerPoint(jeu.objets[0].pos.x,jeu.objets[0].pos.y);
	else if(jeu.objets[0].alt_sprites%3==1)
	{
		temp=jeu.images[81];
		if(jeu.objets[0].direction=='g' || jeu.objets[0].direction=='n')
		{
			temp2=rotozoomImage(temp,0,2,2);
		}
		else if(jeu.objets[0].direction=='h')
		{
			temp2=rotozoomImage(temp,(3*PI)/2,2,2);
		}
		else if(jeu.objets[0].direction=='d')
		{
			temp2=rotozoomImage(temp,PI,2,2);
		}
		else if(jeu.objets[0].direction=='b')
		{
			temp2=rotozoomImage(temp,PI/2,2,2);
		}
	}
	else
	{
		temp=jeu.images[82];
		if(jeu.objets[0].direction=='g' || jeu.objets[0].direction=='n')
		{
			temp2=rotozoomImage(temp,0,2,2);
		}
		else if(jeu.objets[0].direction=='h')
		{
			temp2=rotozoomImage(temp,(3*PI)/2,2,2);
		}
		else if(jeu.objets[0].direction=='d')
		{
			temp2=rotozoomImage(temp,PI,2,2);
		}
		else if(jeu.objets[0].direction=='b')
		{
			temp2=rotozoomImage(temp,PI/2,2,2);
		}
	}
	if(jeu.objets[0].alt_sprites%3!=0)afficherImage(temp2,jeu.objets[0].pos.x-tn_largeur(temp2)/2,jeu.objets[0].pos.y+tn_largeur(temp2)/2);
	//Affichage des fantômes
	//Blinky
	temp=rotozoomImage(choix_image_fantome(jeu,1),0,1,1);
	afficherImage(temp,jeu.objets[1].pos.x-tn_largeur(temp)/2,jeu.objets[1].pos.y+tn_largeur(temp)/2);
	//Pinky
	temp=rotozoomImage(choix_image_fantome(jeu,2),0,1,1);
	afficherImage(temp,jeu.objets[2].pos.x-tn_largeur(temp)/2,jeu.objets[2].pos.y+tn_largeur(temp)/2);
	//Inky
	temp=rotozoomImage(choix_image_fantome(jeu,3),0,1,1);
	afficherImage(temp,jeu.objets[3].pos.x-tn_largeur(temp)/2,jeu.objets[3].pos.y+tn_largeur(temp)/2);
	//Clyde
	temp=rotozoomImage(choix_image_fantome(jeu,4),0,1,1);
	afficherImage(temp,jeu.objets[4].pos.x-tn_largeur(temp)/2,jeu.objets[4].pos.y+tn_largeur(temp)/2);
	//Affichage du fruit le cas échéant
	if(jeu.num_fruit<8)
	{
		if(jeu.fruits[jeu.num_fruit].affiche)
		{
			temp=rotozoomImage(jeu.images[69+jeu.num_fruit],0,2,2);
			afficherImage(temp,jeu.plateau[15][10].pos.x-tn_largeur(temp)/2,jeu.plateau[15][10].pos.y+tn_largeur(temp)/2);
		}
	}
	//Affichage des vies restantes
	temp=jeu.images[81];
	temp2=rotozoomImage(temp,PI,2,2);
	for(i=0;i<jeu.vies;i++) afficherImage(temp2,jeu.plateau[26][1+i].pos.x-tn_largeur(temp)/2,jeu.plateau[26][1+i].pos.y-25+tn_largeur(temp)/2);
	//Affichage du fruit suivant
	for(i=jeu.num_fruit;i<8;i++)
	{
		if(!jeu.fruits[i].affiche)
		{
			temp=rotozoomImage(jeu.images[69+i],0,2,2);
			afficherImage(temp,jeu.plateau[26][19].pos.x-tn_largeur(temp)/2,jeu.plateau[26][19].pos.y-25+tn_largeur(temp)/2);
			break;
		}
	}
	//Affichage du score
	affichage_score(jeu);
	//Affichage du gain de score lorsqu'on tue un fantôme
	affichage_gain(jeu);
	//Affichage du gain de score lorsqu'on a mangé un fruit le cas échéant
	if(jeu.compteur_affichage_gain_fruit!=0)
	{
		temp=rotozoomImage(jeu.images[9+jeu.num_fruit],0,2,2);
		afficherImage(temp,jeu.plateau[2][20].pos.x+200-tn_largeur(temp)/2,jeu.plateau[2][20].pos.y+tn_largeur(temp)/2);
	}
	//Affichage du niveau
	affichage_niveau(jeu);
};

sommet_t cible_probable(jeu_t jeu,modele_plateau_t mod)
{
	coord_t cible={x:jeu.objets[0].pos_case.x,y:jeu.objets[0].pos_case.y};
	if(jeu.objets[0].direction=='h')
	{
		cible.x-=4;
		if(cible.x<0) cible.x=0;
		while(jeu.graphe.tab[(int)cible.x][(int)cible.y].interdit) cible.x++;
	}
	if(jeu.objets[0].direction=='d')
	{
		cible.y+=4;
		if(cible.y>20) cible.y=20;
		while(jeu.graphe.tab[(int)cible.x][(int)cible.y].interdit) cible.y--;
	}
	if(jeu.objets[0].direction=='b')
	{
		cible.x+=4;
		if(cible.x>26) cible.x=26;
		while(jeu.graphe.tab[(int)cible.x][(int)cible.y].interdit) cible.x--;
	}
	if(jeu.objets[0].direction=='g')
	{
		cible.y-=4;
		if(cible.y<0) cible.y=0;
		while(jeu.graphe.tab[(int)cible.x][(int)cible.y].interdit) cible.y++;
	}
	//printf("La cible probable du joueur est %d, %d\n",(int)cible.x,(int)cible.y);
	return jeu.graphe.tab[(int)cible.x][(int)cible.y];
};

int tous_visites(jeu_t jeu)
{
	int i,j;
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			if(jeu.graphe.tab[i][j].est_visite==0) return 0;
		}
	}
	return 1;
};

sommet_t plus_petite_marque_non_select(jeu_t jeu)
{
	int i,j;
	int marque_min=999;
	sommet_t sommet_min;
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			if(jeu.graphe.tab[i][j].marque<marque_min && jeu.graphe.tab[i][j].est_visite==0)
			{
				marque_min=jeu.graphe.tab[i][j].marque;
				sommet_min=jeu.graphe.tab[i][j];
			}
		}
	}
	return sommet_min;
};

void init_direction_dijkstra(jeu_t* jeu,sommet_t depart)
{
	int i;
	for(i=0;i<4;i++)
	{
		if(i==0 && jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y].pos_voisins[i] && (int)depart.pos.x!=26)
		{
			jeu->graphe.tab[(int)depart.pos.x+1][(int)depart.pos.y].direction_a_prendre='b';	
		}
		else if(i==1 && jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y].pos_voisins[i] && (int)depart.pos.y!=0)
		{
			jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y-1].direction_a_prendre='g';
		}
		else if(i==2 && jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y].pos_voisins[i] && (int)depart.pos.x!=0)
		{
			jeu->graphe.tab[(int)depart.pos.x-1][(int)depart.pos.y].direction_a_prendre='h';
		}
		else if(i==3 && jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y].pos_voisins[i] && (int)depart.pos.y!=20)
		{
			jeu->graphe.tab[(int)depart.pos.x][(int)depart.pos.y+1].direction_a_prendre='d';
		}
	}
};

char dijkstra(jeu_t jeu,sommet_t depart,sommet_t destination)
{
	int i,nvl_marque;
	sommet_t sommet_min;
	//On attribue au sommet de départ la marque 0
	jeu.graphe.tab[(int)depart.pos.x][(int)depart.pos.y].marque=0;
	jeu.graphe.tab[(int)depart.pos.x][(int)depart.pos.y].direction_a_prendre='n';
	jeu.graphe.tab[(int)depart.pos.x][(int)depart.pos.y].depart=1;
	init_direction_dijkstra(&jeu,jeu.graphe.tab[(int)depart.pos.x][(int)depart.pos.y]);
	while(!tous_visites(jeu))
	{
		sommet_min=plus_petite_marque_non_select(jeu);
		jeu.graphe.tab[(int)sommet_min.pos.x][(int)sommet_min.pos.y].est_visite=1;
		//On arrête la boucle quand la destination est visité
		if(jeu.graphe.tab[(int)destination.pos.x][(int)destination.pos.y].est_visite) break;
		else
		{
			for(i=0;i<4;i++)
			{
				//On s'intéresse aux voisins de sommet_min non-visités
				if(jeu.graphe.tab[(int)sommet_min.pos.x][(int)sommet_min.pos.y].pos_voisins[i] && !jeu.graphe.tab[(int)sommet_min.voisins[i].pos_case.x][(int)sommet_min.voisins[i].pos_case.y].est_visite)
				{
					nvl_marque=sommet_min.marque+1;
					//On vérifie si la nouvelle marque est plus petite que l'ancienne. Si oui, la nouvelle remplace l'ancienne.
					if(nvl_marque<jeu.graphe.tab[(int)sommet_min.voisins[i].pos_case.x][(int)sommet_min.voisins[i].pos_case.y].marque) 
					{
						jeu.graphe.tab[(int)sommet_min.voisins[i].pos_case.x][(int)sommet_min.voisins[i].pos_case.y].marque=nvl_marque;
					}
					//Le sommet étudié prend la direction à prendre de sommet_min
					if(!jeu.graphe.tab[(int)sommet_min.pos.x][(int)sommet_min.pos.y].depart)
					{
						jeu.graphe.tab[(int)sommet_min.voisins[i].pos_case.x][(int)sommet_min.voisins[i].pos_case.y].direction_a_prendre=sommet_min.direction_a_prendre;	
					}
				}
			}
		}
	}
	return jeu.graphe.tab[(int)destination.pos.x][(int)destination.pos.y].direction_a_prendre;
};

char ia_rouge(jeu_t* jeu,modele_plateau_t mod,int num,double dt)
{
	if(jeu->objets[num].sortie)
	{
		if(!jeu->objets[num].mort)
		{
			//Cas du début de la partie ou après que Blinky soit mort
			if(jeu->objets[num].pos_case.x==12 && jeu->objets[num].pos_case.y==10) return 'h';
			else if(jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10) return 'h';
			//Mode "Chase"
			if(jeu->etat_fantomes==0)
			{
				return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[(int)jeu->objets[0].pos_case.x][(int)jeu->objets[0].pos_case.y]);
			}
			//Mode "Frightened"
			else if(jeu->etat_fantomes==1)return ia_orange(jeu,mod,num,dt);
			//Mode "Scatter"
			//La zone scatter de Blinky se situe en haut à droite du plateau. Lorsque celle-ci est atteinte, il tournera autour du bloc situé dans cette zone. 
			else
			{
				if(jeu->objets[num].pos_case.x==1 && jeu->objets[num].pos_case.y>=15 && jeu->objets[num].pos_case.y<19) return 'd';
				else if(jeu->objets[num].pos_case.y==19 && jeu->objets[num].pos_case.x>=1 && jeu->objets[num].pos_case.x<5) return 'b';
				else if(jeu->objets[num].pos_case.x==5 && jeu->objets[num].pos_case.y<=19 && jeu->objets[num].pos_case.y>15) return 'g';
				else if(jeu->objets[num].pos_case.y==15 && jeu->objets[num].pos_case.x<=5 && jeu->objets[num].pos_case.x>1) return 'h';
				else
				{
					return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[1][19]);
				}
			}
		}
		else
		{
			if((jeu->objets[num].pos_case.x==10 && jeu->objets[num].pos_case.y==10) || (jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10)) return 'b';
			else return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[10][10]);
		}
	}
	return 'n';
};

char ia_rose(jeu_t* jeu,modele_plateau_t mod,int num,double dt)
{
	sommet_t cible=cible_probable(*jeu,mod);
	if(jeu->objets[num].sortie)
	{
		if(!jeu->objets[num].mort)
		{
			//Cas du début de la partie ou après que Pinky soit mort
			if(jeu->objets[num].pos_case.x==13 && jeu->objets[num].pos_case.y==10) return 'h';
			else if(jeu->objets[num].pos_case.x==12 && jeu->objets[num].pos_case.y==10) return 'h';
			else if(jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10) return 'h';
			//Mode "Chase"
			if(jeu->etat_fantomes==0)
			{
				return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[(int)cible.pos.x][(int)cible.pos.y]);
			}
			//Mode "Frightened"
			else if(jeu->etat_fantomes==1)
			{
				return ia_orange(jeu,mod,num,dt);
			}
			//Mode "Scatter"
			else
			{
				if(jeu->objets[num].pos_case.x==1 && jeu->objets[num].pos_case.y<=5 && jeu->objets[num].pos_case.y>1) return 'g';
				else if(jeu->objets[num].pos_case.y==1 && jeu->objets[num].pos_case.x>=1 && jeu->objets[num].pos_case.x<5) return 'b';
				else if(jeu->objets[num].pos_case.x==5 && jeu->objets[num].pos_case.y>=1 && jeu->objets[num].pos_case.y<5) return 'd';
				else if(jeu->objets[num].pos_case.y==5 && jeu->objets[num].pos_case.x<=5 && jeu->objets[num].pos_case.x>1) return 'h';
				else
				{
					return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[1][1]);	
				}
			}
		}
		else 
		{
			if((jeu->objets[num].pos_case.x==10 && jeu->objets[num].pos_case.y==10) || (jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10) || (jeu->objets[num].pos_case.x==12 && jeu->objets[num].pos_case.y==10)) return 'b';
			else return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[10][10]);
		}
	}
	return 'n';
};

char ia_orange(jeu_t* jeu,modele_plateau_t mod,int num,double dt)
{
	if(jeu->objets[num].sortie)
	{
		if(!jeu->objets[num].mort)
		{
			//Cas du début de la partie ou après que Clyde soit mort
			if(jeu->objets[num].pos_case.x==13 && jeu->objets[num].pos_case.y==11) return 'g';
			else if(jeu->objets[num].pos_case.x==13 && jeu->objets[num].pos_case.y==10) return 'h';
			else if(jeu->objets[num].pos_case.x==12 && jeu->objets[num].pos_case.y==10) return 'h';
			else if(jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10) return 'h';
			//Mode "Chase" & "Frightened"
			if(jeu->etat_fantomes==0 || jeu->etat_fantomes==1)
			{
				return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[(int)jeu->objets[num].cible_alea.x][(int)jeu->objets[num].cible_alea.y]);
			}
			//Mode "Scatter"
			//La zone Scatter de Clyde se situe en bas à gauche du plateau. Lorsque celle-ci est atteinte, il tournera autour du bloc situé dans cette zone.
			else
			{
				if(jeu->objets[num].pos_case.x==25 && jeu->objets[num].pos_case.y>=1 && jeu->objets[num].pos_case.y<9) return 'd';
				else if(jeu->objets[num].pos_case.y==9 && jeu->objets[num].pos_case.x<=25 && jeu->objets[num].pos_case.x>23) return 'h';
				else if(jeu->objets[num].pos_case.x==23 && jeu->objets[num].pos_case.y<=9 && jeu->objets[num].pos_case.y>7) return 'g';
				else if(jeu->objets[num].pos_case.y==7 && jeu->objets[num].pos_case.x<=23 && jeu->objets[num].pos_case.x>20) return 'h';
				else if(jeu->objets[num].pos_case.x==20 && jeu->objets[num].pos_case.y<=7 && jeu->objets[num].pos_case.y>5) return 'g';
				else if(jeu->objets[num].pos_case.y==5 && jeu->objets[num].pos_case.x>=20 && jeu->objets[num].pos_case.x<23) return 'b';
				else if(jeu->objets[num].pos_case.x==23 && jeu->objets[num].pos_case.y<=5 && jeu->objets[num].pos_case.y>1) return 'g';
				else if(jeu->objets[num].pos_case.y==1 && jeu->objets[num].pos_case.x>=23 && jeu->objets[num].pos_case.x<25) return 'b';
				else
				{
					return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[25][1]);	
				}
			}
		}
		//Retour de Clyde à sa position initiale lorsqu'il est mort
		if((jeu->objets[num].pos_case.x==10 && jeu->objets[num].pos_case.y==10) || (jeu->objets[num].pos_case.x==11 && jeu->objets[num].pos_case.y==10) || (jeu->objets[num].pos_case.x==12 && jeu->objets[num].pos_case.y==10)) return 'b';
		else if(jeu->objets[num].pos_case.x==13 && jeu->objets[num].pos_case.y==10) return 'd';
		else return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[num].pos_case.x][(int)jeu->objets[num].pos_case.y],jeu->graphe.tab[10][10]);
	}
	return 'n';
};

char ia_bleu(jeu_t* jeu,modele_plateau_t mod,double dt)
{
	if(jeu->objets[3].sortie)
	{
		if(!jeu->objets[3].mort)
		{
			//Cas du début de la partie ou après que Inky soit mort
			if(jeu->objets[3].pos_case.x==13 && jeu->objets[3].pos_case.y==9) return 'd';
			else if(jeu->objets[3].pos_case.x==13 && jeu->objets[3].pos_case.y==10) return 'h';
			else if(jeu->objets[3].pos_case.x==12 && jeu->objets[3].pos_case.y==10) return 'h';
			else if(jeu->objets[3].pos_case.x==11 && jeu->objets[3].pos_case.y==10) return 'h';
			//Mode "Chase"
			if(jeu->etat_fantomes==0)
			{
				if(jeu->objets[3].modele_inky==0) return ia_rouge(jeu,mod,3,dt);
				else if(jeu->objets[3].modele_inky==1) return ia_rose(jeu,mod,3,dt);
				else return ia_orange(jeu,mod,3,dt);
			}
			//Mode "Frightened"
			else if(jeu->etat_fantomes==1)
			{
				return ia_orange(jeu,mod,3,dt);
			}
			//Mode "Scatter"
			//La zone Scatter d'Inky se situe en bas à droite du plateau. Lorsque celle-ci est atteinte, il tournera autour du bloc situé dans cette zone.
			if(jeu->objets[3].pos_case.x==25 && jeu->objets[3].pos_case.y<=19 && jeu->objets[3].pos_case.y>11) return 'g';
			else if(jeu->objets[3].pos_case.y==11 && jeu->objets[3].pos_case.x<=25 && jeu->objets[3].pos_case.x>23) return 'h';
			else if(jeu->objets[3].pos_case.x==23 && jeu->objets[3].pos_case.y>=11 && jeu->objets[3].pos_case.y<13) return 'd';
			else if(jeu->objets[3].pos_case.y==13 && jeu->objets[3].pos_case.x<=23 && jeu->objets[3].pos_case.x>20) return 'h';
			else if(jeu->objets[3].pos_case.x==20 && jeu->objets[3].pos_case.y>=13 && jeu->objets[3].pos_case.y<15) return 'd';
			else if(jeu->objets[3].pos_case.y==15 && jeu->objets[3].pos_case.x>=20 && jeu->objets[3].pos_case.x<23) return 'b';
			else if(jeu->objets[3].pos_case.x==23 && jeu->objets[3].pos_case.y>=16 && jeu->objets[3].pos_case.y<19) return 'd';
			else if(jeu->objets[3].pos_case.y==19 && jeu->objets[3].pos_case.x>=23 && jeu->objets[3].pos_case.x<25) return 'b';
			else return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[3].pos_case.x][(int)jeu->objets[3].pos_case.y],jeu->graphe.tab[25][19]);	
		}
		else 
		{	
			//Retour d'Inky à sa position initiale lorsqu'il est mort
			if((jeu->objets[3].pos_case.x==10 && jeu->objets[3].pos_case.y==10) || (jeu->objets[3].pos_case.x==11 && jeu->objets[3].pos_case.y==10) || (jeu->objets[3].pos_case.x==12 && jeu->objets[3].pos_case.y==10)) return 'b';
			else if(jeu->objets[3].pos_case.x==13 && jeu->objets[3].pos_case.y==10) return 'g';
			else return dijkstra(*jeu,jeu->graphe.tab[(int)jeu->objets[3].pos_case.x][(int)jeu->objets[3].pos_case.y],jeu->graphe.tab[10][10]);
		}
	}	
	return 'n';
};

void deplacement_objets(jeu_t* jeu,modele_plateau_t mod,double dt)
{
	int i;
	double vitesse=VITESSE;
	EtatSourisClavier esc = lireEtatSourisClavier();
	for(i=0;i<5;i++)
	{
		//On vérifie la vitesse à laquelle doit se déplacer l'objet
		if(jeu->objets[i].mort)vitesse=VITESSE*1.25;
		else if(jeu->etat_fantomes==1 && i!=0)vitesse=VITESSE/2;
		else if(i!=0 && jeu->objets[i].pos_case.x==13 && ((jeu->objets[i].pos_case.y>=0 && jeu->objets[i].pos_case.y<5) || (jeu->objets[i].pos_case.y>15 && jeu->objets[i].pos_case.y<=20)))
		{
			vitesse=VITESSE*0.6;
		}
		else if(i!=0) vitesse=0.9*VITESSE;
		else vitesse=VITESSE;
		//Si l'objet se situe sur une case, on met à jour sa direction et pos_case
		if(jeu->objets[i].sur_case)
		{
			//Mise à jour de la direction de pacman
			if(i==0)
			{
				if(esc.touchesClavier[4]) jeu->objets[i].direction='g'; //20 à la maison, 4 à l'univ
				else if(esc.touchesClavier[26]) jeu->objets[i].direction='h'; //29 à la maison, 26 à l'univ
				else if(esc.touchesClavier[7]) jeu->objets[i].direction='d';
				else if(esc.touchesClavier[22]) jeu->objets[i].direction='b';
				else jeu->objets[i].direction='n';
			}			
			//Mise à jour de la direction des fantômes
			//Blinky
			if(i==1) jeu->objets[i].direction=ia_rouge(jeu,mod,i,dt);
			//Pinky
			if(i==2) jeu->objets[i].direction=ia_rose(jeu,mod,i,dt);
			//Inky
			if(i==3) jeu->objets[i].direction=ia_bleu(jeu,mod,dt);
			//Clyde
			if(i==4) jeu->objets[i].direction=ia_orange(jeu,mod,i,dt);
			//Déplacement vers le haut
			if(jeu->objets[i].direction=='h' && mod.tab[(int)jeu->objets[i].pos_case.x-1][(int)jeu->objets[i].pos_case.y]!='x')
			{
				jeu->objets[i].pos.y+=vitesse*dt;
				jeu->objets[i].pos_case.x--;
				jeu->objets[i].sur_case=0;
			}
			//Déplacement vers la droite
			else if(jeu->objets[i].direction=='d' && mod.tab[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y+1]!='x')
			{
				jeu->objets[i].pos.x+=vitesse*dt;
				jeu->objets[i].pos_case.y++;
				jeu->objets[i].sur_case=0;
			}
			//Déplacement vers le bas
			else if(jeu->objets[i].direction=='b' && mod.tab[(int)jeu->objets[i].pos_case.x+1][(int)jeu->objets[i].pos_case.y]!='x')
			{
				jeu->objets[i].pos.y-=vitesse*dt;
				jeu->objets[i].pos_case.x++;
				jeu->objets[i].sur_case=0;
			}
			//Déplacement vers la gauche
			else if(jeu->objets[i].direction=='g' && mod.tab[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y-1]!='x')
			{
				jeu->objets[i].pos.x-=vitesse*dt;
				jeu->objets[i].pos_case.y--;
				jeu->objets[i].sur_case=0;
			}
			//Cas pour les fantômes lors de la sortie du carré central
			else if(jeu->objets[i].direction=='h' && ((jeu->objets[i].pos_case.x==12 && jeu->objets[i].pos_case.y==10) || (jeu->objets[i].pos_case.x==11 && jeu->objets[i].pos_case.y==10)))
			{
				jeu->objets[i].pos.y+=vitesse*dt;
				jeu->objets[i].pos_case.x--;
				jeu->objets[i].sur_case=0;	
			}
			//Cas pour les fantômes morts lors de l'entrée de la maison
			else if(jeu->objets[i].mort && jeu->objets[i].direction=='b' && ((jeu->objets[i].pos_case.x==10 && jeu->objets[i].pos_case.y==10) || (jeu->objets[i].pos_case.x==11 && jeu->objets[i].pos_case.y==10)))
			{
				jeu->objets[i].pos.y-=vitesse*dt;
				jeu->objets[i].pos_case.x++;
				jeu->objets[i].sur_case=0;	
			}
			//Pour passer d'un côté à l'autre du plateau via les passages sur les côtés
			//Pour passer de gauche à droite
			if(jeu->objets[i].direction=='g' && (int)jeu->objets[i].pos_case.x==13 && (int)jeu->objets[i].pos_case.y==0)
			{
				jeu->objets[i].pos_case.x=13;
				jeu->objets[i].pos_case.y=20;
				jeu->objets[i].pos.x=jeu->plateau[13][20].pos.x;
				jeu->objets[i].pos.y=jeu->plateau[13][20].pos.y;
			}
			//Pour passer de droite à gauche
			if(jeu->objets[i].direction=='d' && (int)jeu->objets[i].pos_case.x==13 && (int)jeu->objets[i].pos_case.y==20)
			{
				jeu->objets[i].pos_case.x=13;
				jeu->objets[i].pos_case.y=0;
				jeu->objets[i].pos.x=jeu->plateau[13][0].pos.x;
				jeu->objets[i].pos.y=jeu->plateau[13][0].pos.y;
			}
		}
		else
		{
			/*Afin d'avoir un déplacement continu et d'avoir un rendu semblable
			  à celui du jeu de base, les objets ne peuvent changer de direction entre 
			  deux cases. Le cas échéant, ils continuent de se déplacer comme lors
			  de la frame précédente*/
			//Déplacement vers le haut
			if(jeu->objets[i].direction=='h')
			{
				jeu->objets[i].pos.y+=vitesse*dt;
			}
			//Déplacement vers la droite
			else if(jeu->objets[i].direction=='d')
			{
				jeu->objets[i].pos.x+=vitesse*dt;
			}
			//Déplacement vers le bas
			else if(jeu->objets[i].direction=='b')
			{
				jeu->objets[i].pos.y-=vitesse*dt;
			}
			//Déplacement vers la gauche
			else if(jeu->objets[i].direction=='g')
			{
				jeu->objets[i].pos.x-=vitesse*dt;
			}
			//On vérifie si l'objet se situe de nouveau sur une case
			if(jeu->objets[i].pos.x>=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.x-3.0 && jeu->objets[i].pos.x<=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.x+3.0)
			{
				if(jeu->objets[i].pos.y>=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.y-3.0 && jeu->objets[i].pos.y<=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.y+3.0)
				{
					jeu->objets[i].pos.x=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.x;
					jeu->objets[i].pos.y=jeu->plateau[(int)jeu->objets[i].pos_case.x][(int)jeu->objets[i].pos_case.y].pos.y;
					jeu->objets[i].sur_case=1;
				}
			}
		}
	}
};

void maj_compteurs_dots(jeu_t* jeu)
{
	int i;
	jeu->nbr_dots_manges++;
	for(i=1;i<5;i++)
	{
		jeu->objets[i].compteur_dots++;
	}
};

void gestion_gum(jeu_t* jeu,double dt)
{
	if(jeu->etat_fantomes==1) 
	{
		jeu->duree_gum+=dt;
		if(jeu->duree_gum>DUREE_GUM)
		{
			jeu->duree_gum=0;
			jeu->nb_fantomes_manges=0;
			if(jeu->indice_alternance%2==0) jeu->etat_fantomes=2;
			else jeu->etat_fantomes=0;
		}
	}
};

void maj_compteurs_temps(jeu_t* jeu,double dt)
{
	int i,alea_i,alea_j;
	jeu->objets[0].compteur_alt_sprites+=dt;
	for(i=1;i<5;i++)
	{
		jeu->objets[i].compteur_alt_sprites+=dt;
		if(jeu->objets[i].sortie)
		{
			jeu->objets[i].compteur_temps+=dt;
			if(jeu->objets[i].compteur_temps>=3.0) jeu->objets[i].compteur_temps=0.0;
			if(jeu->objets[i].compteur_temps==0.0 || (jeu->objets[i].cible_alea.x==jeu->objets[i].pos_case.x && jeu->objets[i].cible_alea.y==jeu->objets[i].pos_case.y))
			{
				alea_i=rand()%26;
				alea_j=rand()%20;
				while(jeu->graphe.tab[alea_i][alea_j].interdit)
				{
					alea_i=rand()%26;
					alea_j=rand()%20;
					if(!jeu->graphe.tab[alea_i][alea_j].interdit)
					{
						jeu->objets[i].cible_alea.x=alea_i;
						jeu->objets[i].cible_alea.y=alea_j;	
					}
				}
			}
		}
	}
	if(jeu->compteur_affichage_gain_fruit!=0)
	{
		jeu->compteur_affichage_gain_fruit+=dt;
		if(jeu->compteur_affichage_gain_fruit>5.0)
		{
			jeu->compteur_affichage_gain_fruit=0.0;
		}
	}
	if(jeu->objets[3].sortie && jeu->etat_fantomes==0)
	{
		jeu->objets[3].compteur_inky+=dt;
		if(jeu->objets[3].compteur_inky>=5.0) jeu->objets[3].compteur_inky=0.0;
		if(jeu->objets[3].compteur_inky==0.0) jeu->objets[3].modele_inky=rand()%3;
	}
	if(jeu->etat_fantomes!=1)jeu->compteur_alternance+=dt;
};

void anim_mort(jeu_t jeu)
{
	int i;
	Image temp;
	for(i=54;i<=64;i++)
	{
		temp=rotozoomImage(jeu.images[i],0,2,2);
		afficherImage(temp,jeu.objets[0].pos.x-tn_largeur(temp)/2,jeu.objets[0].pos.y+tn_largeur(temp)/2);
		tamponner();
	}
};

void reset_mort(jeu_t* jeu)
{
	int i,j;
	//On efface les fantômes durant l'animation de la mort, pour n'afficher que le terrain
	effacerTableau();
	init_terrain(*jeu);
	for(i=0;i<27;i++)
	{
		for(j=0;j<21;j++)
		{
			if(jeu->plateau[i][j].dot)
			{
				selectionnerStylo(jeu->stylos.dot);
				tracerPoint(jeu->plateau[i][j].pos.x,jeu->plateau[i][j].pos.y);
			}
			if(jeu->plateau[i][j].pacgum)
			{
				selectionnerStylo(jeu->stylos.pacgum);
				tracerPoint(jeu->plateau[i][j].pos.x,jeu->plateau[i][j].pos.y);
			}
		}
	}
	//Réinitialisation des fantômes
	//Blinky
	jeu->objets[1]=init_objet(*jeu,1,10,10,1);
	//Pinky
	jeu->objets[2]=init_objet(*jeu,2,13,10,0);
	//Inky
	jeu->objets[3]=init_objet(*jeu,3,13,9,0);
	//Clyde
	jeu->objets[4]=init_objet(*jeu,4,13,11,0);
	//Animation mort
	anim_mort(*jeu);
	//Réinitialisation du joueur
	jeu->objets[0]=init_objet(*jeu,0,15,10,1);
	//Réinitialisation de l'état des fantômes
	jeu->etat_fantomes=2;
	//Réinitialisation des compteurs de temps des fantômes
	for(i=1;i<5;i++) jeu->objets[i].compteur_temps=0.0;
	jeu->objets[3].compteur_inky=0.0;
	//Réinitialisation de l'indice_alternance
	jeu->indice_alternance=0;
	//Réinitialisation du compteur permettant l'alternance entre les deux comportements des fantômes
	jeu->compteur_alternance=0.0;
};

void reset_niveau(jeu_t* jeu,modele_plateau_t mod)
{
	int i;
	init_plateau_graphe(jeu,mod);
	//Initialisation de pacman
	jeu->objets[0]=init_objet(*jeu,0,15,10,1);
	//Initialisation des fantômes
	//Blinky (rouge)
	jeu->objets[1]=init_objet(*jeu,1,10,10,1);
	//Pinky (rose)
	jeu->objets[2]=init_objet(*jeu,2,13,10,0);
	//Inky (bleu)
	jeu->objets[3]=init_objet(*jeu,3,13,9,0);
	//Clyde (orange)
	jeu->objets[4]=init_objet(*jeu,4,13,11,0);
	for(i=1;i<5;i++) jeu->objets[i].compteur_temps=0.0;
	jeu->objets[3].compteur_inky=0.0;
	jeu->etat_fantomes=2;
	jeu->nbr_dots_manges=0;
	jeu->indice_alternance=0;
	//Initialisation du compteur permettant l'alternance entre les deux comportements des fantômes
	jeu->compteur_alternance=0.0;
};

void collisions(jeu_t* jeu,double dt)
{
	int i;
	for(i=1;i<5;i++)
	{
		if(jeu->objets[0].pos_case.x==jeu->objets[i].pos_case.x && jeu->objets[0].pos_case.y==jeu->objets[i].pos_case.y && !jeu->objets[i].mort)
		{
			if(jeu->etat_fantomes==0 || jeu->etat_fantomes==2)
			{
				jeu->vies-=1;
				reset_mort(jeu);
			}
			else
			{
				jeu->objets[i].mort=1;
				jeu->nb_fantomes_manges++;
				if(jeu->nb_fantomes_manges==1)
				{	
					jeu->score+=200;
					jeu->compteur_score_vies+=200;
				}
				else if(jeu->nb_fantomes_manges==2)
				{
					jeu->score+=400;
					jeu->compteur_score_vies+=400;
				}
				else if(jeu->nb_fantomes_manges==3)
				{
					jeu->score+=800;
					jeu->compteur_score_vies+=800;
				}
				else if(jeu->nb_fantomes_manges==4)
				{
					jeu->score+=1600;
					jeu->compteur_score_vies+=1600;
				}
			}
		}
	}	
};

void fin_jeu(jeu_t* jeu)
{
	EtatSourisClavier esc=lireEtatSourisClavier();
	Image temp=jeu->images[0];
	if(jeu->vies==0) 
	{
		temp=rotozoomImage(jeu->images[84],0,2,2);
		afficherImage(temp,jeu->plateau[17][10].pos.x-tn_largeur(temp)/2,jeu->plateau[17][10].pos.y+tn_largeur(temp)/2);
		tamponner();
		attendreNms(1500);
		jeu->fin=1;
	}
	else if(esc.sourisBoutonDroit) jeu->fin=1;

};

void respawn_fantomes(jeu_t* jeu)
{
		if(jeu->objets[1].pos_case.x==12 && jeu->objets[1].pos_case.y==10) jeu->objets[1].mort=0;
		if(jeu->objets[2].pos_case.x==13 && jeu->objets[2].pos_case.y==10) jeu->objets[2].mort=0;
		if(jeu->objets[3].pos_case.x==13 && jeu->objets[3].pos_case.y==9) jeu->objets[3].mort=0;
		if(jeu->objets[4].pos_case.x==13 && jeu->objets[4].pos_case.y==11) jeu->objets[4].mort=0;
};

void maj_alternance(jeu_t* jeu,alt_t alt_chase_scatter)
{
	if(jeu->niveau==1 && jeu->compteur_alternance>alt_chase_scatter.tab[0][jeu->indice_alternance])
	{
		jeu->compteur_alternance=0.0;
		if(jeu->indice_alternance<8)
		{
			jeu->indice_alternance++;
		}
	}
	if(jeu->niveau>=2 && jeu->niveau<=4 && jeu->compteur_alternance>alt_chase_scatter.tab[1][jeu->indice_alternance])
	{
		jeu->compteur_alternance=0.0;
		if(jeu->indice_alternance<8)
		{
			jeu->indice_alternance++;
		}
	}
	if(jeu->niveau>=5 && jeu->compteur_alternance>alt_chase_scatter.tab[2][jeu->indice_alternance])
	{
		jeu->compteur_alternance=0.0;
		if(jeu->indice_alternance<8)
		{
			jeu->indice_alternance++;
		}
	}
	if(jeu->compteur_alternance==0.0)
	{
		if(jeu->indice_alternance%2==0) jeu->etat_fantomes=2;
		else jeu->etat_fantomes=0;
	}
};

void maj_fruits(jeu_t* jeu,double dt)
{
	
	if(jeu->num_fruit<8)
	{
		if(jeu->nbr_dots_manges==70 || jeu->nbr_dots_manges==140) jeu->fruits[jeu->num_fruit].affiche=1;
		if(jeu->fruits[jeu->num_fruit].affiche==1) jeu->fruits[jeu->num_fruit].compteur_fruit+=dt;
		if(jeu->fruits[jeu->num_fruit].affiche==1 && (jeu->fruits[jeu->num_fruit].compteur_fruit>DUREE_FRUIT || (jeu->objets[0].pos_case.x==15 && jeu->objets[0].pos_case.y==10)))
		{
			jeu->fruits[jeu->num_fruit].affiche=0;
			jeu->fruits[jeu->num_fruit].compteur_fruit=0.0;
			if(jeu->objets[0].pos_case.x==15 && jeu->objets[0].pos_case.y==10) 
			{
				jeu->score+=jeu->fruits[jeu->num_fruit].gain;
				jeu->compteur_score_vies+=jeu->fruits[jeu->num_fruit].gain;
				jeu->compteur_affichage_gain_fruit+=dt;
			}
			jeu->num_fruit++;
		}
	}
};

void maj_sprites(jeu_t* jeu)
{
	int i;
	for(i=0;i<5;i++) 
	{
		if(i!=0 && jeu->objets[i].compteur_alt_sprites>DT_SPRITES)jeu->objets[i].compteur_alt_sprites=0.0;
		else if(i==0 && jeu->objets[i].compteur_alt_sprites>DT_SPRITES/2.0)jeu->objets[i].compteur_alt_sprites=0.0;
		if(jeu->objets[i].compteur_alt_sprites==0.0)jeu->objets[i].alt_sprites++;
	}
};

void mettre_a_jour(jeu_t* jeu,modele_plateau_t mod,alt_t alt_chase_scatter)
{
	int i;
	EtatSourisClavier esc = lireEtatSourisClavier();
	double dt = delta_temps();
	//Mise à jour des compteurs temps des fantômes sortis
	maj_compteurs_temps(jeu,dt);
	//Mise à jour des sprites des objets
	maj_sprites(jeu);
	//Mise à jour du mode de déplacement des fantômes
	maj_alternance(jeu,alt_chase_scatter);
	//Mise à jour du fruit
	maj_fruits(jeu,dt);
	//Condition d'arrêt de la boucle d'animation
	if(esc.sourisBoutonDroit) jeu->fin=1;
	//Gestion des déplacements des objets mobiles
	deplacement_objets(jeu,mod,dt);
	//Mise à jour du score, d'etat_fantomes, de nbr_dots_manges
	if(jeu->objets[0].sur_case && jeu->plateau[(int)jeu->objets[0].pos_case.x][(int)jeu->objets[0].pos_case.y].dot)
	{
		jeu->score+=10;
		jeu->compteur_score_vies+=10;
		jeu->plateau[(int)jeu->objets[0].pos_case.x][(int)jeu->objets[0].pos_case.y].dot=0;
		maj_compteurs_dots(jeu);
	}
	if(jeu->objets[0].sur_case && jeu->plateau[(int)jeu->objets[0].pos_case.x][(int)jeu->objets[0].pos_case.y].pacgum)
	{
		jeu->score+=50;
		jeu->compteur_score_vies+=50;
		jeu->etat_fantomes=1;
		jeu->duree_gum=0;
		jeu->plateau[(int)jeu->objets[0].pos_case.x][(int)jeu->objets[0].pos_case.y].pacgum=0;
		maj_compteurs_dots(jeu);
	}
	gestion_gum(jeu,dt);
	//On vérifie les conditions de sortie de chacun des fantômes
	for(i=1;i<5;i++)
	{
		if(i==1 || i==2) jeu->objets[i].sortie=1;
		if(i==3 && jeu->objets[i].compteur_dots>30) jeu->objets[i].sortie=1;
		if(i==4 && jeu->objets[i].compteur_dots>60) jeu->objets[i].sortie=1;
	}
	//On réalise les collisions le cas échéant
	collisions(jeu,dt);
	//On vérifie si les fantômes morts ont atteint leur position de départ
	respawn_fantomes(jeu);
	//Vérification de la condition de fin de niveau
	if(jeu->nbr_dots_manges==193) 
	{
		jeu->niveau++;
		reset_niveau(jeu,mod);
	}
	//On vérifie si le joueur gagne une vie supplémentaire
	if(jeu->compteur_score_vies>10000)
	{
		jeu->vies++;
		jeu->compteur_score_vies=0;
	}
	//Vérification des conditions de fin de partie
	fin_jeu(jeu);
};





























