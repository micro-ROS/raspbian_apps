#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define TEMPERATURE_SENSOR_ADDR 0x40
#define TEMPERATURE_MEASURE_COMMAND 0xF3
#define HUMIDITY_MEASURE_COMMAND 0xF5

void main()
{
    char * path = "/dev/i2c-1";
    int fd = open(path, O_RDWR);
    if (-1 == fd)
    {
        printf("Failed to open I2C bus.\n");
        exit(1);
    }

    ioctl(fd, I2C_SLAVE, TEMPERATURE_SENSOR_ADDR);

    char const temperature_command[1] = {TEMPERATURE_MEASURE_COMMAND};
    char const humidity_command[1] = {TEMPERATURE_MEASURE_COMMAND};
    char data[2];
    float temperature;
    float humidity;

    write(fd, temperature_command, 1);
    sleep(1);
    read(fd, data, 2);
    temperature = (((data[0] * 256 + data[1]) * 175.72) / 65536.0) - 46.85;

    write(fd, humidity_command, 1);
    sleep(1);
    read(fd, data, 2);
    humidity = (((data[0] * 256 + data[1]) * 125.0) / 65536.0) - 6.0;

    printf("Temperature: %.2f C\n", temperature);
    printf("Humidity: %.2f RH\n", humidity);
}