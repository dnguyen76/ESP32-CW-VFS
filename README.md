# ESP32-CW-VFS
 software for ESP32 to generate CW with Volume, Speed and Frequency user settings
 
 This is an ESP32-CW software enhanced version. User can modify settings for Volume, Speed of Morse sequence and also CW generated frequency (Continuous Wave). These settings are implemented by adding supplementary characteristics to UART Blutooth LE service
 
 Also PWM feature of ESP32 is used ( instead of delay() in ESP32-CW) to generate the Morse sequence

ESP32 generate CW (Morse) ; Text to be converted in Morse, is written in an android application using Bluetooth connectio Tis text is forwarded to an ESP32 board connected to a buzzer. Any ESP32 board can be used, GPIO pins for LED and Buzzer must be modified in Arduino software

esp32_ble_X_PWM_VSF.ino : software for ESP32 DEVKIT MH-ET-LIVE board ( approx 6€ ) a buzzer is attached to GPIO pin 22 and LED pin 2 is also used (to be compiled with Arduino IDE with ESP32 support)

BLE_ESP32_CW_VSF.aia : source to AppInventor2 . Can be used to modifie apk file BLE_ESP32_CW_VSF.apk : For android smartphone compatible with android 5.0 and upper  This is the User interface communicating with ESP32 board through Bluetooth LE connection

Nota: To install ESP32 support in Arduino IDE follow these instructions https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md
