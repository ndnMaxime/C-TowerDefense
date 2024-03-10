#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <MLV/MLV_all.h>

#define A 50
#define R 100
#define P 25
#define X 200
#define F 75
#define LIGNES 7
#define COLONNES 30
#define TAILLE_MARGE 40
#define RAYON_POS 7
#define TAILLE_PV_X 30
#define TAILLE_PV_Y 5
#define TAILLE_IMAGE_DEF 50 

typedef struct chips{
    int type;
    int life;
    int line;
    int position;
    int price;
    int dmg;//domages qu'elle inflige
    struct chips* next;
}Chips;

typedef struct virus{
    int type;
    int life;
    int line;
    int position;
    int speed;
    int dmg;
    int turn;
    int can_move;
    int life_init;
    struct virus* next;
    struct virus* next_on_line; // ils ne sont pas sur des lignes differentes
    struct virus* prev_on_line;         
} Virus;

typedef struct{
    Virus* virus;
    Chips* chips;
    Virus* last_virus; // Facilite l'insertion d'un élément dans la liste
    Chips* last_chip; //  idem
    int turn;
    int money;
}Game;

/* ---------------------------------------
   FONCTIONS D'OUVERTURRE/INITIALISATION
   ---------------------------------------
*/
char* level_fichier(int cpt){
    /*
    la fonction retourne le niveau correpsondant au niveau donné en parametre
    */
    if(cpt == 1){
        return "level1.txt";
    }else if(cpt == 2){
        return "level2.txt";
    }else if(cpt == 3){
        return "level3.txt";
    }else if(cpt == 4){
        return "level4.txt";
    }else if(cpt == 5){
        return "level5.txt";
    }
    return " ";
}

FILE* existFichier(char fichier[]){
    /*
    La fonction verifie s'il est possible d'ouvrir le fichier qu'on lui donne.
    Elle renvoit soit NULL si on a pas pu ouvrir le fichier ou 
    un pointeur sur le fichier ouvert sinon.
    */
    return fopen(fichier, "r");
}

void initialiseGame(Game* game){
    /*
    La fonction initialise le game en prenant un pointeur sur celui ci en parametre.
    */
    game->chips = NULL;
    game->virus = NULL;
    game->last_chip = NULL;
    game->last_virus = NULL;
    game->turn = 0;
    game->money = 0;
}

void associeChip(Chips* chip, char t){
  /*
    la fonction associe a un l'adresse d'un chip donné en argument 
    le type de tourelle qui lui correspond grace au caractère donné en paramètre
    elle ne retourne rien.
  */
    if (t == 'A'){
      // tourelle de base 
      chip->type = 1;
      chip->life = 3;
      chip->price = A;
      chip->next = NULL;    
      chip->dmg = 1;

      return;

    }else if (t == 'R'){
      // tourelle qui ralenti les virus
      chip->type = 2;
      chip->life = 3;
      chip->price = R;
      chip->next = NULL;
      chip->dmg = 0;
  
      return;

    }else if(t == 'X'){
      // tourelle qui explose
      chip->type = 3;
      chip->life = 1;
      chip->price = X;
      chip->next = NULL;
      chip->dmg = 0;

      return;

    }else if(t == 'P'){
      // tourelle qui tire sur 3 lignes
      chip->type = 4;
      chip->life = 5;
      chip->price = P;
      chip->next = NULL;
      chip->dmg = 3;

      return;

    }else{ // t=='F'
      // tourelle mur 
      chip->type = 5;
      chip->life = 10;
      chip->price = F;
      chip->next = NULL;
      chip->dmg = 0;

      return;
    }
}

int associeVirus(Virus* virus, char t){
  /*
  la fonction associe a un l'adresse d'un virus donné en argument 
  le type de tourelle qui lui correpsond grace au caractère donné en paramètre
  elle retourne 1 si le virus existe 0 sinon.
  */
    if (t == 'E'){
        //virus simple
        virus->type = 1;
        virus->life = 16;
        virus->speed = 2;
        virus->dmg = 2;
        virus->life_init = 16;
        virus->next = NULL;
        virus->next_on_line = NULL;
        virus->prev_on_line = NULL;
        return 1;
    }
    if (t == 'D'){
        //virus simple 2
        virus->type = 2;
        virus->life = 12;
        virus->speed = 4;
        virus->dmg = 1;
        virus->life_init = 12;
        virus->next = NULL;
        virus->next_on_line = NULL;
        virus->prev_on_line = NULL;
        return 1;
    }
    if (t == 'S'){
        //virus tank
        virus->type = 3;
        virus->life = 30;
        virus->speed = 1;
        virus->dmg = 2;
        virus->life_init = 30;
        virus->next = NULL;
        virus->next_on_line = NULL;
        virus->prev_on_line = NULL;
        return 1;
    }
    if (t == 'M'){
        //virus heal
        virus->type = 4;
        virus->life = 13;
        virus->speed = 2;
        virus->dmg = 0;
        virus->life_init = 13;
        virus->next = NULL;
        virus->next_on_line = NULL;
        virus->prev_on_line = NULL;
        return 1;
    }
    if (t == 'B'){
        // virus Boss
        virus->type = 5;
        virus->life = 60;
        virus->speed = 2;
        virus->dmg = 4;
        virus->life_init = 60;
        virus->next = NULL;
        virus->next_on_line = NULL;
        virus->prev_on_line = NULL;
        return 1;
    }
    return 0;
}

Chips * allouer_celluleChips(){
    /*
    La fonction alloue l'espace memoire pour la nouvelle tourelle.
    elle retourne l'adresse de cette nouvelle tourelle.
    */
    Chips * tmp = (Chips*)malloc(sizeof(Chips));
    return tmp;
}

Virus * allouer_celluleVirus(){
  /*
    La fonction alloue l'espace memoire pour le nouveau virus.
    elle retourne l'adresse de ce nouveau virus.
  */
    Virus * tmp = (Virus*)malloc(sizeof(Virus));
    return tmp;
}


int sur_bouton(int bouton[4], int x, int y){
    /*
    La fonction verifie si les coordonnées du click données en parametre sont comprises dans 
    celle du bouton donnée en parametre.
    */
    if(bouton[0] < x && x < bouton[2] && bouton[1] < y && y < bouton[3]){
        return 1;
    }
    return 0;
}

int initialiseBoutons_menu(int bouton[4], char nom){
    /*
    La fonction initialise un bouton du menu donné en parametre par rapport
    a son nom egalement donné en parametre.
    elle retourne 1 si le bouton existe et 0 sinon
    */
    if (nom == 'J'){
        bouton[0] = 100;
        bouton[1] = 100;
        bouton[2] = bouton[0] + 400;
        bouton[3] = bouton[1] + 75 ;
        return 1;
    }else if(nom == 'O'){
        bouton[0] = 100;
        bouton[1] = 200;
        bouton[2] = bouton[0] + 400;
        bouton[3] = bouton[1] + 75 ;
        return 1;
    }else if(nom == 'Q'){
        bouton[0] = 100;
        bouton[1] = 300;
        bouton[2] = bouton[0] + 400;
        bouton[3] = bouton[1] + 75 ;
        return 1;
    }else if(nom == 'R'){
        bouton[0] = 100;
        bouton[1] = 400;
        bouton[2] = bouton[0] + 400;
        bouton[3] = bouton[1] + 75 ;
    }
    return 0;
}

