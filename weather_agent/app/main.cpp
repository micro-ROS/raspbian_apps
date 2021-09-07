#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#include <uxr/agent/transport/serial/TermiosAgentLinux.hpp>
#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

#include <termios.h>
#include <fcntl.h>
#include <fstream>

int main(int argc, char** argv)
{
    // TODO: use args for UDP port?
    uint16_t port = 8888;
    std::string port_name;

    // Read serial port from file
    std::ifstream port_file("/tmp/uros_port.log");

    if (port_file.is_open())
    {
        getline (port_file, port_name);
        port_file.close();
    }
    else
    {
        std::cout << "Bridge script not initialized" << std::endl;
        return -1;
    }

    struct termios attr;

    attr.c_cflag |= unsigned(CREAD);    // Enable read.
    attr.c_cflag |= unsigned(CLOCAL);   // Set local mode.
    attr.c_cflag &= unsigned(~PARENB);  // Disable parity.
    attr.c_cflag &= unsigned(~CSTOPB);  // Set one stop bit.
    attr.c_cflag &= unsigned(~CSIZE);   // Mask the character size bits.
    attr.c_cflag |= unsigned(CS8);      // Set 8 data bits.
    attr.c_cflag &= unsigned(~CRTSCTS); // Disable hardware flow control.

    /* Setting LOCAL OPTIONS. */
    attr.c_lflag &= unsigned(~ICANON);  // Set non-canonical input.
    attr.c_lflag &= unsigned(~ECHO);    // Disable echoing of input characters.
    attr.c_lflag &= unsigned(~ECHOE);   // Disable echoing the erase character.
    attr.c_lflag &= unsigned(~ISIG);    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

    /* Setting INPUT OPTIONS. */
    attr.c_iflag &= unsigned(~IXON);    // Disable output software flow control.
    attr.c_iflag &= unsigned(~IXOFF);   // Disable input software flow control.
    attr.c_iflag &= unsigned(~INPCK);   // Disable parity check.
    attr.c_iflag &= unsigned(~ISTRIP);  // Disable strip parity bits.
    attr.c_iflag &= unsigned(~IGNBRK);  // No ignore break condition.
    attr.c_iflag &= unsigned(~IGNCR);   // No ignore carrier return.
    attr.c_iflag &= unsigned(~INLCR);   // No map NL to CR.
    attr.c_iflag &= unsigned(~ICRNL);   // No map CR to NL.

    /* Setting OUTPUT OPTIONS. */
    attr.c_oflag &= unsigned(~OPOST);   // Set raw output.

    /* Setting OUTPUT CHARACTERS. */
    attr.c_cc[VMIN] = 1;
    attr.c_cc[VTIME] = 1;

    /* Setting baudrate. */
    speed_t baudrate = getBaudRate("115200");
    attr.c_ispeed = baudrate;
    attr.c_ospeed = baudrate;

    eprosima::uxr::UDPv4Agent agent_udp4(port, eprosima::uxr::Middleware::Kind::CED);
    agent_udp4.set_verbose_level(6);
    agent_udp4.start();

    eprosima::uxr::TermiosAgent agent_serial(port_name.c_str(), O_RDWR | O_NOCTTY, attr, 0, eprosima::uxr::Middleware::Kind::CED);
    agent_serial.set_verbose_level(6);
    agent_serial.start();

    while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }

    return 0;
}