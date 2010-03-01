/*
*      confparse.h
*      Copyright © 2008, 2009, 2010 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONFPARSE_H
#define CONFPARSE_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../hftirc.h"

/* Section delimiter */
#define SEC_DEL_S '['
#define SEC_DEL_E ']'

/* List delimiter */
#define LIST_DEL_S '{'
#define LIST_DEL_E '}'

/* Comment character */
#define COMMENT_CHAR '#'

enum { SecStart, SecEnd, SecLast };
typedef enum { False, True } Bool;

typedef struct
{
     long int num;
     float fnum;
     Bool bl;
     char *str;
} opt_type;

/* util.c */
char *erase_delim_content(char *buf);
Bool is_in_delimiter(char *buf, int p);
char *erase_sec_content(char *buf);
char *opt_srch(char *buf, char *opt);
opt_type str_to_opt(char *str);
char *clean_value(char *str);
void cfg_set_sauv(char *str);
char **secname(char *name);
void free_secname(char **secname);

/* confparse.c */
char *file_to_str(char *path);
char *get_sec(char *src, char *name);
char *get_nsec(char *src, char *name, int n);
int get_size_sec(char *src, char *name);
opt_type get_opt(char *src, char *def, char *name);
opt_type *get_list_opt(char *src, char *def, char *name, int *n);

static const opt_type null_opt_type = {0, 0, 0, NULL};

char *sauv_delimc;
char *sauv_secc;

#endif /* CONFPARSE_H */
