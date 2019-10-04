FROM ubuntu
RUN apt-get update && apt-get install -y wget
RUN wget -O- https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/bin sh
RUN arduino-cli core update-index
RUN arduino-cli core install arduino:avr

