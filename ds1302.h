#ifndef DS1302_H
#define DS1302_H

#include <stdint.h>

// Estructura para representar la fecha y hora
typedef struct {
    uint8_t segundos;
    uint8_t minutos;
    uint8_t horas;
    uint8_t dia;
    uint8_t mes;
    uint8_t anio;  // últimos dos dígitos (00–99)
} rtc_time_t;

// Inicializa los pines GPIO y prepara el DS1302
void ds1302_init(void);

// Lee la hora actual del DS1302 y la guarda en la estructura
void ds1302_leer(rtc_time_t *t);

// (opcional) Función para escribir una hora nueva
void ds1302_escribir(const rtc_time_t *t);

#endif
