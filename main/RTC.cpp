
#include <driver/gpio.h>
#include <esp_timer.h>

#include <thread>

#include "Controller.h"
#include "String_format.h"

#include "RTC.h"

//******************************************************************************
//Адреса регистров DS1307
//******************************************************************************
typedef enum {

  DS1307_REG_SEC = 0x00,
  DS1307_REG_MIN,
  DS1307_REG_HOUR,
  DS1307_REG_DAY,
  DS1307_REG_DATE,
  DS1307_REG_MONTH,
  DS1307_REG_YEAR,
  DS1307_REG_RAM,

} t_ds1307_reg;

//******************************************************************************
//Результат выполнения операции с ds1307
//******************************************************************************
typedef enum {
  DS1307_SUCCESS = 0,
  DS1307_TIMEOUT,
  DS1307_ERROR
} t_ds1307_status;

//******************************************************************************
// Режим отображения часов AM/PM или 24-часовой DS1307
//******************************************************************************
typedef enum {
  DS1307_TIME_MODE_24H = 0,
  DS1307_TIME_MODE_AMPM
} t_ds1307_time_mode;

//******************************************************************************
// Структура для хранения времени и даты DS1307
//******************************************************************************
typedef struct {

  //Секунды
  struct {
    unsigned char seconds : 7;
    unsigned char ch : 1;
  };

  //Минуты
  unsigned char minutes;

  //Часы
  struct {
    unsigned char hours : 6;
    t_ds1307_time_mode mode : 1;
    unsigned char zero : 1;
  };

  //День недели
  unsigned char day;
  //Дата (число)
  unsigned char date;
  //Месяц
  unsigned char month;
  //Год
  unsigned char year;

} t_ds1307_date_time;

//******************************************************************************
// Прототипы функций
//******************************************************************************
extern t_ds1307_status ds1307_init(t_ds1307_time_mode time_mode);
extern t_ds1307_status ds1307_set(t_ds1307_date_time *dt);
extern t_ds1307_status ds1307_reset(t_ds1307_time_mode time_mode);
extern t_ds1307_status ds1307_get(t_ds1307_date_time *dt);

//******************************************************************************
// Константы
//******************************************************************************
static constexpr gpio_num_t RTC_seccond_irq_pin = GPIO_NUM_23;
static constexpr gpio_num_t SDA_PIN = GPIO_NUM_21;
static constexpr gpio_num_t SCL_PIN = GPIO_NUM_22;
static constexpr uint32_t I2C_SPEED = 100000;
static constexpr uint32_t Sync_Clock_EVERY_s = 60;

//******************************************************************************

#if 0
//******************************************************************************
//Перевод числа из десятичного представления в BCD-код
//******************************************************************************
static unsigned int ds1307_dec2bcd(unsigned int dec) {

  unsigned int temp = dec, result;

  result = (unsigned int)(temp / 1000) << 12;
  temp %= 1000;
  result |= (temp / 100) << 8;
  temp %= 100;
  result |= (temp / 10) << 4;
  result |= temp % 10;

  return result;
}

//******************************************************************************
//Перевод числа из BCD-кода в десятичное представление
//******************************************************************************
static unsigned int ds1307_bcd2dec(unsigned int bcd) {

  unsigned int temp = bcd, result;

  result = (temp >> 12) * 1000;
  temp &= 0x0FFF;
  result += (temp >> 8) * 100;
  temp &= 0x00FF;
  result += (temp >> 4) * 10;
  temp &= 0x000F;
  result += temp;

  return result;
}

//******************************************************************************
//Функция выдачи состояния выполнения операции ds1307 на основе состояния
//выполнения операции с I2C
//******************************************************************************
static t_ds1307_status ds1307_i2c_error(t_i2c_status status) {
  if (status != I2C_SUCCESS) {
    switch (status) {
    case I2C_TIMEOUT:
      return DS1307_TIMEOUT;
      break;
    case I2C_ERROR:
      return DS1307_ERROR;
      break;
    default:
      return DS1307_ERROR;
      break;
    }
  } else {
    return DS1307_SUCCESS;
  }
}

//******************************************************************************
//Проверка на наличие ошибок выполнения операции I2C
//******************************************************************************
#define ds1307_check_error(i2c_status)                                         \
  if (i2c_status != I2C_SUCCESS)                                               \
  return ds1307_i2c_error(i2c_status)

//******************************************************************************
//Инициализация часов DS1307
//******************************************************************************
t_ds1307_status ds1307_init(t_ds1307_time_mode time_mode) {

  //Состояние выполнения операции I2C
  t_i2c_status status = I2C_SUCCESS;

  //Переменная для хранения прочитанных данных
  t_ds1307_date_time data;

  //Инициализация RTC. Читаем 0-й регистр
  status = (DS1307_SLAVE_ADDR, DS1307_REG_SEC, (char *)&data, 1);

  //Проверка состояния выполнении операции по I2C
  ds1307_check_error(status);

  //Если работа часов запрещена, то разрешаем сбросом CH=0
  if (data.ch) {
    //Сброс даты и времени
    return ds1307_reset(time_mode);
  }

  return DS1307_SUCCESS;
}

