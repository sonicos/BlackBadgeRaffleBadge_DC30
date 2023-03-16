# DC30 Black Badge Raffle Badge #
Free-range artisan Black Badge Raffle Badge for DEFCON 30. Now arsenic free!

Everything I have is included here from when I flashed the badges in Vegas.

The current SDKCONFIG will give you a "normal" badge configured to use encrypted flash. To use encryption, youll need to Flash Fuse with
`C:/Espressif/python_env/idf4.4_py3.8_env/Scripts/python.exe C:\Espressif\frameworks\esp-idf-v4.4.1\components\esptool_py\esptool\espefuse.py  burn_key BLOCK_KEY0  .\my_flash_encryption_key.bin XTS_AES_128_KEY --port COMX`

I used VSCode to build this.

I MAY or may not update this README further later on. If you have any issues, open an issue and I'll try to respond. 

Important things to note. Anything that is good and working likely came from code I ~~stole~~ borrowed from the (And!Xor DC26 Bender Badge)[https://github.com/ANDnXOR/ANDnXOR_DC26]. Anything that is broken is probably mine. You'll find a LOT of code that isn't used by my project as I didn't take the time to clean up after myself.

Thank you to the Black Badge Raffle qualifiers for playing along. Thank you to DT for not 86'ing me. Thank you to the rest of you for not beating my ass.

Find me on Twitter @DefconRaffle

-Sonicos

## Waiver ##
I take no responsibility for what you do or do not do with this code. If you compile this, and your badge becomes sentient and tries to eat your cat, that's entirely on you. You have been warned.