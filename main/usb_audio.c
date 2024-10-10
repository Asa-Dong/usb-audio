/* USB Audio device
*/
#include <string.h>
#include <usb_device_uac.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "i2s_stream.h"
#include "board.h"
#include "esp_io_expander_tca9554.h"
#include "driver/i2s_std.h"

static const char *TAG = "USB_AUDIO";

static audio_element_handle_t i2s_stream_writer;
static audio_element_handle_t i2s_stream_reader;
static audio_board_handle_t board_handle = NULL;

static esp_err_t uac_device_output_cb(uint8_t *buf, size_t len, void *arg) {
    // ESP_LOGE(TAG, "i2s output");

    audio_element_output(i2s_stream_writer, (char *) buf, len);

    return ESP_OK;
}

static esp_err_t uac_device_input_cb(uint8_t *buf, size_t len, size_t *bytes_read, void *arg) {
    audio_element_err_t res = audio_element_input(i2s_stream_reader, (char *) buf, len);
    if (res > 0) {
        *bytes_read = res;
        // ESP_LOGI(TAG, "i2s read %d %p", res, arg);
    } else {
        *bytes_read = 0;
        ESP_LOGE(TAG, "i2s read fail %d %p", res, arg);
    }
    return ESP_OK;
}

static void uac_device_set_mute_cb(uint32_t mute, void *arg) {
    (void) mute;
    (void) arg;
    ESP_LOGI(TAG, "uac_device_set_mute_cb: %"PRIu32"", mute);
    audio_hal_set_mute(board_handle->audio_hal, 90);
}

static void uac_device_set_volume_cb(uint32_t volume, void *arg) {
    (void) volume;
    (void) arg;
    ESP_LOGI(TAG, "uac_device_set_volume_cb: %"PRIu32"", volume);

    // audio_hal_set_volume(board_handle->audio_hal, 90);
}

static esp_io_expander_handle_t io_expander = NULL;


void app_main(void) {
    ESP_LOGI(TAG, "[1] Start codec chip");
    board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    int volume = 10;
    audio_hal_set_volume(board_handle->audio_hal, 90);
    audio_hal_get_volume(board_handle->audio_hal, &volume);
    ESP_LOGI(TAG, "Get volume:%.2d", volume);

    ESP_LOGI(TAG, "[2] Create i2s stream to write data to codec chip. rate:%d", CONFIG_UAC_SAMPLE_RATE);

    // i2s writer [I2S_NUM_0]
    {
        // sample_rate need same as usb
        i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT_WITH_PARA(I2S_NUM_0, CONFIG_UAC_SAMPLE_RATE,
                                                                    I2S_DATA_BIT_WIDTH_16BIT, AUDIO_STREAM_WRITER);
        i2s_cfg.type = AUDIO_STREAM_WRITER;
        i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    }
    // i2s mic read [I2S_NUM_1]
    {
        i2s_stream_cfg_t i2s_cfg2 = I2S_STREAM_CFG_DEFAULT_WITH_PARA(I2S_NUM_0, CONFIG_UAC_SAMPLE_RATE,
                                                                    I2S_DATA_BIT_WIDTH_16BIT, AUDIO_STREAM_READER);
        i2s_cfg2.type = AUDIO_STREAM_READER;
        i2s_cfg2.out_rb_size = 16 * 1024;
        i2s_stream_reader = i2s_stream_init(&i2s_cfg2);
    }

    // io_expander tca9554 init
    esp_io_expander_new_i2c_tca9554(0, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander);
    esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_7,
                            IO_EXPANDER_OUTPUT);
    // PA Chip NS4150 up. // only for this board
    esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0, 1);

    uac_device_config_t config = {
        .output_cb = uac_device_output_cb,
        .input_cb = uac_device_input_cb,
        // .set_mute_cb = uac_device_set_mute_cb,
        // .set_volume_cb = uac_device_set_volume_cb,
        .cb_ctx = NULL,
    };

    uac_device_init(&config);
}
