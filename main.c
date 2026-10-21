#include <stdint.h>
#include "stm32f10x.h"

void delay(uint32_t ticks) {
    for (uint32_t i = 0; i < ticks; i++) {
        __NOP();
    }
}

void led_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;
}

void buttons_init(void) {
    GPIOC->CRH &= ~(GPIO_CRH_CNF14 | GPIO_CRH_MODE14);
    GPIOC->CRH |= GPIO_CRH_CNF14_1;
    GPIOC->ODR |= (1U << 14);

    GPIOC->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
    GPIOC->CRH |= GPIO_CRH_CNF15_1;
    GPIOC->ODR |= (1U << 15);
}

int __attribute__((noreturn)) main(void) {
    led_init();
    buttons_init();

    int8_t freq_index = 0;
    const int8_t min_index = -6;
    const int8_t max_index = 6;

    uint8_t prev_a = 1;
    uint8_t prev_c = 1;

    while (1) {
        uint8_t a_pressed = !(GPIOC->IDR & (1U << 14));
        uint8_t c_pressed = !(GPIOC->IDR & (1U << 15));

        if (a_pressed && !prev_a) {
            if (freq_index < max_index) freq_index++;
        }
        if (c_pressed && !prev_c) {
            if (freq_index > min_index) freq_index--;
        }

        prev_a = a_pressed;
        prev_c = c_pressed;

        uint32_t delay_val;
        if (freq_index >= 0) {
            delay_val = 1000000 >> freq_index;
        } else {
            delay_val = 1000000 << (-freq_index);
        }

        GPIOC->ODR |= (1U << 13);
        delay(delay_val);
        GPIOC->ODR &= ~(1U << 13);
        delay(delay_val);
    }
}