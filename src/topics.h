/**
 * \file topics.h
 * Definicje topiców mqtt w postaci zmiennych, w celu łatwej modyfikacji i porządku w programie.
 */
#ifndef _TOPICS_H_
#define _TOPICS_H_

/** @name Zmienne wartości topiców
 * 
 */
///@{
/** Znak oznaczający miejsce */
String asp_c = "0";
/** Znak oznaczający urządzenie */
String asm_c = "3";
///@}

/**
 *  Sklejenie początku topicu ze zmiennymi częściami w jeden łańcuch znaków
 */
String location = "asps/asp" + asp_c + "/asm" + asm_c + "/";

/** @name Topici na które przychodzą żądania
 * 
 */
///@{
/** Czy drzwi są otwarte? */
String door_status_request = location + "door/status/request";
/** Otwarcie elektrozamka */
String door_permit = location + "door/permit";
/** Przprowadzenie inwentaryzacji */
String reader_read_request = location + "reader/read/request";
///@}

/** @name Topici na które lodówka przesyła wiadomości
 * 
 */
///@{
/** Informacja czy drzwi są otwarte */
String door_status = location + "door/status";
/** Wynik inwentaryzacji w postaci listy EPC */
String reader_read_tags = location + "reader/read/tags";
///@}

/** @name Topici diagonostyczne input
 * 
 */
///@{
/** Temperatura czytnika */
String diagnostics_reader_temperature_request = location + "diagnostics/reader/temperature/request";
/** Informacje o czytniku */
String diagnostics_reader_info_request = location + "diagnostics/reader/info/request"; 
/** Detekcja anten */
String diagnostics_reader_antenna_detection_request = location + "diagnostics/reader/antennas/request";
/** Temperatura w lodówce */
String diagnostics_environment_temperature_request = location + "diagnostics/environment/temperature/request";
/** Wilgotność powietrza w lodówce */
String diagnostics_environment_humidity_request = location + "diagnostics/environment/humidity/request";
///@}

/** @name Topici diagonostyczne output
 * 
 */
///@{
/** Timestamp */
String diagnostics_timestamp = location + "diagnostics/timestamp";
/** Temperatura czytnika */
String diagnostics_reader_temperature = location + "diagnostics/reader/temperature";
/** Informacje o czytniku */
String diagnostics_reader_info = location + "diagnostics/reader/info"; 
/** Detekcja anten */
String diagnostics_reader_antenna_detection = location + "diagnostics/reader/antennas";
/** Temperatura w lodówce */
String diagnostics_environment_temperature = location + "diagnostics/environment/temperature";
/** Wilgotność powietrza w lodówce */
String diagnostics_environment_humidity = location + "diagnostics/environment/humidity";
///@}

#endif