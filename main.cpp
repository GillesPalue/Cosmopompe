/*
 * Gilles 02/2021
 * 
 * test du Pro-mini :
 * 
 * branchement sur module USB->TTL vert :
 * - croiser RX et TX,
 * - alim sur 3.3V ou 5V (ou alors RAW qui adpate l'arrivée de courant en 3.3V
 *   ou 5V selon le modèle de pro mini).
 * - le RESET : une fois compilé, lancer le Run, et attendre que 
 *   le téléversement affiche ça :
 
 " avrdude.exe: Version 6.3, compiled on Jan 17 2017 at 12:00:53
             Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
             Copyright (c) 2007-2014 Joerg Wunsch

             System wide configuration file is "C:/ProgramData/Arduino/hardware/
                                                    tools/avr/etc/avrdude.conf"

             Using Port                    : COM7
             Using Programmer              : arduino
             Overriding Baud Rate          : 57600      "

 *  Et à ce moment-là, appuyer brièvement sur le bouton Reset du Pro-Mini
 * 
 * 
 * La consommation : une fois virée la led permanente + régulateur de tension,
 * on tourne à 20uA en dodo (voir photo de la carte bricolée)
 * 
 * POMPE ABEL OUED
 * - calibration du CPU par rapport au quartz
 * - si : - accu pompe > 10,5 V (pont diviseur tension)
 *        - ET il y a de l'eau à pomper (capteur eau)
 *   alors mise en route de la pompe 30 secondes.
 * - sommeil 24h
 * 
 * circuit : voir circuit 12V là :
 * https://www.gammon.com.au/forum/?id=12737&reply=1#reply1
 * 
 * conso mesurée en alim du pro-mini avec le circuit transistor + mosfet :
 * en veille (24h) : 66 uA (et 73 uA toutes les 8s, pour les micro-réveils)
 * en route (1 minute) : 7 mA
 * 
 */


#include <Arduino.h>
#include "Params.h"
#include "Dodo.h"
#include "Flotteur.h"


#ifdef MODE_DEBUG
    extern HardwareSerial Serial;
#endif

  
    /*************************************
    *              L'ACCU
    **************************************/
 


// voir https://www.gammon.com.au/forum/?id=12779&reply=1#reply1
bool is_accu_ok()
{    
    bool wIsAccuOK = false;
    
    pinMode (PIN_ACCU, INPUT);
    int value = analogRead(PIN_ACCU);
//    #ifdef MODE_DEBUG
//        Serial.print(F("value (0 to 1023 bits): ")); Serial.println(value);
//    #endif 
    int vout_mV = (int)(((long)value * 5000) / 1024);
//    #ifdef MODE_DEBUG
//        Serial.print(F("vout_mV : ")); Serial.println(vout_mV);
//    #endif 
    int vin_mV = vout_mV * (int)((long)(R1 + R2)/R2);
//    #ifdef MODE_DEBUG
//        Serial.print(F("vin_mV : ")); Serial.println(vin_mV);
//    #endif 
    if (vin_mV < 130) // c'est un bruît de fond
    {
        vin_mV = 0 ;
    }    
    wIsAccuOK = ((vin_mV - ACCU_TENSION_COUPURE_mV) > 0);
    #ifdef MODE_DEBUG
        Serial.print(F("_is_accu_ok : ")); Serial.print(wIsAccuOK);
        Serial.print(F(" \t(")); Serial.print(vin_mV);Serial.println(F(" mV)"));
    #endif    

    return wIsAccuOK;
}


    /*************************************
    *              LE POMPAGE
    **************************************/

void demarrer_pompe()
{    
    pinMode (PIN_POMPE, OUTPUT);
    digitalWrite (PIN_POMPE, HIGH);
    
    #ifdef MODE_DEBUG
        Serial.println(F("pompe demarree"));
    #endif
}
void arreter_pompe()
{
    digitalWrite (PIN_POMPE, LOW);
    pinMode (PIN_POMPE, INPUT);    
    
    #ifdef MODE_DEBUG
        Serial.println(F("pompe arretee"));
    #endif  
}


    /*************************************
    *              LE FLOTTEUR
    **************************************/

Flotteur* _flotteur;  // tout est géré dans cette classe




    /*************************************
    *              ACTION !
    **************************************/

void setup(void)
{
    #ifdef MODE_DEBUG
        Serial.begin(115200);
        Serial.println(F("reboot..."));    
    #endif

    _flotteur = new Flotteur();
    _flotteur->set_variable_tuyau_is_desamorce(true); // la première fois, le tuyau est desamorcé
}

void loop(void)
{      
    #ifdef MODE_DEBUG
        Serial.println();
    #endif
        
    if(is_accu_ok())
    {        
        if(_flotteur->is_eau_a_pomper())        
        {
            if(_flotteur->is_tuyau_desamorce())  // l'eau est remontée : YOYO
            {
                // on est dans le cas où l'eau est remontée depuis le
                // dernier réveil journalier ("yoyo" du siphon)                
                #ifdef MODE_DEBUG
                    Serial.println(F("YOYO JOURNALIER"));
                #endif

                demarrer_pompe();    
                Dodo::dodo_CPU(DUREE_POMPAGE_ms);                     
                arreter_pompe (); 
                _flotteur->set_variable_tuyau_is_desamorce(false);
            }
            else // le tuyau est resté amorcé = CRUE PERMANENTE
            {
                #ifdef MODE_DEBUG
                    Serial.println(F("CRUE PERMANENTE"));                    
                #endif
                
                // le tuyau est resté amorcé, donc la surveillance du flotteur
                // est toujours en vigueur
                _flotteur->set_variable_tuyau_is_desamorce(false); 
            }            
        }
        else // le siphon est vide, et l'eau n'est pas remontée = ETIAGE   
        {
            #ifdef MODE_DEBUG
                Serial.println(F("ETIAGE"));
            #endif 

            _flotteur->set_variable_tuyau_is_desamorce(true); 
        }        
    }
        
    // dodo
    Dodo::calibrer_Horloge_CPU();
    Dodo::dodo_CPU(DUREE_DODO_ms);
}
