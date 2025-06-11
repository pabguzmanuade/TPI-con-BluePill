#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/systick.h>
#include <stdbool.h>
#include <stdio.h>
#include "ds1302.h"

// --- Macros de pines ---
#define LED_PIN     GPIO13
#define LED_PORT    GPIOC
#define BUTTON_PIN  GPIO0
#define BUTTON_PORT GPIOA

#define AJUSTAR_RELOJ 0

// --- Variables ---
volatile uint32_t ms_ticks = 0;
bool boton_anterior = false;
uint32_t fichada_id = 1;

// --- SysTick delay ---
void sys_tick_handler(void) { ms_ticks++; }
void delay_ms(uint32_t ms) {
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

// --- UART ---
void uart_init(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

    usart_set_baudrate(USART1, 9600);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_enable(USART1);
}

void uart_print(const char *str) {
    while (*str) {
        usart_send_blocking(USART1, *str++);
    }
}

// --- GPIO ---
void gpio_init(void) {
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
    gpio_set_mode(BUTTON_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_PIN);
    gpio_set(BUTTON_PORT, BUTTON_PIN); // Activar pull-up
}

bool leer_boton(void) {
    return !(gpio_get(BUTTON_PORT, BUTTON_PIN)); // Activo en bajo
}

// --- Main ---
int main(void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    systick_set_reload(72000 - 1); // 1 ms
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_clear();
    systick_counter_enable();
    systick_interrupt_enable();

    gpio_init();
    uart_init();
    ds1302_init();

    //SE AJUSTA POR UNICA VEZ CAMBIANDO A 1 EN EL #DEFINE DE LA LINEA 15
    #if AJUSTAR_RELOJ
    rtc_time_t nueva_hora = {
        .segundos = 0,
        .minutos  = 50,
        .horas    = 17,
        .dia      = 8,
        .mes      = 6,
        .anio     = 25
    };
    ds1302_escribir(&nueva_hora);
    #endif

    uart_print("Sistema iniciado.\r\n");

    while (1) {
        bool boton = leer_boton();
        if (boton && !boton_anterior) {
            gpio_toggle(LED_PORT, LED_PIN);

            rtc_time_t tiempo;
            ds1302_leer(&tiempo);

            char buffer[64];
            snprintf(buffer, sizeof(buffer),
                     "%02lu, %02d:%02d:%02d\r\n",
                     fichada_id++, tiempo.horas, tiempo.minutos, tiempo.segundos);
            uart_print(buffer);

            delay_ms(200); // Antirebote
        }
        boton_anterior = boton;
    }

    return 0;
}
