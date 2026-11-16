#include <stdint.h>
#include "stm32f10x.h"

void delay(uint32_t ticks) {
    for (uint32_t i = 0; i < ticks; i++) {
        __NOP();
    }
}

void SPI1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;
    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5 | GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (0xB << 20) | (0xB << 28);
    GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 | GPIO_CRL_MODE2 | GPIO_CRL_CNF2 | GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
    GPIOA->CRL |= (0x3 << 4) | (0x3 << 8) | (0x3 << 16);
    GPIOA->BSRR = GPIO_BSRR_BS4 | GPIO_BSRR_BS1;
    GPIOA->BSRR = GPIO_BSRR_BR2; 
    delay(10000); 
    GPIOA->BSRR = GPIO_BSRR_BS2;
    delay(10000);
    SPI1->CR1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_MSTR | SPI_CR1_BR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE;
}

void SPI1_Write(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
    while (!(SPI1->SR & SPI_SR_TXE)); 
}

void display_cmd(uint8_t cmd) {
    GPIOA->BSRR = GPIO_BSRR_BR4; 
    GPIOA->BSRR = GPIO_BSRR_BR1; 
    delay(10);
    SPI1_Write(cmd);
    GPIOA->BSRR = GPIO_BSRR_BS4; 
}

void display_data(uint8_t data) {
    GPIOA->BSRR = GPIO_BSRR_BR4; 
    GPIOA->BSRR = GPIO_BSRR_BS1; 
    delay(10);
    SPI1_Write(data);
    GPIOA->BSRR = GPIO_BSRR_BS4; 
}

void SSD1306_Init(void) {
    display_cmd(0xAE); display_cmd(0x20); display_cmd(0x00);
    display_cmd(0xB0); display_cmd(0xC8); display_cmd(0x00);
    display_cmd(0x10); display_cmd(0x40); display_cmd(0x81); display_cmd(0xFF);
    display_cmd(0xA1); display_cmd(0xA6); display_cmd(0xA8); display_cmd(0x3F);
    display_cmd(0xA4); display_cmd(0xD3); display_cmd(0x00); display_cmd(0xD5);
    display_cmd(0xF0); display_cmd(0xD9); display_cmd(0x22); display_cmd(0xDA);
    display_cmd(0x12); display_cmd(0xDB); display_cmd(0x20); display_cmd(0x8D);
    display_cmd(0x14); display_cmd(0xAF); 
}

const uint8_t font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x50, 0x30, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x60, 0x60, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x21, 0x41, 0x45, 0x4B, 0x31, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03, 0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49, 0x29, 0x1E
};

uint8_t bitrev(uint8_t b){
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);
    return b;
}

void PutChar(char x) {
    const uint8_t *pData = &font[(x - 32) * 5];
    for(int i=0; i<5; i++) {
        display_data(bitrev(*pData));
        pData++;
    }
    display_data(0x00);
}

void DrawDigit(uint8_t digit) {
    display_cmd(0xB0 + 3); 
    display_cmd(0x00);     
    display_cmd(0x10);     
    PutChar('0' + digit);  
}

volatile uint8_t count = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF; 
        count++;
        if (count > 9) count = 0; 
        DrawDigit(count);         
    }
}

int main(void) {
    SPI1_Init();
    SSD1306_Init();
    DrawDigit(0); 
    
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
    TIM2->PSC = 8000 - 1;
    TIM2->ARR = 1000 - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;

    while (1) {
    }
}