int initialiseBoutons(int bouton[4], char nom, int taille[2]){
    /*
    La fonction initilaise les boutons du jeu a partir de leur nom et de la taille du plateau
    donné en parametre
    Elle retourn e1 si le bouton existe 0 sinon.
    */
    int pixel_largeur_defense = (taille[0] * 40)/100;
    int pixel_largeur_button = (taille[0] * 10)/100;
    int pos_vague = pixel_largeur_defense + TAILLE_MARGE * 5;
    int pos_play = pos_vague + pixel_largeur_button + TAILLE_MARGE * 2;
    int pos_quitter = pos_play + pixel_largeur_button + TAILLE_MARGE * 2;
    if (nom == 'P'){
        bouton[0] = pos_play;
        bouton[1] = TAILLE_MARGE;
        bouton[2] = bouton[0] + pixel_largeur_button;
        bouton[3] = bouton[1] + TAILLE_MARGE * 2;
        return 1;
    }else if (nom == 'V'){
        bouton[0] = pos_vague;
        bouton[1] = TAILLE_MARGE;
        bouton[2] = bouton[0] + pixel_largeur_button;
        bouton[3] = bouton[1] + TAILLE_MARGE * 2;
        return 1;
    }else if (nom == 'Q'){
        bouton[0] = pos_quitter;
        bouton[1] = TAILLE_MARGE;
        bouton[2] = bouton[0] + pixel_largeur_button;
        bouton[3] = bouton[1] + TAILLE_MARGE * 2;
        return 1;
    }
    return 0;
}

/* -------------------------------
   FONCTIONS AFFICHAGE CONSOLE
   -------------------------------
*/
void menu(int money){
    /*
    La fonction permet l'affichage du menu. Elle prend en parametre
    l'argent que posède le joueur et ne renvoit rien.
    */
    printf("Quelle tourelle voulez vour deployer ?\n");
    printf("Vous avez %d€ \n", money);
    printf("A : tourelle de base %d€\n", A);
    printf("R : ralentisseur %d€\n", R);
    printf("X : tourelle qui explose %d€\n", X);
    printf("P : tourelle qui tire sur 3 lignes %d€\n", P);
    printf("F : mur %d€\n", F);
    printf("q : fini \n");
    printf("Votre choix ? \n");
}

void C_ordreAff(Game* game, int line, Chips* tab[COLONNES]){
    /*
    La fonction remplit un tableau de chip dans 
    l'ordre de leur apparition sur le plateau.
    Elle prend en paramètre le jeu, la ligne sur laquel on cherche
    les chips et un tableau de chips.
    */
    Chips* chip = game->chips;
    while(chip){
        if (chip->line == line){
            tab[chip->position] = chip;// remplit le tableau
        }
        chip = chip->next;
    }
}

void V_ordreAff(Game* game, int line, Virus* tab[COLONNES]){ 
    /*
    La fonction remplit un tableau de virus dans 
    l'ordre de leur apparition sur le plateau.
    Elle prend en paramètre le jeu, la ligne sur laquel on cherche
    les virus et un tableau de virus.
    Elle ne revoit rien.
    */
   //int cpt = 0;
    Virus* virus = game->virus;
    while(virus){
        if (virus->line == line){
            break;
        }
        virus = virus->next;
    }
    while(virus){
        tab[virus->position] = virus;
        virus = virus->next_on_line;
    }
}

char Chips_typeI_typeC(int type){
    /*
    La fonction retourne le type correspondant au type donné
    en paramètre.
    */
    if (type == 1){return 'A';}
    if (type == 2){return 'R';}
    if (type == 3){return 'X';}
    if (type == 4){return 'P';}
    if (type == 5){return 'F';}
    return ' ';
}

char Virus_typeI_typeC(int type){
    /*
    La fonction retourne le type correspondant au type donné
    en paramètre.
    */
    if (type == 1){return 'E';}
    if (type == 2){return 'D';}
    if (type == 3){return 'S';}
    if (type == 4){return 'M';}
    if (type == 5){return 'B';}
    return ' ';
}

int afficheLigne(Game* game, int line){
    /*
    La fonction affiche une ligne dans le terminal.
    Elle prend en paramètre le jeu et la ligne sur laquelle elle travaille.
    Elle retourne 1 si l'affichage a marcher et 0 sinon.
    */
    Chips** tabC = calloc(COLONNES, sizeof(Chips));
    Virus** tabV = calloc(COLONNES, sizeof(Virus));

    if (tabC == NULL || tabV == NULL){
        return 0;
    }

    C_ordreAff(game, line, tabC);
    V_ordreAff(game, line, tabV);
    printf("%d| ", line);
    for(int i=1; i<COLONNES; i++){
        if(tabC[i]){
            printf("%c%d  ", Chips_typeI_typeC(tabC[i]->type), tabC[i]->life);
        }else if (tabV[i]){
            printf("%c%d  ", Virus_typeI_typeC(tabV[i]->type), tabV[i]->life);
        }else{
            printf("    ");
        }
    }
    printf("\n");
    free(tabC);
    free(tabV);
    return 1;
}

void affiche(Game* game){
    /*
    La fonction affiche les tourelle et les virus dans le terminal.
    Elle prend en argument le jeu et ne retourne rien.
    */
    printf("\e[1;1H\e[2J"); // nettoie la console
    printf("Tour : %d\n\n", game->turn);
    for(int i=1; i<LIGNES+1; i++){
        afficheLigne(game, i);
    }  
}

void afficheVirus(Game* game, int line){
    /*
    la fonction affiche les virus sur la ligne line dans la console.
    Elle prend egalement en parametre un pointeur sur le game.
    */
    Virus* tmp = game->virus;
    Virus** tabV = calloc(COLONNES, sizeof(Virus));

    if (tabV == NULL){
        return;
    }
    while(tmp){
        if (tmp->line == line+1){
            tabV[tmp->turn - 1] = tmp;// remplit le tableau
        }
        tmp = tmp->next;
    }
    for (int i =0; i<COLONNES; i++){
        if (tabV[i]){
            printf("%c%d  ", Virus_typeI_typeC(tabV[i]->type), tabV[i]->life);
        }else{
            printf("    ");
        }
    }
    printf("\n");
    free(tabV);
}

void afficheVague(Game* game){
    /*
    La fonction prend en parametre un pointeur sur le game.
    Elle permet l'affichage des lignes des virus.
    */
    for (int i = 0; i<LIGNES; i++){
        printf("%d| ", i+1);
        afficheVirus(game, i);
    }
}

