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


#ifndef SEASHELL_H_
#define SEASHELL_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <ulog.h>
#include "seashell_config.h"

typedef int(*cmd_cb)(int,const char **);
typedef size_t(*flush_cb)(const char *,size_t);

typedef struct {
  const char *name;
  cmd_cb on_execute;
  const char *description;
} Command_t;


void shell_init(Command_t *commands, size_t commands_size,flush_cb on_flush);
void shell_set_prompt(const char *str);
void shell_update(const char *rx_buff,size_t rx_cnt);

void shell_print(const char *str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SEASHELL_H_