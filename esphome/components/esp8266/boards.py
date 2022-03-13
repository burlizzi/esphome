FLASH_SIZE_1_MB = 2**20
FLASH_SIZE_512_KB = FLASH_SIZE_1_MB // 2
FLASH_SIZE_2_MB = 2 * FLASH_SIZE_1_MB
FLASH_SIZE_4_MB = 4 * FLASH_SIZE_1_MB
FLASH_SIZE_16_MB = 16 * FLASH_SIZE_1_MB

ESP8266_FLASH_SIZES = {
    "d1": FLASH_SIZE_4_MB,
    "d1_mini": FLASH_SIZE_4_MB,
    "d1_mini_lite": FLASH_SIZE_1_MB,
    "d1_mini_pro": FLASH_SIZE_16_MB,
    "esp01": FLASH_SIZE_512_KB,
    "esp01_1m": FLASH_SIZE_1_MB,
    "esp07": FLASH_SIZE_4_MB,
    "esp12e": FLASH_SIZE_4_MB,
    "esp210": FLASH_SIZE_4_MB,
    "esp8285": FLASH_SIZE_1_MB,
    "esp_wroom_02": FLASH_SIZE_2_MB,
    "espduino": FLASH_SIZE_4_MB,
    "espectro": FLASH_SIZE_4_MB,
    "espino": FLASH_SIZE_4_MB,
    "espinotee": FLASH_SIZE_4_MB,
    "espmxdevkit": FLASH_SIZE_1_MB,
    "espresso_lite_v1": FLASH_SIZE_4_MB,
    "espresso_lite_v2": FLASH_SIZE_4_MB,
    "gen4iod": FLASH_SIZE_512_KB,
    "heltec_wifi_kit_8": FLASH_SIZE_4_MB,
    "huzzah": FLASH_SIZE_4_MB,
    "inventone": FLASH_SIZE_4_MB,
    "modwifi": FLASH_SIZE_2_MB,
    "nodemcu": FLASH_SIZE_4_MB,
    "nodemcuv2": FLASH_SIZE_4_MB,
    "oak": FLASH_SIZE_4_MB,
    "phoenix_v1": FLASH_SIZE_4_MB,
    "phoenix_v2": FLASH_SIZE_4_MB,
    "sonoff_basic": FLASH_SIZE_1_MB,
    "sonoff_s20": FLASH_SIZE_1_MB,
    "sonoff_sv": FLASH_SIZE_1_MB,
    "sonoff_th": FLASH_SIZE_1_MB,
    "sparkfunBlynk": FLASH_SIZE_4_MB,
    "thing": FLASH_SIZE_512_KB,
    "thingdev": FLASH_SIZE_512_KB,
    "wifi_slot": FLASH_SIZE_1_MB,
    "wifiduino": FLASH_SIZE_4_MB,
    "wifinfo": FLASH_SIZE_1_MB,
    "wio_link": FLASH_SIZE_4_MB,
    "wio_node": FLASH_SIZE_4_MB,
    "xinabox_cw01": FLASH_SIZE_4_MB,
}

ESP8266_LD_SCRIPTS = {
    FLASH_SIZE_512_KB: ("eagle.flash.512k0.ld", "eagle.flash.512k.ld"),
    FLASH_SIZE_1_MB: ("eagle.flash.1m0.ld", "eagle.flash.1m.ld"),
    FLASH_SIZE_2_MB: ("eagle.flash.2m.ld", "eagle.flash.2m.ld"),
    FLASH_SIZE_4_MB: ("eagle.flash.4m.ld", "eagle.flash.4m.ld"),
    FLASH_SIZE_16_MB: ("eagle.flash.16m.ld", "eagle.flash.16m14m.ld"),
}

ESP8266_BASE_PINS = {
    "A0": 17,
    "SS": 15,
    "MOSI": 13,
    "MISO": 12,
    "SCK": 14,
    "SDA": 4,
    "SCL": 5,
    "RX": 3,
    "TX": 1,
}

