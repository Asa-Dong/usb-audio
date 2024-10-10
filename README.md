# USB UAC
本项目用 ESP32 开发板实现 USB 麦克风与扬声器功能，解决手边无麦克风的问题。

# 其它
本项目在 ESP32_S3_KORVO2 兼容版测试可用。
本人用的开发板，需要tca9554拉高引脚使能PA Chip，官方板或者完全兼容开发板的请注释掉这段代码。
```c
usb_audio.c

// io_expander tca9554 init
esp_io_expander_new_i2c_tca9554(0, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander);
esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_7,
                        IO_EXPANDER_OUTPUT);
// PA Chip NS4150 up. // only for this board
esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0, 1);
```