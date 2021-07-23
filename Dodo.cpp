/* 
 * File:   Dodo.cpp
 * Author: Gilles
 * 
 * Created on 02 février 2021, 20:32
 */

#include "Dodo.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

//Dodo::Dodo () { }
//
//Dodo::Dodo (const Dodo& orig) { }
//
//Dodo::~Dodo () { }


unsigned long   _tempsPasseADormir = 0; // temps total passé à dormir
float           _calibv = 0.9066311836; // ratio of real clock with WDT clock
volatile byte   _isrcalled = 0;         // WDT vector flag



// Internal function: Start watchdog timer
// byte psVal - Prescale mask
void WDT_On(byte psVal)
{
    // prepare timed sequence first
    byte ps = (psVal | (1 << WDIE)) & ~(1 << WDE);
    cli();
    wdt_reset();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1 << WDRF);
    // start timed sequence
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // set new watchdog timeout value
    WDTCSR = ps;
    sei();
}

// Internal function.  Stop watchdog timer
void WDT_Off()
{
    cli();
    wdt_reset();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1 << WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    sei();
}

// watchdog interrupt : 
// internal interrupt pour me réveiller moi-même toutes les n secondes
ISR(WDT_vect)
{
    WDT_Off();
    _isrcalled = 1;
}

// internal function. 
// voir http://gammon.com.au/forum/?id=11497&reply=6#reply6
// @param : le temps demandé à dormir
// @return : Le temps restant à dormir, qui ne tombe pas toujours sur zéro à la fin.
int sleep_CPU(const long &inTempsDemande)
{
    long wTempsRestant = inTempsDemande;
    byte WDTps = 9; // WDT Prescaler value, 9 = 8192ms

    _isrcalled = 0;

    // Do not interrupt before we go to sleep, or the
    // ISR will detach interrupts and we won't wake.
    // cf http://gammon.com.au/power Sketch J
//    noInterrupts();

    sleep_enable();
    while (wTempsRestant > 0)
    {
        //work out next prescale unit to use
        while ((0x10 << WDTps) > wTempsRestant && WDTps > 0)
        {
            WDTps--;
        }
        // send prescaler mask to WDT_On
        WDT_On((WDTps & 0x08 ? (1 << WDP3) : 0x00) | (WDTps & 0x07));
        _isrcalled = 0;
        while (_isrcalled == 0)
        {
            // turn bod off
            MCUCR |= (1 << BODS) | (1 << BODSE);
            MCUCR &= ~(1 << BODSE); // must be done right before sleep
            sleep_cpu();            // ici, le dodo
        }
        // calculer le temps restant
        wTempsRestant -= (0x10 << WDTps);
    }    
    // désactiver par précaution
    sleep_disable();

    return wTempsRestant;
}

// Delay function avec sleepTime multipe de 16ms
void Dodo::dodo_CPU (const long &inDureeTotaleDodo_ms)
{
    ADCSRA &= ~(1 << ADEN); // adc off
    PRR = 0xEF; // modules off

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    // le temps restant après le dodo ne tombe pas toujours sur zéro
    int wTempsRestant_ms = sleep_CPU(inDureeTotaleDodo_ms * _calibv);
    _tempsPasseADormir += (inDureeTotaleDodo_ms - wTempsRestant_ms);

    PRR = 0x00; //modules on
    ADCSRA |= (1 << ADEN); // adc on
}

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
 * quelques secondes jusqu'à 80s., pas testé au delà)
 * Sans calibrage préalable au dodo, la variation est de +/- 20% !
 */
void Dodo::calibrer_Horloge_CPU ()
{
    long wTemps1, wTemps2;
    set_sleep_mode(SLEEP_MODE_IDLE); // timer 0 (quartz) continue à tourner dans ce mode
    wTemps1 = micros();                   // un travail en micros() plutôt qu'en millis() donne de meilleurs résultats
    sleep_CPU(256);
    wTemps2 = micros();
    _calibv = 256000.0 / (wTemps2 - wTemps1);
}