//******************************************************************************
//Установка времени и даты DS1307
//******************************************************************************
t_ds1307_status ds1307_set(t_ds1307_date_time *dt) {

  //Состояние выполнения операции I2C
  t_i2c_status status = I2C_SUCCESS;

  //Не запрещаем работу часов
  dt->ch = 0;

  //Переводим дату и время в BCD-формат
  dt->seconds = ds1307_dec2bcd(dt->seconds);
  dt->minutes = ds1307_dec2bcd(dt->minutes);
  dt->hours = ds1307_dec2bcd(dt->hours);
  dt->day = ds1307_dec2bcd(dt->day);
  dt->date = ds1307_dec2bcd(dt->date);
  dt->month = ds1307_dec2bcd(dt->month);
  dt->year = ds1307_dec2bcd(dt->year);

  //Запись регистров ds1307
  status = i2c_wr_reg(DS1307_SLAVE_ADDR, DS1307_REG_SEC, (char *)dt,
                      sizeof(t_ds1307_date_time));

  //Проверка состояния выполнении операции по I2C
  ds1307_check_error(status);

  return DS1307_SUCCESS;
}

//******************************************************************************
//Сброс времени и даты DS1307
//******************************************************************************
t_ds1307_status ds1307_reset(t_ds1307_time_mode time_mode) {

  //Состояние выполнения операции I2C
  t_i2c_status status = I2C_SUCCESS;

  //Структура хранения времени и даты
  t_ds1307_date_time dt;

  //Включаем тактирование часов
  dt.ch = 0;

  //Сброс времени
  dt.seconds = 0;
  dt.minutes = 0;
  dt.hours = 0;
  dt.mode = time_mode;

  //Сброс даты
  dt.date = 1;
  dt.month = 1;
  dt.year = 00;

  //Запись регистров ds1307
  status = i2c_wr_reg(DS1307_SLAVE_ADDR, DS1307_REG_SEC, (char *)&dt,
                      sizeof(t_ds1307_date_time));

  //Проверка состояния выполнении операции по I2C
  ds1307_check_error(status);

  return DS1307_SUCCESS;
}

//******************************************************************************
//Чтение времени и даты из регистров DS1307
//******************************************************************************
t_ds1307_status ds1307_get(t_ds1307_date_time *dt) {

  //Состояние выполнения операции I2C
  t_i2c_status status = I2C_SUCCESS;

  t_ds1307_date_time datetime;

  //Чтение регистров ds1307
  status = i2c_rd_reg(DS1307_SLAVE_ADDR, DS1307_REG_SEC, (char *)&datetime,
                      sizeof(t_ds1307_date_time));

  //Проверка состояния выполнении операции по I2C
  ds1307_check_error(status);

  //Переводим дату и время из BCD-формата в обычный
  dt->seconds = ds1307_bcd2dec(datetime.seconds);
  dt->minutes = ds1307_bcd2dec(datetime.minutes);
  dt->hours = ds1307_bcd2dec(datetime.hours);
  dt->day = ds1307_bcd2dec(datetime.day);
  dt->date = ds1307_bcd2dec(datetime.date);
  dt->month = ds1307_bcd2dec(datetime.month);
  dt->year = ds1307_bcd2dec(datetime.year);

  return DS1307_SUCCESS;
}

//******************************************************************************
#endif

void RTCManager::register_rtc_interrupt() {
  gpio_pad_select_gpio(RTC_seccond_irq_pin);
  gpio_set_direction(RTC_seccond_irq_pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(RTC_seccond_irq_pin, GPIO_FLOATING);
  gpio_set_intr_type(RTC_seccond_irq_pin, GPIO_INTR_NEGEDGE);
  gpio_intr_enable(RTC_seccond_irq_pin);

  gpio_isr_handler_add(
      RTC_seccond_irq_pin,
      [](void *ctx) {
        xSemaphoreGiveFromISR(static_cast<SemaphoreHandle_t>(ctx), nullptr);
      },
      rtc_sem);
}

void RTCManager::load_clock() {}

void RTCManager::thread_func(RTCManager *self) {
  self->register_rtc_interrupt();
  while (true) {
    if (self->exitflag)
      return;
    if (xSemaphoreTake(self->rtc_sem, portMAX_DELAY) == pdTRUE) {
      std::time_t result{std::time(nullptr)};
      std::tm tm{*std::localtime(&result)};
      std::string s = format("%02d%02d%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

      if (self->cb) {
        self->cb(s);
      }

      ++self->sync_counter;
      if (self->sync_counter == Sync_Clock_EVERY_s) {
        self->load_clock();
      }
    }
  }
}

RTCManager::RTCManager(uint8_t rtc_addr)
    : rtc_sem(xSemaphoreCreateBinary()),
      exitflag(false), i2c{I2C_MODE_MASTER,     SDA_PIN,
                           GPIO_PULLUP_DISABLE, SCL_PIN,
                           GPIO_PULLUP_DISABLE, I2C_SPEED},
      sync_counter(0), clock_i2c_addr(rtc_addr) {}

RTCManager::~RTCManager() {
  exitflag = true;
  update_thread->join();
  i2c_driver_delete(I2C_NUM_0);
}

RTCManager &RTCManager::loadTime() {
  load_clock();
  return *this;
}

RTCManager &RTCManager::begin() {
  update_thread = new std::thread{&RTCManager::thread_func, this};
  i2c_param_config(I2C_NUM_0, &i2c);
  i2c_driver_install(I2C_NUM_0, i2c.mode, 0, 0, 0);
  return *this;
}

RTCManager &
RTCManager::setCallback(const RTCManager::onTimeUpdated &onTimeUpdated) {
  cb = onTimeUpdated;
  // cb(format("%06d", 138));
  return *this;
}
