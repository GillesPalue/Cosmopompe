/* 
 * File:   Params.h
 * Author: Gilles-PC
 *
 * Created on 1 décembre 2015, 08:51
 */

#ifndef PARAMS_H
#define	PARAMS_H

    /****************************************************************
     *                          LOG                                 *
     ***************************************************************/
    /* modes en commentaire => pas de sortie console, et gros gain de poids */
//    #define MODE_DEBUG 1 
    

    /****************************************************************
     *                          PIN                                 *
     ***************************************************************/
    #define PIN_POMPE               13
    #define PIN_ACCU                A4      // pont diviseur de tension 
    #define PIN_FLOTTEUR            2      // flotteur sur interrupt D2 INT0 : 1 = eau à pomper, 0 = pas d'eau

    /****************************************************************
     *                          MESURE DE L'ACCU                    *
     ***************************************************************/
    /* NB : si on utilise un pont diviseur de tension, 
    * préciser les valeurs des résistances en Ohms
    * 
    *  16V max  _______
    *                  |          
    *                [R1] = 30 kOhms
    *                  |__________________ U pinArduino (maxi 5V !)
    *                  |                   (on mesure 4V maxi, marge oblige, d'où 16 in fine)
    *                [R2] = 10 kOhms
    *          ________|
    *       0V   Masse
    */
    #define R2 10000.0  // 10 kOhms
    #define R1 3*R2 
    #define ACCU_TENSION_COUPURE_mV 10500   // le système arrête le pompage à 10,5V

    
    /****************************************************************
     *                          PARAMS DODO ET POMPAGE              *
     ***************************************************************/
    #define DUREE_DODO_ms 86400000 // 86400000 = 24h. Ce doit être un multiple de 16ms
    #define DUREE_POMPAGE_ms 64000 //  64000 = 1 minute. Ce doit être un multiple de 16ms

#endif	/* PARAMS_H */

