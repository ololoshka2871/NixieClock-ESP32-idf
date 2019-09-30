#include <ctime>

#include "String_format.h"

#include "FastLED.h"
#include "Nixie.h"

#include "DateDisplayer.h"

static const CRGB month2Color[] = {
    {0, 0, 0xff},       // Январь - чисто синий
    {0, 0xbf, 0xff},    // Февраль более голубой
    {0, 0xff, 0xff},    // март - светло-голубой
    {0x99, 0xff, 0xe6}, // апрель - бледно-зеленый
    {0, 0xff, 0},       // май - зеленый
    {0xff, 0xff, 0x99}, // июнь светло-желтый
    {0xff, 0, 0},       // июль - красный
    {0xff, 0xb3, 0x66}, // август - оранжевый
    {0xcc, 0xcc, 0},    // сентябрь - желтый
    {0x99, 0x4d, 0},    // октябрь - коричневый
    {0xA3, 0, 0xcc},    // ноябрь - фиолетовый
    {0x77, 0x33, 0xff}  // декабрь - сине-фиолетовый
};

void DateDisplay::enter(InterfaceButton *btn, Nixie *indicators,
                        CFastLED *leds) {

  AbstractGUIState::enter(btn, indicators, leds);

  std::time_t result{std::time(nullptr)};
  std::tm tm{*std::gmtime(&result)};
  std::string s =
      format("%02d%02d%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);

  indicators->setValue(s);

  leds->showColor(month2Color[tm.tm_mon]);
}
