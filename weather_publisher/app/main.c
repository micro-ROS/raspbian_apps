#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <sensor_msgs/msg/laser_echo.h>

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define TEMPERATURE_SENSOR_ADDR 0x40
#define TEMPERATURE_MEASURE_COMMAND 0xF3
#define HUMIDITY_MEASURE_COMMAND 0xF5

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

int main(int argc, char const * const * argv)
{
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
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, LaserEcho),
      "Float__Sequence"));

  float sensor_data[2];
  sensor_msgs__msg__LaserEcho sensor_topic;
  sensor_topic.echoes.capacity = 2;
  sensor_topic.echoes.size = 2;
  sensor_topic.echoes.data = sensor_data;

  char * path = "/dev/i2c-1";
  int fd = open(path, O_RDWR);
  if (-1 == fd)
  {
    printf("Failed to open I2C bus.\n");
    exit(1);
  }

  ioctl(fd, I2C_SLAVE, TEMPERATURE_SENSOR_ADDR);

  char const temperature_command[1] = {TEMPERATURE_MEASURE_COMMAND};
  char const humidity_command[1] = {HUMIDITY_MEASURE_COMMAND};
  char data[2];

  rcl_ret_t rc;
  do {
    write(fd, temperature_command, 1);
    sleep(1);
    read(fd, data, 2);
    sensor_data[0] = (((data[0] * 256 + data[1]) * 175.72) / 65536.0) - 46.85;

    write(fd, humidity_command, 1);
    sleep(1);
    read(fd, data, 2);
    sensor_data[1] = (((data[0] * 256 + data[1]) * 125.0) / 65536.0) - 6.0;

    rc = rcl_publish(&publisher, (const void*)&sensor_topic, NULL);
    if (RCL_RET_OK == rc)
    {
      printf("Published:\n");
      printf("  temperature: %.2f C\n", sensor_data[0]);
      printf("  humidity: %.2f RH\n", sensor_data[1]);
    }

  } while (RCL_RET_OK == rc);

  return 0;
}
