.\qjsc.exe -c -m -M GPIO -N jslib_gpio -o bytecode\jslib_gpio.c src\gpio.js
.\qjsc.exe -c -m -N jslib_events -o bytecode\jslib_events.c src\events.js
.\qjsc.exe -c -m -M events -M CRYPTO -N jslib_crypto -o bytecode\jslib_crypto.c src\crypto.js
.\qjsc.exe -c -m -M events -M APPOTA -N jslib_appota -o bytecode\jslib_appota.c src\appota.js
.\qjsc.exe -c -m -M events -M AIOT_DEVICE -M AIOT_GATEWAY -N jslib_iot -o bytecode\jslib_iot.c src\iot.js
.\qjsc.exe -c -m -M events -M BT_HOST -N jslib_bt_host -o bytecode\jslib_bt_host.c src\bt_host.js
.\qjsc.exe -c -m -M REPL -M os -M std -N jslib_repl -o bytecode\jslib_repl.c src\repl.js
.\qjsc.exe -c -m -M events -M UART -N jslib_uart -o bytecode\jslib_uart.c src\uart.js
.\qjsc.exe -c -m -M events -M FS -N jslib_fs -o bytecode\jslib_fs.c src\fs.js
.\qjsc.exe -c -m -M events -M AUDIOPLAYER -N jslib_audioplayer -o bytecode\jslib_audioplayer.c src\audioplayer.js
.\qjsc.exe -c -m -M kv -N jslib_device -o bytecode\jslib_device.c src\device.js
.\qjsc.exe -c -m -M events -M NETMGR -N jslib_netmgr -o bytecode\jslib_netmgr.c src\netmgr.js
.\qjsc.exe -c -m -M events -M NETWORK -M NETMGR -M CELLULAR -N jslib_network -o bytecode\jslib_network.c src\network.js
.\qjsc.exe -c -m -M events -M MQTT -N jslib_mqtt -o bytecode\jslib_mqtt.c src\mqtt.js
.\qjsc.exe -c -m -M events -M TCP -N jslib_tcp -o bytecode\jslib_tcp.c src\tcp.js
.\qjsc.exe -c -m -M NETWORK -M NETMGR -M CELLULAR -N jslib_location -o bytecode\jslib_location.c src\location.js
