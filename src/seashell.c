/**
MIT License

Copyright (c) 2026 N. Janssen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "seashell.h"

#include <stdlib.h>
#include <string.h>

#include <ulog.h>


static char g_prefix[SH_PREFIX_SIZE];

static char g_input_buff[SH_INPUT_BUFF_SIZE];
static size_t g_input_buff_size = 0;

static char g_escape_buff[3];

static char g_output_buff[SH_OUTPUT_BUFF_SIZE];
static size_t g_output_buff_size = 0;

static Command_t *g_commands = NULL;
static size_t g_commands_size = 0;


int cmd_list(int argc, const char **argv);
int cmd_clear(int argc, const char **argv);
int cmd_echo(int argc, const char **argv);

static Command_t g_std_commands[] = {
  {.name = "list", .on_execute = cmd_list, .description = "list all commands"},
  {.name = "clear", .on_execute = cmd_clear, .description = "clear the screen"},
  {.name = "echo", .on_execute = cmd_echo, .description = "simple echo"},
};
#define STD_COMMANDS_COUNT (sizeof(g_std_commands) / sizeof(g_std_commands[0]))

static flush_cb g_on_flush = NULL;

static int prompt_visible = 1;
static int escape_sequence = 0;


void _shell_handle_char(char c);
void _shell_handle_input_buff();
void _shell_clear_input_buff();
void _shell_call_command(int argc, const char **argv);
void _shell_output(char c);
void _shell_output_str(const char *c);
void _shell_output_buff(const char *c, size_t len);
void _shell_ulog_wrapper(ulog_level_t level, char *payload);
void _shell_flush_output();

void shell_init(Command_t *commands, size_t commands_size ,flush_cb on_flush)
{
  ULOG_INIT();
  ULOG_SUBSCRIBE(_shell_ulog_wrapper,ULOG_INFO_LEVEL);

  g_commands = commands;
  g_commands_size = commands_size;

  _shell_clear_input_buff();

  g_on_flush = on_flush;
  shell_set_prompt("\x1b[34;1mseashell: \x1b[0m");
}

void shell_set_prompt(const char *str)
{
  if (strlen(str) < SH_PREFIX_SIZE)
  {
    memcpy(g_prefix,str,strlen(str));
  }
}
void shell_update(const char *rx_buff,size_t rx_cnt)
{
  for (size_t i = 0; i < rx_cnt; ++i)
  {
    _shell_handle_char(rx_buff[i]);
  }
  _shell_flush_output();
}

void shell_print(const char *str)
{
  // Handle if the prompt is up
  _shell_output_str(str);
}

void _shell_handle_char(char c)
{  
  if (c == 0x1b || escape_sequence)
  {
    g_escape_buff[escape_sequence++] = c;
    if (escape_sequence == 3)
    {
      ULOG_DEBUG("Escape: %c",g_escape_buff[2]);
      escape_sequence = 0;
    }
  }
  // handle "standerd" input
  else if (c >= 32 && c <= 126)
  {
    // Add it to the the input buff, and echo it
    if (g_input_buff_size < (SH_INPUT_BUFF_SIZE - 1))
    {
      g_input_buff[g_input_buff_size++] = c;
      _shell_output(c);
    }
  }
  // Backspace
  else if (c == 0x08 || c == 0x7F)
  {
    if (g_input_buff_size > 0)
    {
      g_input_buff_size--;
      _shell_output_str("\x1b[1D \x1b[1D");
    }
  }
  // Handle enter
  else if (c == 0x0D)
  {
    // Echo the enter char
    _shell_output_str("\n\r");
    prompt_visible = 0;
    // handle command
    _shell_handle_input_buff();
    // Reprint the prompt
    prompt_visible = 1;
    _shell_output_str(g_prefix);
  }
}

void _shell_handle_input_buff()
{
  // Transform all white spaces to null terminators

  char *argv[10];
  int argc = 0;
  int read_next_arg = 1;
  int in_quotes = 0;

  for (size_t i = 0; i < g_input_buff_size; ++i)
  {
    char c = g_input_buff[i];

    // handle escape characters
    if (c == '\x1b')
    {

    }
    else if (c == '"')
    {
      if (in_quotes)
      {
        g_input_buff[i] = '\0';
      }
      in_quotes = !in_quotes;
    }
    else if (in_quotes)
    {
      if (read_next_arg)
      {
        read_next_arg = 0;
        argv[argc++] = &g_input_buff[i];
      }
      // Do nothing...
    }
    else if (c == ';')
    {
      g_input_buff[i] = '\0';
      _shell_call_command(argc,(const char **)argv);
      argc = 0;
      read_next_arg = 1;
    }
    else if (c == ' ')
    {
      read_next_arg = 1;
      g_input_buff[i] = '\0';
    }
    else if(read_next_arg)
    {
      read_next_arg = 0;
      argv[argc++] = &g_input_buff[i];
    }
  }

  // Force last byte to be '\0'
  g_input_buff[g_input_buff_size] = '\0';

  _shell_call_command(argc,(const char **)argv);
  _shell_clear_input_buff();
}

void _shell_call_command(int argc, const char **argv)
{
  if (argc >= 1)
  {
    int cmd_found = 0;
    for (size_t i = 0; i < g_commands_size; ++i)
    {
      if (strcmp(g_commands[i].name,argv[0]) == 0)
      {
        g_commands[i].on_execute(argc,argv);
        cmd_found = 1;
      }
    }

    for (size_t i = 0; i < STD_COMMANDS_COUNT; ++i)
    {
      if (strcmp(g_std_commands[i].name,argv[0]) == 0)
      {
        g_std_commands[i].on_execute(argc,argv);
        cmd_found = 1;
      }
    }


    if (cmd_found == 0)
    {
      ULOG_WARNING("command \"%s\" not available",argv[0]);
    }
  }
}

void _shell_clear_input_buff()
{
  memset(g_input_buff,'\0',SH_INPUT_BUFF_SIZE);
  g_input_buff_size = 0;
}

void _shell_output(char c)
{
  if (g_output_buff_size >= SH_OUTPUT_BUFF_SIZE)
  {
    _shell_flush_output();
  }
  g_output_buff[g_output_buff_size++] = c;
}

void _shell_output_str(const char *c)
{
  _shell_output_buff(c,strlen(c));
}

void _shell_output_buff(const char *str, size_t len)
{
  for (size_t i = 0; i < len; ++i)
  {
    _shell_output(str[i]);
  }
}

void _shell_ulog_wrapper(ulog_level_t level, char *payload)
{
  if (prompt_visible)
  {
    // Clear the current line
    shell_print("\x1b[0G\x1b[0K");
  }

  if (level == ULOG_WARNING_LEVEL)
  {
    shell_print("\x1b[33m");
  }
  else if (level == ULOG_ERROR_LEVEL)
  {
    shell_print("\x1b[31m");
  }
  else if (level == ULOG_CRITICAL_LEVEL)
  {
    shell_print("\x1b[41m");
  }
  shell_print(payload);
  shell_print("\x1b[0m\n\r");
  if (prompt_visible)
  {
    _shell_output_str(g_prefix);
    _shell_output_buff(g_input_buff,g_input_buff_size);
  }
}

void _shell_flush_output()
{
  if (g_output_buff_size)
  {
    g_on_flush(g_output_buff,g_output_buff_size);
    g_output_buff_size = 0;
  }
}

int cmd_list(int argc, const char **argv)
{
  for (size_t i = 0; i < STD_COMMANDS_COUNT; ++i)
  {
    ULOG_INFO("%s\x1b[20G %s",g_std_commands[i].name,g_std_commands[i].description);
  }

  for (size_t i = 0; i < g_commands_size; ++i)
  {
    ULOG_INFO("%s\x1b[20G %s",g_commands[i].name,g_commands[i].description);
  }
  return 0;
}

int cmd_clear(int argc, const char **argv)
{
  shell_print("\x1b[2J\x1b[H");
  return 0;
}

int cmd_echo(int argc, const char **argv)
{
  for (int i = 1; i < argc; ++i)
  {
    ULOG_INFO("%s",argv[i]);
  }
  return 0;
}