simple autoclicker for linux

This operates directly on input devices and does not use any desktop-specific APIs, meaning that it works with wayland, x11 or even the tty.

Configuration is done by modifying config.h and recompiling\
Make sure that you have uinput loaded (ls /dev/uinput)\
Add your user to the "input" group, otherwise you need to run as root
