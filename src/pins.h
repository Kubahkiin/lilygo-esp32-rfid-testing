/**
 * \file pins.h
 * Definicje pinów
 */
#ifndef _PINS_H_
#define _PINS_H_

/** @name Elektrozamek
 * 
 */
///@{
constexpr uint8_t LOCK = 15;
constexpr uint8_t LOCK_SWITCH = 8;
///@}

/** @name Światła LED
 * 
 */
///@{
constexpr uint8_t RGB_1 = 17;
constexpr uint8_t RGB_2 = 18;
///@}

/** @name Czytnik RFID
 * 
 */
///@{
constexpr uint8_t READER_TX = 19;
constexpr uint8_t READER_RX = 20;
///@}

/** @name Buzzer
 * 
 */
///@{
//constexpr uint8_t BUZZER = 6;
///@}

/** @name Oświetlenie
 * 
 */
///@{
constexpr uint8_t LIGHT_1 = 40;
constexpr uint8_t LIGHT_2 = 41;
///@}

#endif
