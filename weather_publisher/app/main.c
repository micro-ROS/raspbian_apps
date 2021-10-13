#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#define TEMPERATURE_SENSOR_ADDR 0x40
#define TEMPERATURE_MEASURE_COMMAND 0xF3
#define HUMIDITY_MEASURE_COMMAND 0xF5

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

pthread_mutex_t mutex;
float sensor_data[2];

void read_sensors(void *arg)
{
    char const temperature_command[1] = {TEMPERATURE_MEASURE_COMMAND};
    char const humidity_command[1] = {HUMIDITY_MEASURE_COMMAND};

    int fd = *(int*) arg;
    char data[4];

    while (1)
    {
        write(fd, temperature_command, 1);
        sleep(1);
        read(fd, &data[0], 2);

        write(fd, humidity_command, 1);
        sleep(1);
        read(fd, &data[2], 2);

        pthread_mutex_lock(&mutex);
        sensor_data[0] = (((data[0] * 256 + data[1]) * 175.72) / 65536.0) - 46.85;
        sensor_data[1] = (((data[2] * 256 + data[3]) * 125.0) / 65536.0) - 6.0;
        pthread_mutex_unlock(&mutex);

        printf("Got sensor data:\n");
        printf("  temperature: %.2f C\n", sensor_data[0]);
        printf("  humidity: %.2f RH\n", sensor_data[1]);
    }
}

int main(int argc, char const * const * argv)
{
  pthread_mutexattr_t mat;
  pthread_mutexattr_init(&mat);
  pthread_mutex_init(&mutex, &mat);

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
  rcl_node_t node;
  RCCHECK(rclc_node_init_default(&node, "weather_publisher", "", &support));

  rcl_publisher_t publisher;
  RCCHECK(rclc_publisher_init_best_effort(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "Sensor_data"));

  std_msgs__msg__Int32 sensor_topic;

  char * path = "/dev/i2c-1";
  int fd = open(path, O_RDWR);
  if (-1 == fd)
  {
    printf("Failed to open I2C bus.\n");
    exit(1);
  }

  ioctl(fd, I2C_SLAVE, TEMPERATURE_SENSOR_ADDR);

  pthread_t sensor_thread;
  pthread_create(&sensor_thread, NULL, (void *) &read_sensors, (void *) &fd);

  rcl_ret_t rc;
  do {
    pthread_mutex_lock(&mutex);
    sensor_topic.data = (int32_t) (sensor_data[0]*1000);
    sensor_topic.data += ((int32_t) (sensor_data[1]*1000)) << 16;
    pthread_mutex_unlock(&mutex);

    rc = rcl_publish(&publisher, (const void*)&sensor_topic, NULL);

    usleep(50000);

  } while (RCL_RET_OK == rc);

  pthread_exit(NULL);
  return 0;
}
