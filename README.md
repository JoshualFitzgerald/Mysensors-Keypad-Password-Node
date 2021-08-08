# Mysensors-Keypad-Password-Node
An Arduino keypad&amp;password sketch to use with Mysensors.org  network and a home controller eg. Openhab.org
Features- Keyapd 3x4, LCD 1602, motion sensor SR505, door switch
        - 6 different users/passwords 
        - Passwords can be set from the controller, need to be set prior to first use, are stored in eeprom memeory
        - Password entry must be set within 12 seconds or automatically resets.
        - * to reset password
        - # to enter password
        - User Names can be set from the controller, remain in eeprom memory
        - 20 different zones/items can be operated from the keypad using a combination of the user password and zone extenion 01-20 
        - Password takes 6 digits, extension 2 digits 01 through 20. Password with zone extensions  12345601, 12345606, 12345617, 12345620.
        - Zone access for each zone for each user can be set from the controller, must be set  prior to first use, stored in eeprom memory
        - Zone Names can be set from the controller, these are not stored in eeprom, will revert to coded default name on reboot
        - Motion sensor  will remain in triggered state for 30 seconds after triggering.
 
