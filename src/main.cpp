// Модуль 2.3: три світлодіоди з різними інтервалами без delay()
// GPIO18 -> 220 Ом -> LED1 -> GND   (200 мс)
// GPIO17 -> 220 Ом -> LED2 -> GND   (500 мс)
// GPIO16 -> 220 Ом -> LED3 -> GND   (1000 мс)
#include <Arduino.h>

struct Config {
    static constexpr uint8_t LED1_PIN = 18;
    static constexpr uint8_t LED2_PIN = 17;
    static constexpr uint8_t LED3_PIN = 16;
    static constexpr uint32_t LED1_MS = 200;
    static constexpr uint32_t LED2_MS = 500;
    static constexpr uint32_t LED3_MS = 1000;
    static constexpr uint32_t REPORT_MS = 10000;
    static constexpr uint32_t BAUD = 115200;
};

class BlinkLed {
public:
    constexpr BlinkLed(uint8_t pin, uint32_t interval) : pin_(pin), interval_(interval) {}

    void init() const {
        pinMode(pin_, OUTPUT);
        digitalWrite(pin_, LOW);
    }

    // Викликається на кожному оберті loop(). Якщо ще не час, одразу виходить.
    // now - last_ (а не now >= last_ + interval_) правильно працює і після
    // переповнення millis().
    void update(uint32_t now) {
        if (now - last_ < interval_)
            return;
        last_ = now;
        on_ = !on_;
        digitalWrite(pin_, on_ ? HIGH : LOW);
        ++toggles_;
    }

    uint8_t pin() const { return pin_; }

    uint32_t takeToggles() {
        const uint32_t n = toggles_;
        toggles_ = 0;
        return n;
    }

private:
    const uint8_t pin_;
    const uint32_t interval_;
    uint32_t last_ = 0;
    uint32_t toggles_ = 0;
    bool on_ = false;
};

BlinkLed leds[] = {
    {Config::LED1_PIN, Config::LED1_MS},
    {Config::LED2_PIN, Config::LED2_MS},
    {Config::LED3_PIN, Config::LED3_MS},
};

uint32_t lastReport = 0;

// Раз на 10 с друкує, скільки разів перемкнувся кожен світлодіод.
void report(uint32_t now) {
    if (now - lastReport < Config::REPORT_MS)
        return;
    lastReport = now;
    Serial.printf("%lu мс · за 10 с:", static_cast<unsigned long>(now));
    for (auto &led : leds)
        Serial.printf("  GPIO%u = %lu", led.pin(), static_cast<unsigned long>(led.takeToggles()));
    Serial.println();
}

void setup() {
    Serial.begin(Config::BAUD);
    for (auto &led : leds)
        led.init();
}

void loop() {
    // Один millis() на оберт: усі три світлодіоди порівнюють себе з тією самою миттю.
    const uint32_t now = millis();
    for (auto &led : leds)
        led.update(now);
    report(now);
}
