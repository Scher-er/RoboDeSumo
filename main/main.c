// Teste de motores - Robô de Sumô (ESP32 + TB6612FNG) - ESP-IDF v5.x
// Pinagem do manual v7. STBY está ligado fixo no 5V (não usa GPIO).
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "motor_test";

// ---- Pinos ----
#define PIN_PWMA 25   // motor esquerdo (AO1/AO2)
#define PIN_AIN1 26
#define PIN_AIN2 27
#define PIN_PWMB 33   // motor direito (BO1/BO2)
#define PIN_BIN1 32
#define PIN_BIN2 13

// ---- PWM ----
#define PWM_FREQ_HZ  20000
#define PWM_RES      LEDC_TIMER_10_BIT
#define PWM_MAX      1023
#define TEST_SPEED   100      // velocidade do teste em % (suba depois de validar)
#define STEP_MS      2000    // duração de cada movimento
#define PAUSE_MS     1000    // pausa entre movimentos

typedef struct {
    gpio_num_t in1;
    gpio_num_t in2;
    ledc_channel_t ch;
} motor_t;

static motor_t motor_left  = { PIN_AIN1, PIN_AIN2, LEDC_CHANNEL_0 };
static motor_t motor_right = { PIN_BIN1, PIN_BIN2, LEDC_CHANNEL_1 };

static void motors_init(void)
{
    // Pinos de direção
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_AIN1) | (1ULL << PIN_AIN2) |
                        (1ULL << PIN_BIN1) | (1ULL << PIN_BIN2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    gpio_set_level(PIN_AIN1, 0);
    gpio_set_level(PIN_AIN2, 0);
    gpio_set_level(PIN_BIN1, 0);
    gpio_set_level(PIN_BIN2, 0);

    // Timer PWM
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    // Canais PWM (um por motor)
    ledc_channel_config_t ch_a = {
        .gpio_num = PIN_PWMA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = motor_left.ch,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config_t ch_b = ch_a;
    ch_b.gpio_num = PIN_PWMB;
    ch_b.channel = motor_right.ch;
    ESP_ERROR_CHECK(ledc_channel_config(&ch_a));
    ESP_ERROR_CHECK(ledc_channel_config(&ch_b));
}

// speed: -100 (ré total) a +100 (frente total). 0 = parado.
static void motor_set(const motor_t *m, int speed)
{
    if (speed > 100)  speed = 100;
    if (speed < -100) speed = -100;

    if (speed > 0) {
        gpio_set_level(m->in1, 1);
        gpio_set_level(m->in2, 0);
    } else if (speed < 0) {
        gpio_set_level(m->in1, 0);
        gpio_set_level(m->in2, 1);
    } else {
        gpio_set_level(m->in1, 0);
        gpio_set_level(m->in2, 0);
    }

    uint32_t duty = (uint32_t)abs(speed) * PWM_MAX / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, m->ch, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, m->ch);
}

static void drive(const char *label, int left, int right)
{
    ESP_LOGI(TAG, "%s  (esq=%d%%, dir=%d%%)", label, left, right);
    motor_set(&motor_left, left);
    motor_set(&motor_right, right);
    vTaskDelay(pdMS_TO_TICKS(STEP_MS));

    motor_set(&motor_left, 0);
    motor_set(&motor_right, 0);
    vTaskDelay(pdMS_TO_TICKS(PAUSE_MS));
}

void app_main(void)
{
    motors_init();

    // Regra do sumô: 5 s de espera antes de qualquer movimento
    for (int i = 5; i > 0; i--) {
        ESP_LOGI(TAG, "Começa em %d s... (robô com as rodas no ar!)", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    drive("1. Motor ESQUERDO - frente", TEST_SPEED, 0);
    drive("2. Motor ESQUERDO - ré",    -TEST_SPEED, 0);
    drive("3. Motor DIREITO  - frente", 0, TEST_SPEED);
    drive("4. Motor DIREITO  - ré",     0, -TEST_SPEED);
    drive("5. Os dois - frente",  TEST_SPEED,  TEST_SPEED);
    drive("6. Os dois - ré",     -TEST_SPEED, -TEST_SPEED);
    drive("7. Giro no lugar (esq frente, dir ré)",  TEST_SPEED, -TEST_SPEED);
    drive("8. Giro no lugar (esq ré, dir frente)", -TEST_SPEED,  TEST_SPEED);

    ESP_LOGI(TAG, "Teste concluído. Aperte EN no ESP32 para repetir.");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
