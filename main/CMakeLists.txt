file(GLOB_RECURSE srcs "src/*.c")
idf_component_register(SRCS "main.c"
                            "source/bluetooth.c"
                            "source/buzzer.c"
                            "source/keypad.c"
                            "source/ldr.c"
                            "source/light.c"
                            "source/servo.c"
                            "source/ultrasonic.c"
                            "source/led.c"
                       INCLUDE_DIRS "." "include" "source"
                       REQUIRES bt nvs_flash driver esp_adc)

