/*
  MHZ19.h - MH-Z19 CO2 sensor library for ESP-WROOM-02/32(ESP8266/ESP32) or
  Arduino version 1.0

  License MIT
*/

#ifndef _MHZ19
#define _MHZ19

#include <stdint.h>

#include <driver/gpio.h>
#include <driver/uart.h>

// https://github.com/crisap94/MHZ19

enum MHZ19_UART_DATA { PPM, TEMPERATURE, STAT };

enum MHZ19_PWM_DATA { CALC_2000_PPM, CALC_5000_PPM };

class MHZ19 {
public:
  static const char LOG_TAG[];

  MHZ19(const uart_port_t uart_num, const gpio_num_t rx, const gpio_num_t tx);
  ~MHZ19();

  void setAutoCalibration(bool autocalib);
  void calibrateZero();
  void calibrateSpan(int ppm);

  int getPPM();
  int getTemperature();
  int getStatus();

  bool isWarming();

protected:
  void writeCommand(const uint8_t com[]);
  void writeCommand(const uint8_t com[], uint8_t response[]);

private:
  static uint8_t mhz19_checksum(const uint8_t com[]);
  int getSerialData(MHZ19_UART_DATA flg);

  static constexpr int REQUEST_CNT = 8;
  static constexpr int RESPONSE_CNT = 9;

  static constexpr int MHZ_19_UART_BUTRATE = 9600;
  static constexpr int UART_BUFFER_SIZE = 132;

  // serial command
  static const uint8_t getppm[REQUEST_CNT];
  static const uint8_t zerocalib[REQUEST_CNT];
  static const uint8_t spancalib[REQUEST_CNT];
  static const uint8_t autocalib_on[REQUEST_CNT];
  static const uint8_t autocalib_off[REQUEST_CNT];

  const uart_port_t uart_num;
  QueueHandle_t uartQueue;
};

#endif
