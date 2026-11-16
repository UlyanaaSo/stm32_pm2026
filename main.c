#include <stdint.h>
#include "stm32f10x.h"

void delay_cycles(volatile uint32_t count) {
    while(count--) { __asm("nop"); }
}

void SPI1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;

    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5 | GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (0xB << 20) | (0xB << 28);

    GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 | GPIO_CRL_MODE2 | GPIO_CRL_CNF2 | GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
    GPIOA->CRL |= (0x3 << 4) | (0x3 << 8) | (0x3 << 16);

    GPIOA->BSRR = GPIO_BSRR_BS4 | GPIO_BSRR_BS1;
    GPIOA->BSRR = GPIO_BSRR_BR2; 
    delay_cycles(100000);
    GPIOA->BSRR = GPIO_BSRR_BS2;
    delay_cycles(100000);

    SPI1->CR1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_MSTR | 
                SPI_CR1_BR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE;
}

void SPI1_Write(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
    while (!(SPI1->SR & SPI_SR_TXE)); 
}

uint8_t SPI1_Read(void) {
    SPI1->DR = 0;
    while(!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

void display_cmd(uint8_t cmd) {
    GPIOA->BSRR = GPIO_BSRR_BR4; 
    GPIOA->BSRR = GPIO_BSRR_BR1; 
    delay_cycles(100);
    SPI1_Write(cmd);
    GPIOA->BSRR = GPIO_BSRR_BS4; 
}

void display_data(uint8_t data) {
    GPIOA->BSRR = GPIO_BSRR_BR4; 
    GPIOA->BSRR = GPIO_BSRR_BS1; 
    delay_cycles(100);
    SPI1_Write(data);
    GPIOA->BSRR = GPIO_BSRR_BS4; 
}

void SSD1306_Init(void) {
    display_cmd(0xAE); 
    display_cmd(0x20); 
    display_cmd(0x00); 
    display_cmd(0xB0); 
    display_cmd(0xC8); 
    display_cmd(0x00); 
    display_cmd(0x10); 
    display_cmd(0x40); 
    display_cmd(0x81); 
    display_cmd(0xFF); 
    display_cmd(0xA1); 
    display_cmd(0xA6); 
    display_cmd(0xA8); 
    display_cmd(0x3F); 
    display_cmd(0xA4); 
    display_cmd(0xD3); 
    display_cmd(0x00); 
    display_cmd(0xD5); 
    display_cmd(0xF0); 
    display_cmd(0xD9); 
    display_cmd(0x22); 
    display_cmd(0xDA); 
    display_cmd(0x12);
    display_cmd(0xDB); 
    display_cmd(0x20); 
    display_cmd(0x8D); 
    display_cmd(0x14); 
    display_cmd(0xAF); 
}

void draw_chessboard(void) {
    for (uint8_t page = 0; page < 8; page++) {
        display_cmd(0xB0 + page); 
        display_cmd(0x00);        
        display_cmd(0x10);        
        for (uint8_t col = 0; col < 128; col++) {
            if (col % 2 == 0) display_data(0xAA);
            else display_data(0x55);
        }
    }
}

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

        SPI1_Init();
    SSD1306_Init();
    draw_chessboard();

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