#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/tcs34725"

#define TCS34725_IOCTL_MAGIC      't'
#define TCS34725_IOCTL_READ_COLOR _IOR(TCS34725_IOCTL_MAGIC, 1, struct tcs34725_color)

struct tcs34725_color {
    unsigned short clear;
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};

int main() {
    int fd;
    struct tcs34725_color color_data;

    // Open the device
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
    while(1) {
    // Read color data using ioctl
    if (ioctl(fd, TCS34725_IOCTL_READ_COLOR, &color_data) < 0) {
        perror("Failed to read color data");
        close(fd);
        return errno;
    }

    printf("Color readings:\n");
    printf("Clear: %u\n", color_data.clear);
    printf("Red  : %u\n", color_data.red);
    printf("Green: %u\n", color_data.green);
    printf("Blue : %u\n", color_data.blue);
    }
    // Close the device
    close(fd);
    return 0;
}