void affiche_victoire(int resultat){
    /*
    la fonction permet l'affichage dans le temrinal du resulatat de la partie.
    Elle prend en parametre le resultat.
    */
    if(resultat == 1){
        printf("Bravo! Vous avez gagné !\n");
    }
    else{
        printf("Perdu ! \n");
    }
}


/*GESTION DE LA MUSIQUE*/

MLV_Music* nom_musique(int music){
    /*
    La fonction retourne la musique correspondant a son numéro donné en parametre.
    */
    if(music == 1){
        return MLV_load_music("menu.mp3");
    }
    if(music == 2){
        return MLV_load_music("preparation.mp3");
    }
    if(music == 3){
        return MLV_load_music("vague.mp3");
    }
    else{
        return MLV_load_music("level_clear.mp3");
    }
}

void stop_musique(MLV_Music* musique){
    /*
    La fonction stop la musique donnée en parametre.
    */
    MLV_stop_music(musique);
    MLV_free_music(musique);
    MLV_free_audio();
}

/* -------------------------------
   FONCTIONS AFFICHAGE GRAPHIQUE
   -------------------------------
*/

MLV_Image* image_nom_Chips(int a){
    /*
    La fonction asocie au type donné en parametre l'image correspondante.
    */
    if (a == 1){
        return MLV_load_image("fleur_feu.png"); 
    }
    if(a == 2){
        return MLV_load_image("fleur_glace.png"); 
    }
    if(a == 3){
        return MLV_load_image("mine.png"); 
    }
    if(a == 4){
        return MLV_load_image("tourelle_max_range.png"); 
    }
    return MLV_load_image("wall.png"); 
    
}

MLV_Image* image_nom_Virus(int a){
    /*
    La fonction asocie au type donné en parametre l'image correspondante.
    */
    if (a == 1){
        return MLV_load_image("goomba.png"); 
    }
    if(a == 2){
        return MLV_load_image("paratoopa.png"); 
    }
    if(a == 3){
        return MLV_load_image("heriss.png"); 
    }
    if(a == 4){
        return MLV_load_image("Larry_Koopa.png"); 
    }
    return MLV_load_image("bowser.png");
}

void cree_fenetre_game(int taille[2]){
    /*
    La fonction crée une fentre de taille taille[0] par taille[1].
    */
    MLV_create_window("projet","projet", taille[0],taille[1]);
}

void taille_full_screen(int taille[2]){
    /*
    La fonction remplit le tableau taille qui lui est donné en paramètre avec la taille de l'ecran.
    */
    unsigned int a,b;
    MLV_get_desktop_size(&a,&b);
    taille[0] = a;
    taille[1] = b;
}

