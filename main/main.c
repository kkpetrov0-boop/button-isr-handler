#include <stdio.h>
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#define GPIO_INPUT_PIN 4
#define DEBOUNCE_TIME_MS 200

volatile uint32_t last_interrupt_time = 0;

static void button_isr_handler(void* arg);

QueueHandle_t button_queue = NULL;

void app_main(void)
{
    gpio_install_isr_service(0);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_INPUT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_conf);
    
    button_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_isr_handler_add(GPIO_INPUT_PIN, button_isr_handler, (void *)GPIO_INPUT_PIN);

    while(1)
    {
        uint32_t recieved_pin;
        if (xQueueReceive(button_queue, &recieved_pin, portMAX_DELAY) == pdTRUE)
    {
        printf("Кнопка на пине %lu нажата\n", recieved_pin);
    }
        

    }
}

static void IRAM_ATTR button_isr_handler(void* arg)
{
    // Здесь можно обработать прерывание от кнопки, если нужно
    uint32_t pin_num  = (uint32_t)arg;

    uint32_t current_time = xTaskGetTickCountFromISR();
    if (current_time - last_interrupt_time > DEBOUNCE_TIME_MS / portTICK_PERIOD_MS)
    {
        xQueueSendFromISR(button_queue, &pin_num, NULL);
        last_interrupt_time = current_time;        
    }
    
}
