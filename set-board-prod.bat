@echo off
echo Configuring for PRODUCTION board...

powershell -Command "(Get-Content '.\02-develop\02-fw\water_controller_esp32_pio_arduino\platformio.ini') -replace 'default_envs = .*', 'default_envs = esp32dev' | Set-Content '.\02-develop\02-fw\water_controller_esp32_pio_arduino\platformio.ini'"
powershell -Command "(Get-Content '.\02-develop\02-fw\water_controller_esp32_pio_arduino\include\Config.h') -replace '#define DEV_BOARD', '//#define DEV_BOARD' | Set-Content '.\02-develop\02-fw\water_controller_esp32_pio_arduino\include\Config.h'"

echo Done. Project is now set for the PRODUCTION board (esp32dev).
