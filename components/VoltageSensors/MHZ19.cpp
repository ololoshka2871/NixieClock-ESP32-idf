/*
  MHZ19.cpp - MH-Z19 CO2 sensor library for ESP8266 or Arduino
  version 1.0

  License MIT
*/

#include <thread>

#include <esp_log.h>

#include "MHZ19.h"

#define WAIT_READ_TIMES 100
#define WAIT_READ_DELAY 10

const char MHZ19::LOG_TAG[] = "MHZ19";

const uint8_t MHZ19::getppm[REQUEST_CNT] = {0xff, 0x01, 0x86, 0x00,
                                            0x00, 0x00, 0x00, 0x00};
const uint8_t MHZ19::zerocalib[REQUEST_CNT] = {0xff, 0x01, 0x87, 0x00,
                                               0x00, 0x00, 0x00, 0x00};
const uint8_t MHZ19::spancalib[REQUEST_CNT] = {0xff, 0x01, 0x88, 0x00,
                                               0x00, 0x00, 0x00, 0x00};
const uint8_t MHZ19::autocalib_on[REQUEST_CNT] = {0xff, 0x01, 0x79, 0xA0,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8_t MHZ19::autocalib_off[REQUEST_CNT] = {0xff, 0x01, 0x79, 0x00,
                                                   0x00, 0x00, 0x00, 0x00};

static size_t avalable(uart_port_t uart_num) {
  size_t res;
  ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, &res));
  return res;
}

// public

MHZ19::MHZ19(const uart_port_t uart_num, const gpio_num_t rx,
             const gpio_num_t tx)
    : uart_num(uart_num) {
  uart_config_t uart_config{
      MHZ_19_UART_BUTRATE,      /* UART baud rate*/
      UART_DATA_8_BITS,         /* UART byte size*/
      UART_PARITY_DISABLE,      /* UART parity mode*/
      UART_STOP_BITS_1,         /* UART stop bits*/
      UART_HW_FLOWCTRL_DISABLE, /* UART HW flow control mode (cts/rts)*/
      0                         /* UART HW RTS threshold*/
  };

  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(
      uart_set_pin(uart_num, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(uart_num, UART_BUFFER_SIZE,
                                      UART_BUFFER_SIZE, 10, &uartQueue, 0));
}

MHZ19::~MHZ19() { uart_driver_delete(uart_num); }

void MHZ19::setAutoCalibration(bool autocalib) {
  writeCommand(autocalib ? autocalib_on : autocalib_off);
}

void MHZ19::calibrateZero() { writeCommand(zerocalib); }

void MHZ19::calibrateSpan(int ppm) {
  if (ppm < 1000)
    return;

  uint8_t cmd[MHZ19::REQUEST_CNT];
  for (int i = 0; i < MHZ19::REQUEST_CNT; i++) {
    cmd[i] = spancalib[i];
  }
  cmd[3] = (uint8_t)(ppm / 256);
  cmd[4] = (uint8_t)(ppm % 256);
  writeCommand(cmd);
}

int MHZ19::getPPM() { return getSerialData(MHZ19_UART_DATA::PPM); }

int MHZ19::getTemperature() {
  return getSerialData(MHZ19_UART_DATA::TEMPERATURE);
}

int MHZ19::getStatus() { return getSerialData(MHZ19_UART_DATA::STAT); }

bool MHZ19::isWarming() { return (getStatus() <= 1); }

// protected
void MHZ19::writeCommand(const uint8_t cmd[]) { writeCommand(cmd, nullptr); }

void MHZ19::writeCommand(const uint8_t cmd[], uint8_t *response) {
  const auto checksumm = mhz19_checksum(cmd);

  ESP_ERROR_CHECK(uart_wait_tx_done(uart_num, 1));

  uart_write_bytes(uart_num, reinterpret_cast<const char *>(cmd), REQUEST_CNT);
  uart_write_bytes(uart_num, reinterpret_cast<const char *>(&checksumm),
                   REQUEST_CNT);

  ESP_ERROR_CHECK(uart_wait_tx_done(uart_num, 10));

  if (response != nullptr) {
    int i = 0;
    while (avalable(uart_num) <= 0) {
      if (++i > WAIT_READ_TIMES) {
        ESP_LOGE(LOG_TAG, "error: can't get MH-Z19 response.");
        return;
      }
      std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_READ_DELAY));
    }
    uart_read_bytes(uart_num, response, MHZ19::RESPONSE_CNT, 10);
  }
}

// private

int MHZ19::getSerialData(MHZ19_UART_DATA flg) {
  uint8_t buf[MHZ19::RESPONSE_CNT];
  for (int i = 0; i < MHZ19::RESPONSE_CNT; i++) {
    buf[i] = 0x0;
  }

  writeCommand(getppm, buf);
  int co2 = 0, co2temp = 0, co2status = 0;

  // parse
  if (buf[0] == 0xff && buf[1] == 0x86 &&
      mhz19_checksum(buf) == buf[MHZ19::RESPONSE_CNT - 1]) {
    co2 = buf[2] * 256 + buf[3];
    co2temp = buf[4] - 40;
    co2status = buf[5];
  } else {
    co2 = co2temp = co2status = -1;
  }

  switch (flg) {
  case MHZ19_UART_DATA::TEMPERATURE:
    return co2temp;
    break;
  case MHZ19_UART_DATA::STAT:
    return co2status;
    break;
  case MHZ19_UART_DATA::PPM:
  default:
    return co2;
    break;
  }
}

uint8_t MHZ19::mhz19_checksum(const uint8_t com[]) {
  uint8_t sum = 0x00;
  for (int i = 1; i < MHZ19::REQUEST_CNT; i++) {
    sum += com[i];
  }
  sum = 0xff - sum + 0x01;
  return sum;
}
