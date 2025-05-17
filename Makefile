CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lmosquitto

TARGET = mud_daemon
SERVICE = mud.service
CONFIG_DIR = /etc/mud

all: $(TARGET)

$(TARGET): mud_daemon.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

install:
	sudo install -d -m 755 $(CONFIG_DIR)
	sudo install -m 755 $(TARGET) /usr/local/bin/
	sudo install -m 644 $(SERVICE) /etc/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable $(SERVICE)

uninstall:
	sudo systemctl stop $(SERVICE) || true
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -f /etc/systemd/system/$(SERVICE)
	sudo systemctl daemon-reload

.PHONY: all clean install uninstall