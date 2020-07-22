# Nixeie clock ESP32



# Known issies

1. Не собирается драйвер ds1307 в ESP-IDF 4.3
    Обход: в файле components/esp-idf-lib/components/i2cdev/i2cdev.h добавить `inline static`
    к функциям `i2c_dev_read_reg()` и `i2c_dev_write_reg()` (http://gudok.xyz/inline/)
2. В версии IDF 4.3 изменилась структура `struct tm` фикс года больше не нужен
