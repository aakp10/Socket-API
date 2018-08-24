all: server client
server: server.c
	gcc -o server server.c -lsystemd
client: client.c

install:
	install -m 644 echo-server.socket /etc/systemd/system/echo-server.socket
	install -m 644 echo-server.service /etc/systemd/system/echo-server.service

clean:
	rm -rf client server