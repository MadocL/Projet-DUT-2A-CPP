#include "mbed.h"
#include "DHT.h"
#include "SSD1308.h"

#define VIES 3
#define MAX_ASTEROID 8
#define DATA_ASTEROID 3
#define MAX_SHOT 7
#define DATA_SHOT 2
#define BULLET_LENGTH 7

void movePlayer(int []); //Mouvements du joueur
void shootPlayer(int [][DATA_SHOT], int []); //tir du joueur

void createAsteroids(int [][DATA_ASTEROID]); //Création des astéroïdes 
void moveAsteroids(int [][DATA_ASTEROID], int []); //Mouvements Asteroides 
void dispAsteroids(int [][DATA_ASTEROID], int []); //Affichage mouvements du Asteroide
void deleteAsteroids(int [][DATA_ASTEROID], int []); //Detruire asteroide

void moveShots(int [][DATA_SHOT], int []); //Mouvement tirs joueur
void dispShots(int [][DATA_SHOT], int []); //Affichage tirs
void deleteShots(int [][DATA_SHOT], int []);

int handleHitAsteroids(int [][DATA_SHOT], int [][DATA_ASTEROID], int []);
int handleHitShip(int [][DATA_ASTEROID], int[], int);
void hud(int, int);
void gameOver(int);

AnalogIn pot(A3);
DigitalIn ubt(USER_BUTTON);
DHT sensor (D6, SEN11301P);
    
I2C i2c(I2C_SDA, I2C_SCL);
SSD1308 oled = SSD1308(&i2c, SSD1308_SA0);

