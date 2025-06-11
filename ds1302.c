#include "ds1302.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// --- Pines del DS1302 (pueden modificarse según conexiones reales) ---
#define DS1302_PORT GPIOA
#define DS1302_RST  GPIO4
#define DS1302_CLK  GPIO5
#define DS1302_IO   GPIO6

// --- Comandos DS1302 ---
#define DS1302_CMD_CLOCK_BURST_READ  0xBF
#define DS1302_CMD_CLOCK_BURST_WRITE 0xBE

// --- Utilidades BCD ---
static uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// --- Control de pin bidireccional IO ---
static void ds1302_io_output(void) {
    gpio_set_mode(DS1302_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, DS1302_IO);
}

static void ds1302_io_input(void) {
    gpio_set_mode(DS1302_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, DS1302_IO);
}

// --- Bit-banging para enviar y recibir bytes ---
static void ds1302_write_byte(uint8_t data) {
    ds1302_io_output();
    for (int i = 0; i < 8; i++) {
        if (data & 0x01) {
            gpio_set(DS1302_PORT, DS1302_IO);
        } else {
            gpio_clear(DS1302_PORT, DS1302_IO);
        }
        gpio_set(DS1302_PORT, DS1302_CLK);
        data >>= 1;
        gpio_clear(DS1302_PORT, DS1302_CLK);
    }
}

static uint8_t ds1302_read_byte(void) {
    uint8_t data = 0;
    ds1302_io_input();
    for (int i = 0; i < 8; i++) {
        if (gpio_get(DS1302_PORT, DS1302_IO)) {
            data |= (1 << i);
        }
        gpio_set(DS1302_PORT, DS1302_CLK);
        gpio_clear(DS1302_PORT, DS1302_CLK);
    }
    return data;
}

// --- Inicio del módulo DS1302 ---
void ds1302_init(void) {
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_set_mode(DS1302_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, DS1302_RST | DS1302_CLK | DS1302_IO);
    gpio_clear(DS1302_PORT, DS1302_CLK);
    gpio_clear(DS1302_PORT, DS1302_RST);
}

// --- Leer hora completa en modo burst ---
void ds1302_leer(rtc_time_t *t) {
    gpio_set(DS1302_PORT, DS1302_RST);
    ds1302_write_byte(DS1302_CMD_CLOCK_BURST_READ);

    uint8_t raw[8];
    for (int i = 0; i < 8; i++) {
        raw[i] = ds1302_read_byte();
    }

    gpio_clear(DS1302_PORT, DS1302_RST);

    t->segundos = bcd_to_dec(raw[0] & 0x7F);
    t->minutos  = bcd_to_dec(raw[1]);
    t->horas    = bcd_to_dec(raw[2] & 0x3F); // formato 24h
    t->dia      = bcd_to_dec(raw[3]);
    t->mes      = bcd_to_dec(raw[4]);
    t->anio     = bcd_to_dec(raw[6]);       // año (00–99)
}
// --- Escribir en un registro individual del DS1302 ---
static void ds1302_write_register(uint8_t reg, uint8_t value) {
    gpio_set(DS1302_PORT, DS1302_RST);
    ds1302_write_byte((reg << 1) | 0x80); // Dirección de escritura
    ds1302_write_byte(value);
    gpio_clear(DS1302_PORT, DS1302_RST);
}

void ds1302_escribir(const rtc_time_t *t) {
    // Desactivar protección de escritura
    ds1302_write_register(0x0E, 0x00);  // WP = 0

    gpio_set(DS1302_PORT, DS1302_RST);
    ds1302_write_byte(DS1302_CMD_CLOCK_BURST_WRITE);

    ds1302_write_byte(dec_to_bcd(t->segundos) & 0x7F); // Asegura CH = 0
    ds1302_write_byte(dec_to_bcd(t->minutos));
    ds1302_write_byte(dec_to_bcd(t->horas));
    ds1302_write_byte(dec_to_bcd(t->dia));
    ds1302_write_byte(dec_to_bcd(t->mes));
    ds1302_write_byte(0); // Día de semana (sin usar)
    ds1302_write_byte(dec_to_bcd(t->anio));
    ds1302_write_byte(0); // Registro de control (podés dejar en 0)

    gpio_clear(DS1302_PORT, DS1302_RST);
}
