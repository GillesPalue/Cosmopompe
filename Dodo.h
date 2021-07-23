/* 
 * File:   Dodo.h
 * Author: Gilles
 * 
 * Created on 02 février 2021, 20:32
 * 
 * gère :
 * - la mise en sommeil,
 * - le calibrage du CPU sur le quartz avant mise en sommeil
 * - les interrupts
 */

#ifndef DODO_H
#define DODO_H

#include <Arduino.h>

class Dodo
{
    public:
        /* 
        * ATTENTION : CETTE FONCTION ENDORT LE CPU PENDANT 256ms !
        * 
        * principe du calibrage :
        * 
        * - le quartz donne une mesure précise du temps écoulé,
        * - le CPU en donne une valeur approximative, qui dépend de la température, 
        * du voltage, etc.
        * 
        * PROBLEME : en mode sommeil total (SLEEP_MODE_PWR_DOWN), le quartz est coupé,
        * seul fonctionne encore le CPU 
        * 
        * SOLUTION : avant de passer en sommeil total, on fait une mesure précise d'un
        * intervalle de temps en mode sommeil partiel (256ms en SLEEP_MODE_IDLE), on 
        * multipliera ensuite le temps donné par le CPU par le facteur de calibrage.
        * 
        * A NOTER : On fait l'approximation qu'il n'y a pas de variation importante de
        * température ou de voltage du CPU pendant son sommeil !
        * 
        * RESULTAT DES TESTS : environ 6 ms d'erreur sur UN sommeil (qu'il soit de 
        * quelques secondes jusqu'à 80s., pas testé au delà).
        * Sans calibrage préalable au dodo, la variation est de +/- 20% !
        */
        static void calibrer_Horloge_CPU ( void );
        
        /*
        * mise en sommeil du CPU pour un délai souhaité (multiple de 16ms).
        * ATTENTION : faire un calibrateCPU() avant !
        * 
        * ATTENTION : 
        * Quand le CPU dort, millis() s'arrête.
        */
        static void dodo_CPU ( const long &inDureeTotaleDodo_ms );
        
};

#endif /* DODO_H */