//Sprites :
uint8_t ship1[19] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00};
uint8_t ship2[19] = {0x10, 0x10, 0x38, 0x28, 0x54, 0x54, 0x54, 0x28, 0x10, 0xBA, 0x6D, 0x00, 0x00, 0x28, 0x54, 0x92, 0x92, 0x83, 0x82};
uint8_t ship3[19] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01, 0x00};
uint8_t shipErase[19] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t heart[9] = {0x0E, 0x11, 0x21, 0x42, 0x84, 0x42, 0x21, 0x11, 0x0E};
uint8_t fullHeart[9] = {0x0E, 0x1F, 0x3F, 0x7E, 0xFC, 0x7E, 0x3F, 0x1F, 0x0E};
uint8_t bullet[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
uint8_t bulletErase[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t asteroidS[4] = {0x18, 0x2C, 0x14, 0x18};
uint8_t asteroidM[5] = {0x30, 0x5C, 0x6A, 0x5C, 0x28};
uint8_t asteroidL[10] = {0x10, 0x28, 0x58, 0x6C, 0xF2, 0xAD, 0xDB, 0x65, 0x5A, 0x2C};
uint8_t asteroidErase[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int difficulty = 30; //Notre compteur de difficulté concernant l'apparition d'asteroïdes
int intervalle;
int cpt = 20; // Notre compteur permettant de gerer l'apparition d'asteroides


int main() {
    int score = 0;
    
    int player[2] = {0, VIES}; //Position joueur et vies
    int shots[MAX_SHOT][DATA_SHOT] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}; //Tableau de position des tirs
    int oldShotsPos[MAX_SHOT]; //Tableau d'ancienne position des tires (pour l'affichage rapide)
    int asteroids[MAX_ASTEROID][DATA_ASTEROID]= {{120,120,0},{0,0,0},{120,120,0},{120,120,0},{120,120,0},{120,120,0},{120,120,0},{120,120,0}};
    int oldAsteroidsPos[MAX_ASTEROID]; //tableau des anciennes positions des asteroides
    int oldPlayerPos = 0;
    int invu = 0;
    int shootInterval = 0; //Intervalle entre chaque tirs
    
    while(1){ //boucle infini du jeu
         movePlayer(player);
         
        /* if(sensor.readData() == 0){ //Lecture DHT
            
        }else {
            oled.printf("erreur de lecture");
        }*/
         if (player[1] <= 0){  // Si le joueur n'a plus de vie, fin boucle + clearDisplay
            oled.clearDisplay();
            break;
         }
         
         //Exemple d'une fonction d'affichage (le vaisseau)
         if (oldPlayerPos != player[0] || invu > 0){ //Efface l'ancienne position du vaisseau si mouvement
             oled.writeBitmap(shipErase, oldPlayerPos, oldPlayerPos, 1, 19);
             oled.writeBitmap(shipErase, oldPlayerPos+1, oldPlayerPos+1, 1, 19);
             oled.writeBitmap(shipErase, oldPlayerPos+2, oldPlayerPos+2, 1, 19);
         }
         oldPlayerPos = player[0]; //Enregistrement ancienne positon vaisseau
         //Affichage du vaisseau
         oled.writeBitmap(ship1, player[0], player[0], 1, 19);
         oled.writeBitmap(ship2, player[0]+1, player[0]+1, 1, 19);
         oled.writeBitmap(ship3, player[0]+2, player[0]+2, 1, 19);
         
         //Detection pression bouton utilisateur et s'active si interval inférieur à 0
         if (ubt.read()== 0 && shootInterval < 0){
            shootPlayer(shots, player);
            shootInterval = 4;//Réinitialisation intervalle empêchant un autre tir
        }
         hud(score, player[1]); //affichage hud
         createAsteroids(asteroids);
         moveAsteroids(asteroids, oldAsteroidsPos);
         moveShots(shots, oldShotsPos);
         score += handleHitAsteroids(shots, asteroids, oldAsteroidsPos); //Incrémente le score en fonction de l'astéroïde

         invu = handleHitShip(asteroids, player, invu);
         deleteAsteroids(asteroids, oldAsteroidsPos); //efface quand vie égale à 0 ou quand x < 0
         deleteShots(shots, oldShotsPos);
         dispAsteroids(asteroids, oldAsteroidsPos);
         dispShots(shots, oldShotsPos);
         shootInterval --; //Désincrémente l'interval pour les tirs
         
        }
        gameOver(score); //Game Over si sortie de boucle 
        return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

//Mouvement du joueur 
void movePlayer(int player[]){
    player[0] = (pot * 4) / 1; //Convertis les données du potentiomètre en position
}


//fonction pour tirer, est appelée lorsque le button est appuyé
void shootPlayer(int shots[][DATA_SHOT], int player[]){
    for(int i = 0; i < MAX_SHOT; i++){
        if(shots[i][0] == 0 && shots[i][1] == -1){
            shots[i][0] = 20;
            shots[i][1] = player[0]+1;
            break;
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------------------------------------

//créer les astéroides
void createAsteroids(int asteroids [][DATA_ASTEROID] ){
    srand(time(NULL));
    int vide = 0;
    for(int i = 0; i < MAX_ASTEROID; i++){
        if(asteroids[i][2] <= 0 && cpt == 0){
            asteroids[i][0] = 127; //position axe x
            asteroids[i][1] = (rand()%5)+1; //Positon axe Y tirée au sort parmi les lignes possibles de l'écran
            asteroids[i][2] = (rand()%3)+1; //Type asteroides
            if (difficulty > 4 && intervalle == 0){ 
                difficulty --;
                intervalle = 10;
            } else {
                cpt = 4;
            } 
            cpt = difficulty;
            intervalle --;  
        }
        if(asteroids[i][2] == 0 && !vide){
             vide = 1;
        }
    }
    if(vide) cpt --;
}

//fait avancer les astéroides à chaque appel
void moveAsteroids(int asteroids[][DATA_ASTEROID], int oldAsteroidsPos[]){
    for(int i = 0; i < MAX_ASTEROID; i++){
        oldAsteroidsPos[i] = asteroids[i][0];
        asteroids[i][0] = asteroids[i][0] - 3;
    }
}

//affichages des asteroides à l'écran
void dispAsteroids(int asteroids[][DATA_ASTEROID], int oldAsteroidsPos []){
    for(int i = 0; i < MAX_ASTEROID; i++){
        switch(asteroids[i][2]){
            case 1:
            oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 5);
            oled.writeBitmap(asteroidS, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 3);
            break;
            case 2:
            oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 6);
            oled.writeBitmap(asteroidM, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 5);
            break;
            case 3:
            oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 11);
            oled.writeBitmap(asteroidL, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 10);
            break;
        }
    }
}

//supression des asteroides lorsque la vie est égale à 0 ou quand la position en x est inférieure à 0
void deleteAsteroids(int asteroids[][DATA_ASTEROID], int oldAsteroidsPos []){
    for(int i = 0; i < MAX_ASTEROID; i++){
        if(asteroids[i][2] == 0 || asteroids[i][0] < 0){
            switch(asteroids[i][2]){
                case 1:
                oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 5);
                break;
                case 2:
                oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 6);
                break;
                case 3:
                oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], oldAsteroidsPos[i], oldAsteroidsPos[i] + 12);
                break;
            }
            asteroids[i][0] = 0;
            asteroids[i][1] = 0;
            asteroids[i][2] = 0;
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------------------------------------


//fait avancer les tirs à chaque appel de la fonction
void moveShots(int shots[][DATA_SHOT],int oldShotsPos[MAX_SHOT]){
    for(int i = 0; i < MAX_SHOT; i++){
        if(shots[i][1] != -1) {
            oldShotsPos[i] = shots[i][0];
            shots[i][0] = shots[i][0] + 8;
        }
    }
}

//afichages des tirs
void dispShots(int shots[][DATA_SHOT], int oldShotsPos[MAX_SHOT]){
    for(int i = 0; i < MAX_SHOT; i++){
        if(shots[i][1] != -1) {
            oled.writeBitmap(bulletErase, shots[i][1], shots[i][1], oldShotsPos[i], oldShotsPos[i] + BULLET_LENGTH);
            oled.writeBitmap(bullet, shots[i][1], shots[i][1], shots[i][0], shots[i][0] + BULLET_LENGTH);
        }
    }
}

//effacement des tirs lorsque la position en x est supérieur à 127 (taille de l'écran)
void deleteShots(int shots[][DATA_SHOT], int oldShotsPos[]){
    for(int i = 0; i < MAX_SHOT; i++){
        if(shots[i][0] > 127){
            oled.writeBitmap(bulletErase, shots[i][1], shots[i][1], oldShotsPos[i], oldShotsPos[i] + BULLET_LENGTH - 2);
            
            shots[i][0] = 0;
            shots[i][1] = -1;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

//fonction de hitBox entre les astéroides et les tirs du joueur
int handleHitAsteroids(int shots[][DATA_SHOT], int asteroids[][DATA_ASTEROID], int oldAsteroidsPos[]){
    int points = 0;
    for(int i = 0; i < MAX_SHOT; i++){ //pour chaque tirs
        for(int j = 0; j < MAX_ASTEROID; j++){ //pour chaque asteroides
            if(shots[i][1] == asteroids[j][1] && asteroids[j][0] - shots[i][0] <= 0){ //s'ils sont visuellement en contact
                points++;
                oled.writeBitmap(bulletErase, shots[i][1], shots[i][1], shots[i][0], shots[i][0] + BULLET_LENGTH + 2);
                shots[i][0] = 128; //affectation d'une valeur supérieure à 128 pour que le tir soit supprimé au tour suivant
                switch(asteroids[i][2]){ //suppression de l'affichage de l'ancien sprite
                    case 1:
                    oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 8);
                    break;
                    case 2:
                    oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 9);
                    break;
                    case 3:
                    oled.writeBitmap(asteroidErase, asteroids[i][1], asteroids[i][1], asteroids[i][0], asteroids[i][0] + 15);
                    break;
                }
                asteroids[j][2] = asteroids[j][2] - 1; //décrémentation de la vie de l'asteroide
            }
        }
    }
    return points;
}

//fonction hitBox entre le joueur et les asteroides
int handleHitShip(int asteroids[][DATA_ASTEROID] , int player[], int invu){
    for(int j = 0; j < MAX_ASTEROID; j++){
            //s'ils sont visuellement en contact et que le joueur n'est pas dans sa péroides d'invulnérabilité
            if((player[0] + 1 == asteroids[j][1] || player[0] == asteroids[j][1] || player[0] + 2 == asteroids[j][1] ) && asteroids[j][0] == 19 && invu <= 0){
                player[1]--;
                return 35;
            }
    }
    return invu - 1 ;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

//affichage HUD, scoring
void hud(int score, int vies){
    int p = 1;
    char Cscore[50];
    sprintf(Cscore,"Score: %i", score);
    oled.writeString(7, 5, Cscore);
    
    for(int i = 0 ; i < vies ; i++){
            oled.writeBitmap(fullHeart, 7, 7, p, p + 8);
            p += 11;
        }
    for(int i = 0 ; i < VIES - vies ; i++){
            oled.writeBitmap(heart, 7, 7, p, p + 8);
            p += 11;
        }
}


//affichage de l'écran de fin (avec score) lorsque la vie tombe à 0
void gameOver(int score){ 
    char Cscore[50];
    sprintf(Cscore,"Score :%i", score);
    oled.writeString(3, 3, Cscore);
    oled.writeString(1, 3, "Game Over !");
}