ESP8266_BOARD_PINS = {
    "d1": {
        "D0": 3,
        "D1": 1,
        "D2": 16,
        "D3": 5,
        "D4": 4,
        "D5": 14,
        "D6": 12,
        "D7": 13,
        "D8": 0,
        "D9": 2,
        "D10": 15,
        "D11": 13,
        "D12": 14,
        "D13": 14,
        "D14": 4,
        "D15": 5,
        "LED": 2,
    },
    "d1_mini": {
        "D0": 16,
        "D1": 5,
        "D2": 4,
        "D3": 0,
        "D4": 2,
        "D5": 14,
        "D6": 12,
        "D7": 13,
        "D8": 15,
        "LED": 2,
    },
    "d1_mini_lite": "d1_mini",
    "d1_mini_pro": "d1_mini",
    "esp01": {},
    "esp01_1m": {},
    "esp07": {},
    "esp12e": {},
    "esp210": {},
    "esp8285": {},
    "esp_wroom_02": {},
    "espduino": {"LED": 16},
    "espectro": {"LED": 15, "BUTTON": 2},
    "espino": {"LED": 2, "LED_RED": 2, "LED_GREEN": 4, "LED_BLUE": 5, "BUTTON": 0},
    "espinotee": {"LED": 16},
    "espmxdevkit": {},
    "espresso_lite_v1": {"LED": 16},
    "espresso_lite_v2": {"LED": 2},
    "gen4iod": {},
    "heltec_wifi_kit_8": "d1_mini",
    "huzzah": {
        "LED": 0,
        "LED_RED": 0,
        "LED_BLUE": 2,
        "D4": 4,
        "D5": 5,
        "D12": 12,
        "D13": 13,
        "D14": 14,
        "D15": 15,
        "D16": 16,
    },
    "inventone": {},
    "modwifi": {},
    "nodemcu": {
        "D0": 16,
        "D1": 5,
        "D2": 4,
        "D3": 0,
        "D4": 2,
        "D5": 14,
        "D6": 12,
        "D7": 13,
        "D8": 15,
        "D9": 3,
        "D10": 1,
        "LED": 16,
    },
    "nodemcuv2": "nodemcu",
    "oak": {
        "P0": 2,
        "P1": 5,
        "P2": 0,
        "P3": 3,
        "P4": 1,
        "P5": 4,
        "P6": 15,
        "P7": 13,
        "P8": 12,
        "P9": 14,
        "P10": 16,
        "P11": 17,
        "LED": 5,
    },
    "phoenix_v1": {"LED": 16},
    "phoenix_v2": {"LED": 2},
    "sonoff_basic": {},
    "sonoff_s20": {},
    "sonoff_sv": {},
    "sonoff_th": {},
    "sparkfunBlynk": "thing",
    "thing": {"LED": 5, "SDA": 2, "SCL": 14},
    "thingdev": "thing",
    "wifi_slot": {"LED": 2},
    "wifiduino": {
        "D0": 3,
        "D1": 1,
        "D2": 2,
        "D3": 0,
        "D4": 4,
        "D5": 5,
        "D6": 16,
        "D7": 14,
        "D8": 12,
        "D9": 13,
        "D10": 15,
        "D11": 13,
        "D12": 12,
        "D13": 14,
    },
    "wifinfo": {
        "LED": 12,
        "D0": 16,
        "D1": 5,
        "D2": 4,
        "D3": 0,
        "D4": 2,
        "D5": 14,
        "D6": 12,
        "D7": 13,
        "D8": 15,
        "D9": 3,
        "D10": 1,
    },
    "wio_link": {"LED": 2, "GROVE": 15, "D0": 14, "D1": 12, "D2": 13, "BUTTON": 0},
    "wio_node": {"LED": 2, "GROVE": 15, "D0": 3, "D1": 5, "BUTTON": 0},
    "xinabox_cw01": {"SDA": 2, "SCL": 14, "LED": 5, "LED_RED": 12, "LED_GREEN": 13},
}
