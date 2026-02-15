
#include <seashell.h>

#include <serial/serial.h>

serial::Serial port;

int timer_enabled = 0;

size_t shell_flush(const char *payload, size_t len)
{
  return port.write(reinterpret_cast<const uint8_t *>(payload),len);
}

int cmd_echo(int argc, const char **argv)
{
  for (int i = 1; i < argc; ++i)
  {
    ULOG_INFO("%s",argv[i]);
  }
  return 0;
}

int cmd_timer(int argc, const char **argv)
{
  if (argc == 2)
  {
    if (strcmp(argv[1],"enable") == 0)
    {
      timer_enabled = 1;
    }
    else if (strcmp(argv[1],"disable") == 0)
    {
      timer_enabled = 0;
    }
  }
  
  ULOG_INFO("Timer: %s",timer_enabled ? "enabled" : "disabled");
  
  return 0;
}

int cmd_clear(int argc, const char **argv)
{
  shell_print("\x1b[1J\x1b[H");

  return 0;
}

int cmd_log(int argc, const char **argv)
{
  ULOG_INFO("Info");
  ULOG_WARNING("Warning");
  ULOG_ERROR("Error");
  ULOG_CRITICAL("Critical");
  return 0;
}

int main(int argc, char const *argv[])
{
  int timer = 0;

  if (argc != 2)
  {
    return 1;
  }

  serial::Timeout timeout = serial::Timeout::simpleTimeout(1);
  port.setTimeout(timeout);
  port.setPort(argv[1]);
  port.open();


  Command_t commands[] = {
    {.name = "echo", .on_execute = cmd_echo ,.description = "echo argv[1]"},
    {.name = "timer",.on_execute = cmd_timer,.description = "control the timer"},
    {.name = "clear",.on_execute = cmd_clear,.description = "clear screen"},
    {.name = "logdump",.on_execute = cmd_log,.description = "logdump"}
  };

  shell_init(commands,sizeof(commands) / sizeof(commands[0]),shell_flush);

  shell_set_prompt("\x1b[34;1mexample \x1b[36m> \x1b[0m");

  char input_buff[100];
  size_t read_count = 0;

  while (true)
  {
    read_count = port.read(reinterpret_cast<uint8_t *>(input_buff),sizeof(input_buff));
    for (size_t i = 0; i < read_count; ++i)
    {
      printf("0x%02X '%c'\n",input_buff[i], (input_buff[i] >= 32 && input_buff[i] <= 127) ? input_buff[i] : ' ');
    }
    shell_update(input_buff,read_count);
    if (timer_enabled)
    {
      timer++;
      if (timer >= 1000)
      {
        ULOG_INFO("Timer Test");
        timer = 0;
      }
    }
  }
  

  return 0;
}