void dessine_menu(){
    /*
    La fonction dessine les boutons du menu.
    */
    MLV_create_window("menu","menu",600,500);
    MLV_draw_text_box(100, 100,400, 75, "Jouer",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(100, 200,400, 75, "Options",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(100, 300,400, 75, "Quitter",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);

    MLV_actualise_window();
}

int menu_game(){
    /*
    La fonction ne prend rien en parametre et retourne 1 si je joueur veut lancer la partie,
    retourne 2 si le joueur veut acceder aux options,
    retourne 3 si le joueur veux quitter.
    */
    MLV_Music* musique;
    int x,y;
    int boutonJouer[4], boutonOption[4], boutonQuitter[4];
    initialiseBoutons_menu(boutonJouer, 'J');
    initialiseBoutons_menu(boutonOption, 'O');
    initialiseBoutons_menu(boutonQuitter, 'Q');
    MLV_init_audio();
    dessine_menu();
    musique = nom_musique(1);
    MLV_play_music(musique, 35, -1);
    MLV_wait_mouse(&x,&y);
    while(1){
        if(sur_bouton(boutonJouer,x,y)){
            stop_musique(musique);
            MLV_free_window();
            return 1;
        }
        if (sur_bouton(boutonOption,x,y)){
            stop_musique(musique);
            return 2;
        }
        if(sur_bouton(boutonQuitter,x,y)){
            stop_musique(musique);
            MLV_free_window();
            return 0;
        }
        MLV_wait_mouse(&x,&y);
    }
}

void fenetre_option(){
    /*
    La fonction affiche les option de taille de la fenetre de jeu.
    */
    MLV_clear_window(MLV_COLOR_BLACK);
    MLV_draw_text_box(100, 100,400, 75, "1920 x 1080",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(100, 200,400, 75, "1680 x 1050",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(100, 300,400, 75, "1360x768",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(100, 400,400, 75, "FULL SCREEN",
    10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_actualise_window();
}

void dessine_ligne(int taille[2]){
    /*
    La fonction dessine les lignes dans la fenetre, elle prend en prametre les dimensions de celle si.
    */
    MLV_Image* brique;
    brique = MLV_load_image("ligne.png");
    int x,y, espace_position, espace_ligne;
    y = (taille[1] * 20)/100;
    espace_ligne = ((taille[1] *75) / 100) / LIGNES;
    for(int i = 0; i < LIGNES; i++){
        x = (taille[0] * 15)/100;
        espace_position = ((taille[0] * 85) / 100)/COLONNES;
        if(espace_position > 50){ // si l'écart entre chaque ligne dépasse la largeur de notre pixel brique qui est de 50
            espace_position = 50;
        }
        for(int j = 0; j < COLONNES; j++){
            MLV_draw_image(brique,x,y);
             //espace entre chaque position d'une ligne
            x += espace_position;
        }
        y+=espace_ligne; //espace entre chaque ligne
    }
}

void affiche_fond(int taille[2]){
    /*
    La fonction premet d'afficher le fond de l'ecran, elle prend 
    en parametre la taille de celui ci.
    */
    int pos_x, pos_y;
    pos_x = ((15 * taille[0])/100) / 3;
    pos_y = ((75 * taille[1])/100)/2;

    MLV_Image *support, *mario;
    support = MLV_load_image("support1.jpg");
    mario = MLV_load_image("mario3.png");

    MLV_draw_image(support, 0, 0);
    MLV_draw_image(mario, pos_x, pos_y);
    dessine_ligne(taille);
    //MLV_actualise_window();
}

void Points_positions(int taille[2]){
    /*
    Fonction qui affiche les points de positions (en vert) là où le joueur peut disposer ses défenses. 
    Elle prend en paramètre un tableau d'entier tel que : 
        taille[0] = largeur de la fenêtre de jeu
        taille[1] = hauteur de la fenêtre de jeu
    Elle ne renvoit rien.
    */
    int x,y, espace_position, espace_ligne;
    y = (taille[1] * 20)/100;
    espace_ligne = ((taille[1] *75) / 100) / LIGNES;
    for(int i = 0; i < LIGNES; i++){
        x = (taille[0] * 15)/100;
        espace_position = ((taille[0] * 85) / 100)/COLONNES;
        if(espace_position > 50){ // si l'écart entre chaque ligne dépasse la largeur de notre pixel brique qui est de 50
            espace_position = 50;
        }
        for(int j = 0; j < COLONNES - 1; j++){
            MLV_draw_filled_circle(x ,y, RAYON_POS, MLV_COLOR_GREEN);
            x += espace_position; //espace entre chaque position d'une ligne  
        }
        y+=espace_ligne; //espace entre chaque ligne
    }
}

void pv_virus(Virus* virus,int x, int y){
    /*
    La fonction prend en parametre un pointeur sur un virus ainsi que des coordonées, 
    elle dessine la vie au dessus de celui ci.
    */
    y += 10;
    int a,b;
    if(virus->life == virus->life_init){
        MLV_draw_filled_rectangle(x,y, TAILLE_PV_X, TAILLE_PV_Y, MLV_COLOR_GREEN);
    }
    else{
        a = (virus->life * 100) / virus->life_init;
        b = (TAILLE_PV_X * a) / 100;
        MLV_draw_filled_rectangle(x,y, b, TAILLE_PV_Y, MLV_COLOR_GREEN);
        MLV_draw_filled_rectangle(x + TAILLE_PV_Y, y, b - TAILLE_PV_X, TAILLE_PV_Y, MLV_COLOR_RED);
    }
}

int affiche_graphique_ligne(Game* game, int line, int taille[2]){
    /*
    La fonction prend en parametre un pointeur sur le game, la ligne sur laquelle il 
    travaille ainsi que la taille de la fenetre.
    Elle dessine les images des virus et des tourelles a leurs positions.
    Elle retourne 0 s'il y a eu un probleme et 1 sinon.
    */
    Chips** tabC = calloc(COLONNES, sizeof(Chips));
    Virus** tabV = calloc(COLONNES, sizeof(Virus));
    int x,y, espace_position, espace_ligne;
    espace_ligne = ((taille[1] *75) / 100) / LIGNES;
    espace_position = ((taille[0] * 85) / 100)/COLONNES;
    if (tabC == NULL || tabV == NULL){
        return 0;
    }
    C_ordreAff(game, line, tabC);
    V_ordreAff(game, line, tabV);
    for(int i=1; i<COLONNES; i++){
        x = ((taille[0] * 15)/100) - espace_position ;
        y = ((taille[1] * 20)/100) - espace_ligne;
        if(espace_position > 50){ // si l'écart entre chaque ligne dépasse la largeur de notre pixel brique qui est de 50
            espace_position = 50;
        }
        if(tabC[i]){
            x += (tabC[i]->position) * espace_position;
            y += (tabC[i]->line) * espace_ligne;
            MLV_draw_image(image_nom_Chips(tabC[i]->type), x - 20, y - 37);
        }
        else if (tabV[i]){
            x += (tabV[i]->position) * espace_position;
            y += (tabV[i]->line) * espace_ligne;
            MLV_draw_image(image_nom_Virus(tabV[i]->type), x - 20, y - 37);
            pv_virus(tabV[i], x,y);
        }
    }
    free(tabC);
    free(tabV);
    return 1;
}


void affichage_graphique(Game* game, int taille[2]){
    /*
    La fonction permet l'affichage des lignes du jeu (enemis et defenses).
    Elle prend en parametre un pointeur sur le game et un tableau avec les 
    dimensions de la fenetre.
    */
    for(int i = 1; i < LIGNES + 1; i++){
        affiche_graphique_ligne(game, i, taille);
    }
}

void start_game_graphique(int taille[2], Game* game){
    /*
    Fonction d'affichage du début de partie.
    Elle prend en paramètre un tableau d'entier tel que : 
        taille[0] = largeur de la fenêtre de jeu
        taille[1] = hauteur de la fenêtre de jeu
    et 
    Elle ne retourne rien.
    */
    MLV_Image *money = NULL;
    char argent[5];
    int tmp, pos_vague, pos_play, pos_quitter;
    int pixel_largeur_defense = (taille[0] * 40)/100;
    int pixel_largeur_button = (taille[0] * 10)/100;
    int pixel_hauteur = (taille[1] * 20)/100;
    tmp = pixel_largeur_defense / 5;
    if (pixel_hauteur > 100){
        pixel_hauteur = 100;
    }
    money = MLV_load_image("money.png");
    Points_positions(taille);
    MLV_draw_filled_rectangle(20, 20, pixel_largeur_defense, pixel_hauteur, MLV_COLOR_GREY);
    for(int i = 0; i < 5; i++){
        MLV_draw_image(image_nom_Chips(i + 1), TAILLE_MARGE + i * tmp, TAILLE_MARGE);
    }

    MLV_draw_image(money,pixel_largeur_defense + TAILLE_MARGE, TAILLE_MARGE);
    sprintf(argent,"%d",game->money);
    MLV_draw_text(pixel_largeur_defense + TAILLE_MARGE * 3, 70, argent, MLV_COLOR_WHITE);

    pos_vague = pixel_largeur_defense + TAILLE_MARGE * 5;
    pos_play = pos_vague + pixel_largeur_button + TAILLE_MARGE * 2;
    pos_quitter = pos_play + pixel_largeur_button + TAILLE_MARGE * 2;
    MLV_draw_text_box(pos_vague, TAILLE_MARGE,pixel_largeur_button, 75, "Voir la vague",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(pos_play, TAILLE_MARGE,pixel_largeur_button, 75, "Jouer",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
    MLV_draw_text_box(pos_quitter, TAILLE_MARGE,pixel_largeur_button, 75, "Quitter",
                     10,MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_COLOR_GREY, MLV_TEXT_LEFT ,MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);

    MLV_actualise_window();
}

void actualiseFenetre(Game* game, int taille[2]){
    /*
    La fonction prend en paramètre l'adresse d'un jeu et ne retourne rien.
    Elle met a jour la fenetre.
    */
    MLV_clear_window(MLV_COLOR_BLACK);
    affiche_fond(taille);
    affichage_graphique(game, taille);
    MLV_actualise_window();
}

void V_ordreAff2(Game* game, int line, Virus* tab[COLONNES]){ 
    /*
    Presque la meme fonction que V_ordreAff.
    */
    Virus* virus = game->virus;
    while(virus){
        if (virus->line == line){
            break;
        }
        virus = virus->next;
    }
    while(virus){
        tab[virus->turn] = virus;
        virus = virus->next_on_line;
    }
}

int graphique_vague(Game* game, int line, int taille[2]){
    /*
    la fonction prend en parametre un pointeur sur le game ainsi que la ligne sur laquelle
    il travaille et la taille de la fenetre.
    Elle dessine les virus qui apparaitront lors de la partie.
    Elle retourne 1 si tout c'est bien passé 0 sinon.
    */
    Virus** tabV = calloc(COLONNES, sizeof(Virus));
    int x,y, espace_ligne, espace_position;
    if (tabV == NULL){
        return 0;
    }
    V_ordreAff2(game, line, tabV);
    for(int i=1; i<COLONNES; i++){
        x = (taille[0] * 15)/100;
        espace_position = ((taille[0] * 85) / 100)/COLONNES;
        y = (taille[1] * 20)/100;
        espace_ligne = ((taille[1] *75) / 100) / LIGNES;
        if (tabV[i]){
            x += tabV[i]->turn  * espace_position;
            y += (tabV[i]->line - 1) * espace_ligne;
            MLV_draw_image(image_nom_Virus(tabV[i]->type), x - 20, y - 37);
        }
    }
    free(tabV);
    return 1;
}

void affichage_vague_graphique(Game* game, int taille[2]){
    /*
    La fonction prend en parametre un pointeur sur le game et la taille de la fenetre
    et affiche les vagues qui viendront dans la niveau.
    */
    for (int i = 0; i<LIGNES + 1; i++){
        graphique_vague(game, i, taille);
    }
}

int clique_on_defense(int x, int y, int taille[2]){
    /*
    La fonction permet 'selectionner' une tourelle, elle retourne le type de la tourelle en question.
    Elle prend en parametre deux entier (les coordonnées du click) et les dimensions de la fenetre
    */
    int a,espace_defense;
    int tab_Chips_Type[5];
    int pixel_largeur_defense = (taille[0] * 40)/100;
    espace_defense = pixel_largeur_defense/5;
    for(int i = 0; i < LIGNES; i ++){
        a = TAILLE_MARGE + i * espace_defense;
        tab_Chips_Type[i+1] = i + 1;
        if(a < x && x < a + TAILLE_IMAGE_DEF && 45 < y && y < 45 + TAILLE_IMAGE_DEF){
            MLV_draw_rectangle(a, 45, TAILLE_IMAGE_DEF + 5, 55, MLV_COLOR_GREEN);
            MLV_actualise_window();
            return i + 1;
       }
    }
    return 0;
}

void affichage_fin(int taille[2], int resultat){
    /*
    la fonction prend en parametre les dimension de la fenetre et le resultat de la partie.
    elle affiche une image pour la victoire et une autre pour la defaite
    */
    MLV_Image* image, *support, *etoile,*victoire, *mario_triste;
    MLV_clear_window(MLV_COLOR_BLACK);
    support = MLV_load_image("support1.jpg");
    MLV_draw_image(support, 0, 0);
    if(resultat){
        image = MLV_load_image("mario_victoire.png");
        etoile = MLV_load_image("etoile.png");
        victoire = MLV_load_image("victoire.png");
        MLV_draw_image(etoile,taille[0]/4, taille[1]/8);
        MLV_draw_image(image,taille[0]/2, taille[1]/4);
        MLV_draw_image(victoire, taille[0]/2, taille[1]/8);
        MLV_actualise_window();
        MLV_wait_seconds(5);
        MLV_free_window();
    }else{
        mario_triste = MLV_load_image("mario_triste.png");
        MLV_draw_image(mario_triste, taille[0]/2, taille[1]/8);
        MLV_actualise_window();
        MLV_wait_seconds(5);
    }
}

/* -------------------------------
   AJOUT/SUPPRESSION D'ELEMENTS
   -------------------------------
*/
int verif_money(Game * game, int prix){
  /*
    La fonction verifie que l'argent du joueur (dans Game) est superieur
    au prix d'une tourelle qu'il veut acheter.
    Elle retourne 0 si il n'en a pas assez et 1 sinon.
  */
    if(game->money >= prix){
        return 1;
    }
    printf("Vous n'avez pas assez d'argent pour déployer cette défense ! argent restant : %d\n", game->money);
    return 0;
}

int ajout_Chips(Game * game, int info[2], char t){
  /*
    La fontion ajoute une tourelle a la liste des tourelles.
    Elle prend en argument le jeu, un tableau de deux informations 
    (sur la position de la tourelle) et un charactère.
    elle retourne 0 si l'allocation n'a pas foncitonner et 1 sinon.
  */
    Chips * chip;
    if (!(chip = allouer_celluleChips())){
        return 0;
    }

    associeChip(chip, t);
    chip->line= info[0];
    chip->position= info[1];

    if (!verif_money(game, chip->price)){
      return 0;
    }
    game->money -= chip->price;

    if(!game->chips){
        game->chips = chip;
        game->last_chip = chip;
        return 1;
    }
    game->last_chip->next = chip;
    game->last_chip = chip;
    return 1;
}

int ajout_Virus(Game * game, int info[2], char t){
  /*
    La fontion ajoute un virus a la liste des virus.
    Elle prend en argument le jeu, un tableau de deux informations 
    (sur le virus) et un charactère.
    elle retourne 0 si l'allocation n'a pas foncitonner et 1 sinon.
  */
    Virus * virus;
    if(!(virus = allouer_celluleVirus())){
        return 0;
    }

    if (!associeVirus(virus, t)){
        return 0;
    }
    virus->turn = info[0];
    virus->line= info[1]; 
    virus->position = info[0] + COLONNES  ;   //changer le -2 ici
    virus->can_move = 1;

    if(!game->virus){
        game->virus = virus;
        game->last_virus = virus;
        return 1;
    }
    game->last_virus->next = virus; // on met a la suite du dernier
    game->last_virus = virus; // on remplace le dernier
    
    Virus * tmp = game->virus;
    while(tmp->next){ //tant qu'on est pas au denier élement de notre liste
        if(tmp->line == virus->line && tmp->next_on_line == NULL){
            tmp->next_on_line = virus;
            virus->prev_on_line = tmp;
            break; // dans le meilleur des cas on boucle pas jusqu'a la fin
        }
        tmp = tmp->next;
    }
    return 1;
}

int supprimeChips(Chips* chip, Game* game){
     /*
    La fonction supprime la tourelle dont l'adresse est donné en paramètre.
    Elle retourne 1 si la suppression a marché 0 sinon.
    */
    Chips* tmp = game->chips;
    if(chip == game->chips){ 
        // si on supprime le premier virus de la chaine
        game->chips = chip->next;
        free(chip);
        return 1;
    }   
    while(tmp){
        if(tmp->next == chip){
            tmp->next = chip->next;
            free(chip);
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

int supprimeVirus(Virus* virus){
    if(virus->prev_on_line) {
        virus->prev_on_line->next_on_line = virus->next_on_line;
    }
    if(virus->next_on_line) {
        virus->next_on_line->prev_on_line = virus->prev_on_line;
    }
    free(virus);
    return 1;
}

// initialisation du niveau
int init_level(char f[], Game *game){
    /*
    La fonction permet l'initialisation d'un niveau.
    Elle prend en paramètre le nom d'un fichier et un 
    l'adresse d'un game.
    Elle retourne 0 si le fichier n'est pas viable et 1 sinon.
    */
    //cara_virus[0]->num tour
    //cara_virus[1 -> num ligne
    FILE* fichier;
    fichier = existFichier(f);
    if(!fichier){
      return 0;
    }
    int cpt = 0;
    char ligne[6];
    char type;
    int cara_virus[2];
    while (fgets(ligne, 10, fichier)){  
      if (!cpt){
          if(strlen(ligne)-1 == 3){
              game->money += (ligne[0] - 48) * 100 + (ligne[1] - 48) * 10 + (ligne[2] - 48);
          }else if(strlen(ligne)-1==4){
              game->money +=(ligne[0] - 48) * 1000 + (ligne[1] - 48) * 100 + (ligne[2] - 48) * 10 + ligne[3];
          }
      }
      else{
          if(strlen(ligne) - 1 == 4){
              // si le tour est du type XX
              cara_virus[0] = (ligne[0] - 48) * 10 + (ligne[1] - 48); 
              cara_virus[1] = ligne[2] - 48; 
              type = ligne[3];
          }
          else{
              // si le tour est du type X
              cara_virus[0] = ligne[0] - 48;
              cara_virus[1] = ligne[1] - 48;
              type = ligne[2];
          }
          if (!ajout_Virus(game, cara_virus, type)){
            fclose(fichier);
            return 0;
          }
      }
      cpt++;
    }
    fclose(fichier);
    return 1;
}

/* -------------------------------
   POSE DES TOURELLES
   -------------------------------
*/
int emplacement_libre(Game * game, int line, int position){
    /*
      La fonction verifie si l'emplacement ou le joueur veut placer
      une tourelle est libre. Elle prend en argument l'adresse du jeu, 
      la ligne et la colonne ou il veut jouer.
      La fonction retourne O si la place n'est pas disponible et 1 sinon.
    */
    Chips * lst = game->chips;
    while(lst){
        if (lst->position == position && lst->line == line){
            return 0;
        }
        lst = lst->next;
    }
    return 1;
}

int initialise_chips(Game * game, char type, int tab[2]){
    /*
    La fonction permet d'innitailaiser une tourelle. ELle prend 
    en parametre l'adresse du jeu, le type de tourelle et un tableau vide.
    Elle retourne 1 si on a bien innitialiser une tourelle et 0 sinon.
    */
    printf("Où voulez vous placer la tourelle (ligne -> entier entre 1 et %d )?\n", LIGNES);
    scanf("%d",&tab[0]);
    printf("Où voulez vous placer la tourelle (position -> entier entre 1 et %d )?\n", COLONNES);
    scanf("%d",&tab[1]);
    
    if(emplacement_libre(game, tab[2], tab[3])){
        if (ajout_Chips(game,tab, type)){
            return 1;
        }
    }else{
        printf("l'emplacement n'est pas libre.\n");
    }
    return 0;
}

int choix_defense(Game* game){
    /*
    La fonction prend en parametre un pointeur sur le game et permet d'ajouter une
    tourelle.
    Elle retourne 1 si on a ajouté une tourelle et 0 si on est sorti du menu.
    */
    char type;
    int tab[2];
    menu(game->money);
    scanf("%c", &type);
    while(!(type == 'A' || type == 'R' || type == 'P' || type == 'X' || type == 'F' || type == 'q')){
        if (type != '\n'){
            printf("La tourelle %c n'existe pas \n", type);
        }
        scanf("%c", &type);
    }
    if (type == 'q'){
        printf("Vous êtes sortie du menu\n\n");
        return 0;
    }

    if (initialise_chips(game, type, tab)){
        printf("La tourelle est ajoutée !\n");
    }else{
        printf("La tourelle ne peut etre déployé.");
    }
    return 1;
}

void saisie_defense(Game* game){
    /*
    La fonction permet de rajouter des defenses, elle prend en paraemetre 
    un pointeur sur le game.
    */
    while(choix_defense(game)){
        if (!game->money){
            printf("Vous n'avez plus d'argent disponible ! \n");
            break;
        }
    }
}

// fonctionnement du jeu
void degatsChips(Game* game, int dmg[LIGNES], int ralentisseur[LIGNES]){
    /*
    la fontion calcule les degats par ligne et les met dans un tableau dmg,
    elle verifie aussi si il y a eu un ralentisseur.
    */
    for (int i =0; i<LIGNES; i++){
        dmg[i] = 0;
        ralentisseur[i] = 0;
    }

    Chips * tmp =  game->chips;
    while(tmp){
        // type 5 : mur || type 2 : ralenti || type 3 : explose
        if (tmp->type == 5 || tmp->type == 2 || tmp->type == 3){
            if (tmp->type == 2){
                ralentisseur[tmp->line - 1] = 1;
            }
            tmp = tmp->next;
        }else{
            // type 1 : tire sur une ligne (type 3 aussi)
            dmg[tmp->line - 1] += tmp->dmg;
            // type 4 : tir sur 3 lignes
            if (tmp->type == 4){
                dmg[tmp->line -2] += tmp->dmg;
                dmg[tmp->line] += tmp->dmg;
            }
            tmp = tmp->next;
        }  
    }
}

void ajoute_degatsVirus(Virus* virus, int dmg[]){
    /*
    la fonction ajoute les degats au virus concerné, elle prend en parametre 
    un pointeur sur un virus et un tableau contenant les degats.
    */
    int line = virus->line-1;
    int life = virus->life;
    if(life == 0 || !dmg[line] || virus->position >= COLONNES) {
        return;
    }
    if(virus->life > dmg[line]) {
        virus->life -= dmg[line];
        dmg[line] = 0;
        return;
    }
    // si jamais les dégats totaux de la ligne sont superieurs à la vie du premier virus
    dmg[line] -= virus->life;
    virus->life = 0;
}

void retrouve_speed(Virus* virus){
    /*
    La fonction permet de retrouver la vitesse initiale d'un vaisseau à partir de son type.
    Elle prends en paramètre l'adresse d'un virus.
    Elle ne retourne rien.
    */
    if (virus->type == 1){
        virus->speed = 2;
    }
    if (virus->type == 2){
        virus->speed = 4;
    }
    if (virus->type == 3){
        virus->speed = 1;
    }
    if (virus->type == 4){
        virus->speed = 2;
    }
}

int virus_on_chip(Virus* virus, Game* game){
    /*
    La fonction prend en parametre un pointeur sur un virus et sur le game.
    Elle verifie les collisions entre les virus et les tourelles.
    Elle retourne 1 si il y a eu collision et 0 sinon.
    */
    int pos_virus, pos_chip;
    Chips* chip = game->chips;
    while(chip){
        if(chip->line == virus->line){
            pos_virus = virus->position - (1 * virus->speed);
            pos_chip = chip->position;
            if(pos_virus <= pos_chip){// si le virus depasse la tourelle
                virus->can_move = 0;
                virus->position = pos_chip + 1;
                // type 3 : explose
                if (chip->type == 3){
                    chip->life = 0;
                    virus->life = 0;
                }else{
                    chip->life -= virus->dmg;
                }
                return 1;
            }
        }
        chip = chip->next;
    }
    virus->can_move = 1;
    return 0;
}

int move_virus(Virus* virus, Game* game){
    /*
    La fonction permet de contrôler les déplacements des virus en fonction de leur vitesse.
    Ralentissement des virus d'une même ligne si un virus à une toute petite vitesse...
    Elle retourne 1 si on rentre dans le cas ou un virus dépasserait celui qui le succéde et 0 sinon.
    */
    int pos_virus1, pos_virus2;
    if(!virus->prev_on_line){
        retrouve_speed(virus);
        if (virus_on_chip(virus, game)){
            return 2;
        }
    }
    else{
        if(virus->prev_on_line->can_move){
            pos_virus1 = virus->position - (1 * virus->speed);
            pos_virus2 = virus->prev_on_line->position;
            if(pos_virus1 <= pos_virus2){ // le virus 2 va depasser le virus 1
                virus->position = pos_virus2 + 1;
                virus->speed = virus->prev_on_line->speed;
                return 1;
            }
        }
        else{
            virus->position = virus->prev_on_line->position + 1;
            return 3;
        }
    }
    return 0;
}

void ralenti(Virus* virus, int ralentisseur[LIGNES]){
    /*
    La fonction ralenti le premier virus d'une ligne ou il y a un ralentisseur.
    Elle prend en parametre un pointeur sur un virus et un tableau indiquant s'il y a un ralentisseur.
    */
    if(virus->prev_on_line){
        return;
    }
    Virus* tmp = allouer_celluleVirus();
    tmp->type = virus->type;
    retrouve_speed(tmp);
    if (tmp->speed == virus->speed){
        if (ralentisseur[virus->line-1] && virus->speed >1){
            virus->speed = virus->speed / 2;
        }
    }
    else{
        // s'il a perdu de la vitesse mais qu'il n'est plus attaqué par un ralentisseur
        if(!ralentisseur[virus->line-1]){ 
            retrouve_speed(virus);
        }
    }

    free(tmp);
}

void soigne(Virus* virus){
    /*
    La fonction regarde si un virus peut soigné le premier virus de sa ligne.
    Elle prend en parametre un pointeur sur le premier virus.
    */
    Virus* courant =virus;
    if (virus->dmg == 0){ // comme un compteur 
        virus->dmg = 1;// il ne soigne qu'un tour sur deux
        return;
    }
    virus->dmg = 0;
    while (courant->prev_on_line){
        courant = courant->prev_on_line;
    }
    if (courant->life_init == courant->life){
        return;
    }
    courant->life += 1;
}

void actualise_Virus(Game* game){
    /*
    la fonction actualise les virus, elle prend en parametre un pointeur sur le game.
    Elle modifie les virus en les tuant, faisant avancé...
    */
    int dmg[LIGNES];
    int ralentisseur[LIGNES];
    degatsChips(game, dmg, ralentisseur); 
    
    Virus* virus = game->virus;
    Virus* previous = NULL;
    Virus* tmp = NULL;

    while(virus){
        // si on a plusieurs tourelle sur une ligne et que les degats depassent la vie du premier virus on doit quand meme tuer les autres
        ajoute_degatsVirus(virus, dmg);
        
        if (virus->life == 0){
            if(previous) {
                previous->next = virus->next;
            }else if(virus == game->virus){
                game->virus = virus->next;
            }
            tmp = virus->next;
            supprimeVirus(virus); // normalement virus devrait disparaitre de la memoire
            virus = tmp;
        } else{ // si le virus est mort il n'avance plus
            if (virus->position < COLONNES + 1){
                if(!move_virus(virus, game)){
                    ralenti(virus, ralentisseur);
                    if (virus->type == 4){ // type 4: virus qui soigne
                        soigne(virus);
                    }
                    virus->position -= 1 * virus->speed;
                }
            }else{
                // si il est pas encore sur le terrain il avance pas a sa vitesse sinon il apparaitra trop tot
                virus->position -= 1; 
            }
            previous = virus;
            virus = virus->next;
        }
    }
}

void actualise_Chips(Game * game){
    /*
    La fonction actualise les tourelles et prend en parametre un pointeur sur le game.
    Elle tue les tourelle morte.
    */
    Chips * tmp =  game->chips;
    while(tmp){
        if(tmp->life <= 0){
           supprimeChips(tmp, game);
        }
        tmp = tmp->next;
    }  
}

void miseAJour(Game* game){
    /*
    la fonction met a jour le jeu (virus et chips)
    Elle prend en parametre un pointeur sur un Game.
    */
    actualise_Virus(game);
    actualise_Chips(game);
}

int clique_on_position(int x, int y, int info[2],int taille[2]){
    /*
    La fonction verifie si le joueur a selectionner une position valide. 
    Elle prend en parametre deux entier (les coordonnées du click) et deux tableau d'entiers
    dans infos elle stocke la position et taille contient les dimensions de la fenetre.
    Elle retourne 0 si on la positions n'existe pas et 1 sinon.
    */
    int x1,y1, espace_position, espace_ligne;
    y1 = (taille[1] * 20)/100;
    espace_ligne = ((taille[1] *75) / 100) / LIGNES;
    for(int i = 0; i < LIGNES; i ++){
        x1 = (taille[0] * 15)/100;
        espace_position = ((taille[0] * 85) / 100)/COLONNES;
        if(espace_position > 50){ // si l'écart entre chaque ligne dépasse la largeur de notre pixel brique qui est de 50
            espace_position = 50;
        }
        for(int j = 0; j < COLONNES; j++){
            if( x1 - RAYON_POS * 2 < x && x < x1 + RAYON_POS * 2 && y1 - RAYON_POS * 2 < y && y < y1 + RAYON_POS * 2){
                info[0]= i + 1;
                info[1] = j + 1;
                return 1;
            }
            x1 += espace_position;
        }
        y1+=espace_ligne;
    }
    return 0;
}

void pose_defense(int x, int y, int type, Game* game, int taille[2]){
    /*
    la fonction permet au joueur de poser sa defense. 
    Elle prend en parametre les coordonées du click (x, y) le type de la tourelle, 
    un pointeur sur le game et la taille de la fenetre.
    */
    int info[2];
    if(clique_on_position(x, y, info, taille)){
        ajout_Chips(game, info,Chips_typeI_typeC(type));
    }
}


/* -------------------------------
            FIN DE JEU
   -------------------------------
*/
int defaite(Game* game){
    /*
    la fonction prend en parametre un pointeur sur le game.
    Elle verifie si on a perdu.
    Elle retourne 1 si c'est c'est le cas à sinon.
    */
    Virus* virus = game->virus;
    while(virus){
        if (virus->position <= 1){
            return 1;
        }
        virus = virus->next;
    }
    return 0;
}

int endGame(Game* game){
    /*
    La fonction verifie si on a atteind la fin du jeu.
    Elle retourne 1 si on a gagné, 2 si on a perdu et 0 sinon.
    Elle prend en parametre un pointeur sur le game.
    */
    if (!game->virus){
        return 1;//gagné
    }
    if (defaite(game)){
        return 2; 
    }
    return 0;
}

/* -------------------------------
            BOUCLE DE JEU
   -------------------------------
*/
int boucle(Game*game){
    /*
    C'est la boucle principale de la version ascii.
    Elle retourne un entier fin quand le jeu est fini, et prend en parametre un pointeur 
    sur le game.
    */
    int fin;
    affiche(game);
    while(1){
        //affiche(game);
        game->turn += 1;
        fin = endGame(game);
        if (fin){
            return fin;
        }
        miseAJour(game);
        affiche(game);
        sleep(1);
    }
}

void affichageDebut(Game* game, int taille[2]){
    /*
    la fonction fait l'affichage pour la preparation du jeu. 
    Elle prend en parametre un pointeur sur le jeu et la taille de la fenetre de jeu.
    */
    MLV_clear_window(MLV_COLOR_BLACK);
    affiche_fond(taille);
    start_game_graphique(taille, game);
    affichage_graphique(game, taille);
    MLV_actualise_window();
}

int preparation(Game* game, int taille[2]){
    /*
    La fonction permet de placer ses tourelles pour pouvoir lancer le jeu ainsi que de visualiser le niveau.
    Elle prend en parametre un pointeur sur le game et un tableau d'entier.
    Elle retourne 1 si la partie commence ou 0 si le joueur quitte.
    */
    MLV_Music* musique;
    int boutonPlay[4], boutonVague[4], boutonQuitter[4];
    int x,y;
    MLV_init_audio();
    initialiseBoutons(boutonPlay, 'P', taille);
    initialiseBoutons(boutonVague, 'V', taille);
    initialiseBoutons(boutonQuitter, 'Q', taille);
    
    musique = nom_musique(2);
    MLV_play_music(musique, 35, -1);

    affichageDebut(game, taille);
    MLV_wait_mouse(&x, &y);
    while(1){
        if(sur_bouton(boutonVague, x,y)){
            affiche_fond(taille);
            affichage_vague_graphique(game, taille);
            MLV_actualise_window();
            MLV_wait_seconds(5);
            actualiseFenetre(game, taille);
        }else if(sur_bouton(boutonPlay,x,y)){
            stop_musique(musique);
            return 1;
        }else if(sur_bouton(boutonQuitter,x,y)){
            return 2;
        }else{
            int a = clique_on_defense(x,y,taille); // récupère le type de la défense
            if (a){
                MLV_wait_mouse(&x, &y);
                pose_defense(x,y,a,game, taille);
            }
        }
        affichageDebut(game, taille);
        MLV_wait_mouse(&x, &y);
    }
    return 0;
}

void option(int taille[2]){
    /*
    La fonction permet de savoir la taille qu'aura la fenetre de jeu, elle met cette taille dans 
    taille sui lui est donné en parametre.
    */
    int x,y;
    int bouton1920x1080[4], bouton1680x1050[4], boutonFullScreen[4], bouton1360x768[4];
    initialiseBoutons_menu(bouton1920x1080, 'J');
    initialiseBoutons_menu(bouton1680x1050, 'O');
    initialiseBoutons_menu(boutonFullScreen, 'R');
    initialiseBoutons_menu(bouton1360x768, 'Q');
    fenetre_option();
    MLV_wait_mouse(&x,&y);
    while(1){
        if(sur_bouton(bouton1920x1080,x,y)){
            taille[0] = 1920;
            taille[1] = 1080;
            break;
        }
        if (sur_bouton(bouton1680x1050,x,y)){
            taille[0] = 1680;
            taille[1] = 1080;
            break;
        }
        if(sur_bouton(boutonFullScreen,x,y)){
            /*
            unsigned int a,b;
            MLV_get_desktop_size(&a,&b);
            taille[0] = a;
            taille[1] = b;
            */
            break;
        }
        if(sur_bouton(bouton1360x768,x,y)){
            taille[0] = 1360;
            taille[1] = 768;
            break;
        }
        MLV_wait_mouse(&x,&y);
    }
    MLV_free_window();
}

int boucle2(Game*game, int taille[2]){
    /*
    C'est la boucleprincipale de la version graphique du jeu.
    Elle prend en parametre un pointeur sur le game et la taille de la fenetre.
    Elle retourne un entier fin qui detemine la victoire.
    */
    MLV_Music* musique;
    MLV_init_audio();
    int fin;

    musique = nom_musique(3);
    MLV_play_music(musique,35,-1);
    while(1){
        actualiseFenetre(game, taille);
        fin = endGame(game);
        if (fin){
            if(fin == 2){
                affichage_fin(taille, 0);
            }
            return fin;
        }
        miseAJour(game);
        game->turn += 1; 
        MLV_wait_seconds(1); // attente avant actualisation du jeu console
    }
    stop_musique(musique);
}

void partie(int level){
    /*
    La fonction permet de lancer une partie dans le terminal.
    elle prend en parametre un niveau.
    */
    Game game;
    int resultat;
    initialiseGame(&game);
    init_level(level_fichier(level), &game);
    afficheVague(&game);
    saisie_defense(&game);
    resultat = boucle(&game);
    affiche_victoire(resultat);
}

int partie_graphique(int level, int taille[2]){
    /*
    la fonction permet de jouer une partie dans la fenetre. Elle prend en parametre 
    un niveau et la taille de la fenetre et retourne 1 si le jeu c'est bien passé et 0 si le joueur
    a arreter la partie.
    */
    Game game;
    int resultat;
    if (level == 1){
        cree_fenetre_game(taille);
    }
    initialiseGame(&game);
    init_level(level_fichier(level), &game);
    if(preparation(&game, taille)== 2){
        return 0;
    } 
    resultat = boucle2(&game, taille);
    if(resultat == 2){
        return 0;
    }
    return 1;
}

void lance_jeu_graphique(){
    int taille[2];
    int choix = menu_game();
    int level = 1;
    taille_full_screen(taille); // initialise taille
    while(1){
        if (choix == 2){
            option(taille);
            choix = menu_game();
        }else if(choix == 1){
            while(level != 5){
                if(partie_graphique(level,taille) == 1){
                    level += 1;
                    choix+=1;
                }else{
                    level = 0;
                    break;
                }
            }
            break;
        }else if (!choix){
            break;
        }
    }if(level == 5){
        affichage_fin(taille,level);
    }else if(!level){
        MLV_free_window();
    }
}

void demarage(int argc, char* argv[]){
    /*
    La fonction permet de demanrer le jeu.
    Elle prend en parametre un entier qui correspond a la taille du second.
    */
    if (argc != 2){
        return;
    }
    if (argv[1][0] == '-'){
        if (argv[1][1] == 'g'){
            lance_jeu_graphique();
            return;
        }
        if (argv[1][1] == 'a'){
            int c;
            for (int i = 1; i<6; i++){
                partie(i);
                printf("on continue ? 0 pour non 1 pour oui ");
                scanf("%d", &c);
                if (!c){
                    return;
                }
            }
        }
    }
}

int main(int argc, char* argv[]){
    demarage(argc, argv);
    return 0;
}
