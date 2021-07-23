/* 
 * File:   Flotteur.h
 * Author: Gilles
 *
 * Created on 18 juillet 2021, 20:50
 */

#ifndef FLOTTEUR_H
#define FLOTTEUR_H

#include <Arduino.h>

// le temps d'éliminer les rebonds du changement d'état du flotteur, en millisecondes  
#define FLOTTEUR_DUREE_ANTI_REBOND_ms   200  

class Flotteur
{
    public:
                                Flotteur            ( void );
                    void        set_variable_tuyau_is_desamorce  ( bool is_tuyau_desamorce );
                    bool        is_eau_a_pomper     ( void );
                    bool        is_tuyau_desamorce  ( void );
        virtual                 ~Flotteur           ( void );   
                
    private:
        volatile    bool        _is_tuyau_desamorce;     // variable volatile de l'interrupt
        static      Flotteur*   _instance;                    
                    void        tuyau_desamorced    ( void );
        static      void        isr                 ( void );

};

#endif /* FLOTTEUR_H */

