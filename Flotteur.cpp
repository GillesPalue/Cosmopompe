/* 
 * File:   Flotteur.cpp
 * Author: Gilles
 * 
 * Created on 18 juillet 2021, 20:50
 * 
 * La valeur du flotteur indique deux choses différentes :
 * - s'il y a de l'eau à pomper, par une mesure directe du pin, selon :
 *   0 = boucle fermé = eau à pomper, 1 = ouvert = pas d'eau
 * - si le tuyau a désamorcé, par une ISR qui prend note quand le flotteur passe hors d'eau,
 *  donc quand la valeur du pin passe de LOW () à HIGH
 */

#include "Flotteur.h"
#include "Params.h"

Flotteur::Flotteur()
{
    pinMode (PIN_FLOTTEUR, INPUT_PULLUP);    
    _instance = this;
    
    #ifdef MODE_DEBUG
        Serial.print(F("Constructeur Flotteur OK"));
    #endif
}

/*
 * pin branché sur port digital
 * autre fil branché au GND
 * normalement on met une résistance de pull-up (sinon état de la lecture aléatoire)
 * mais ici on se sert de la résistance interne de l'arduino, qu'on définit comme pull-up avant lecture
 * puis qu'on remet en pull-down après lecture (sinon consomme un peu de jus, environ 130 uA)
 * 
 * @return : 
 * false : plus d'eau à pomper = contact ouvert (pin à HIGH)
 * true : eau à pomper = contact fermé (pin à LOW)
 * 
 * voir https://zestedesavoir.com/tutoriels/686/arduino-premiers-pas-en-informatique-embarquee/743_gestion-des-entrees-sorties/3423_un-simple-bouton/
 */
bool Flotteur::is_eau_a_pomper()
{
    bool wIsEauAPomper;
    
//    // on suspend l'interrupt du flotteur durant la lecture, à cause du tirage en pull-down du pin
    detachInterrupt(digitalPinToInterrupt(PIN_FLOTTEUR));
//    Note Gilles 2021/07/22 : pas utile si on interroge is_tuyau_desamorcé avant
    
    digitalWrite(PIN_FLOTTEUR, HIGH);
    // 0 = LOW = boucle fermé = eau à pomper,     1 = HIGH = ouvert = pas d'eau
    wIsEauAPomper = (digitalRead(PIN_FLOTTEUR) == LOW); 
    digitalWrite(PIN_FLOTTEUR, LOW); // une fois la lecture faite, on tire vers le bas, pull-down, pour économiser le jus
       
    
    #ifdef MODE_DEBUG
        Serial.print(F("is_eau_a_pomper : ")); Serial.println(wIsEauAPomper);
    #endif

    return wIsEauAPomper;
}

/*
 * Cette fonction dit si le tuyau s'est désamorcé pendant le sommeil de la pompe
 * 
 * @return :
 * contact ouvert = tuyau desamorce = return true
 * contact fermé = tuyau encore en eau = return false
 */
bool Flotteur::is_tuyau_desamorce()
{
    // voir http://gammon.com.au/forum/?id=11488&reply=7#reply7
    bool wIsTuyauDesamorce;
    byte oldSREG = SREG;                     // remember if interrupts are on or off (sauvegarde du registre)
    noInterrupts ();                         // on suspend les interruptions
    wIsTuyauDesamorce = _is_tuyau_desamorce; // on copie la variable volatile
    SREG = oldSREG;                          // on remet les interruptions en route, là où elles en sont avant la manip    
        
    #ifdef MODE_DEBUG
        Serial.print(F("Flotteur::is_tuyau_desamorce() : ")); Serial.println(wIsTuyauDesamorce);
    #endif

     return wIsTuyauDesamorce;
}

// ISR glue routine
void Flotteur::isr()
{
    _instance->tuyau_desamorced();
}

// à utiliser par la routine glue de l'ISR
Flotteur* Flotteur::_instance;

/*
 * durant le dodo, si le flotteur descend (= tuyau désamorce),
 * un micro-réveil passera la variable _is_tuyau_desamorce à true,
 * et la surveillance du flotteur sera désactivée, pour éviter 
 * d'autres micro-réveils inutiles.
 */
void Flotteur::tuyau_desamorced()
{
    // on fait un léger filtre anti-rebonds. 
    // Voir http://forum.arduino.cc/index.php?topic=45000.0
//    static unsigned long last_interrupt_time = 0;
//    unsigned long interrupt_time = millis();
//    if (interrupt_time - last_interrupt_time > FLOTTEUR_DUREE_ANTI_REBOND_ms)
//    {
//    Gilles 2021/07/22 : Pas utile si on detache l'interrupt
    
        byte oldSREG = SREG;                     // remember if interrupts are on or off (sauvegarde du registre)
        noInterrupts ();                         // on suspend les interruptions
        _is_tuyau_desamorce = true;
        SREG = oldSREG;                          // on remet les interruptions en route, là où elles en sont avant la manip    
        
        // on desactive l'interrupt du flotteur : de toute manière le tuyau a désamorcé
        // On le réactivera plus tard, lors du réveil de la pompe
        detachInterrupt(digitalPinToInterrupt(PIN_FLOTTEUR));
        
//        last_interrupt_time = interrupt_time;
//    }
}

/*
 * réamorçage de la surveillance du flotteur avec un interrupt * 
 * @param : renseigner si le tuyau est déamorcé au pas
 */
void Flotteur::set_variable_tuyau_is_desamorce(bool is_tuyau_desamorce)
{
    if(!is_tuyau_desamorce)
    {
        // quand le flotteur s'ouvre (= pas d'eau), le pin passe de LOW (eau) à HIGH (pas d'eau), 
        // c'est donc un changement d'état RISING (LOW → HIGH)
        attachInterrupt(digitalPinToInterrupt(PIN_FLOTTEUR), isr, RISING);
    }

    // dit à la variable que le flotteur est sous l'eau
    byte oldSREG = SREG;                     // remember if interrupts are on or off (sauvegarde du registre)
    noInterrupts ();                         // on suspend les interruptions
    _is_tuyau_desamorce = is_tuyau_desamorce;
    SREG = oldSREG;                          // on remet les interruptions en route, là où elles en sont avant la manip   
}

Flotteur::~Flotteur()
{
    _instance->~Flotteur();
}
