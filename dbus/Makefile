#
# Makefile
#

all: dbus-server dbus-client

%.o: %.c
	gcc -Wall -c $< `pkg-config --cflags dbus-1`

dbus-server: dbus-server.o
	gcc dbus-server.o -o dbus-server `pkg-config --libs dbus-1`

dbus-client: dbus-client.o
	gcc dbus-client.o -o dbus-client `pkg-config --libs dbus-1`

.PHONY: clean
clean:
	rm *.o dbus-server dbus-client

