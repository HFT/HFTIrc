/*
*      confparse.c
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

#include "confparse.h"

char*
file_to_str(char *path)
{
     char *buf, *ret, *p, *c;
     int fd, i;
     struct stat st;
     Bool is_char = False;

     if (!path)
          return NULL;

     if (!(fd = open(path, O_RDONLY)))
     {
          ui_print_buf(0, "HFTIrc configuration: %s", path);
          return NULL;
     }

     /* Get the file size */
     stat(path, &st);

     /* Bufferize file */
     if((buf = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, SEEK_SET)) == (char*) MAP_FAILED)
          return NULL;

     /* Copy buffer without comments in return value */
     ret = calloc(strlen(buf) + 1, sizeof(char));

     for(p = buf, i = 0; *p != '\0'; p++)
     {
          if(!is_char && (c = strchr("\"'", *p)))
               is_char = !is_char;
          else if (is_char && *p == *c)
               is_char = !is_char;

          if(*p == COMMENT_CHAR && !is_char)
          {
               if(!(p = strchr(p, '\n')))
                    break;
               ret[i++] = '\n';
          }
          else
               ret[i++] = *p;
     }
     ret[i++] = '\0';

     /* Unmap buffer, thanks linkdd. */
     munmap(buf, st.st_size);
     close(fd);

     ui_print_buf(0, "HFTIrc configuration: %s read.", path);

     return ret;
}

char*
get_sec(char *src, char *name)
{
     char *ret = NULL, *start, *end, *p;
     char **sec;
     size_t len;

     if(!src)
          return NULL;

     if(!name)
          return src;

     sec = secname(name);
     len = strlen(sec[SecStart]);

     /* Find start section pointer */
     for(start = src; *start != '\0'; start++)
     {
          if( (p = strchr("\"'", *start)) )
               while (*(++start) && *start != *p);

          if(!strncmp(start, sec[SecStart], len))
               break;
     }

     if(*start != '\0')
     {
          /* Here is_char == False */
          start += len;
          /* Find end section pointer */
          for(end = start; *end != '\0'; end++)
          {
               if( (p = strchr("\"'", *start)) )
                    while (*(++start) && *start != *p);

               if(!strncmp(end, sec[SecEnd], len+1))
                    break;
          }

          /* Allocate and set ret */
          if(end != '\0')
          {
               len = end - start;
               ret = calloc(len + 1, sizeof(char));
               memcpy(ret, start, len);
               ret[len] = '\0';
          }
     }

     free_secname(sec);

     return ret;
}

char*
get_nsec(char *src, char *name, int n)
{
     int i;
     char *ret, *buf, **sec;

     if(!src || !strlen(src))
          return NULL;

     if(!name)
          return src;

     if(!n)
          return get_sec(src, name);

     sec = secname(name);

     buf = strdup(sauv_delimc);

     for(i = 0; i < n && (buf = strstr(buf, sec[SecStart])); ++i, buf += strlen(sec[SecStart]));

     ret = get_sec(src + strlen(src) - strlen(buf), name);

     free_secname(sec);

     return ret;
}

int
get_size_sec(char *src, char *name)
{
     int ret;
     char **sec, *buf;

     if(!src || !name)
          return 0;

     sec = secname(name);

     buf = strdup(sauv_secc);

     for(ret = 0; (buf = strstr(buf, sec[SecStart])); ++ret, buf += strlen(sec[SecStart]));

     free_secname(sec);

     return ret;
}

opt_type
get_opt(char *src, char *def, char *name)
{
     int i;
     char *p = NULL, *p2 = NULL;
     opt_type ret = null_opt_type;

     if(!src || !name)
          return (def) ? str_to_opt(def) : ret;

     if((p = opt_srch(sauv_secc, name)))
     {
          for(i = 0; p[i] && p[i] != '\n'; ++i);
          p[i] = '\0';

          p2 = strdup(p + strlen(name));

          if((p = strchr(p, '=')) && !is_in_delimiter(p, 0))
          {
               for(i = 0; p2[i] && p2[i] != '='; ++i);
               p2[i] = '\0';

               /* Check if there is anything else that spaces
                * between option name and '=' */
               for(i = 0; i < strlen(p2); ++i)
                    if(p2[i] != ' ')
                    {
                         ui_print_buf(0, "HFTIrc configuration warning: Missing '=' after option: '%s'"
                                   " and before expression: '%s'\n", name, p2);
                         return str_to_opt(def);
                    }

               ret = str_to_opt(clean_value(++p));
          }
     }

     if(!ret.str)
          ret = str_to_opt(def);

     return ret;
}

/* option = {val1, val2, val3} */
opt_type*
get_list_opt(char *src, char *def, char *name, int *n)
{
     int i, j;
     char *p, *p2;
     opt_type *ret;

     if(!src || !name)
          return NULL;

     *n = 0;

     if(!(p = get_opt(src, def, name).str))
          return NULL;

     for(i = 0; p[i] && (p[i] != LIST_DEL_E || is_in_delimiter(p, i)); ++i);
     p[i + 1] = '\0';

     /* Syntax of list {val1, val2, ..., valx} */
     if(*p != LIST_DEL_S || *(p + strlen(p) - 1) != LIST_DEL_E)
          return NULL;

     /* Erase ( ) */
     ++p;
     *(p + strlen(p) - 1) = '\0';

     /* > 1 value in list */
     if(strchr(p, ','))
     {
          /* Count ',' */
          for(i = 0, *n = 1; i < strlen(p); ++i)
               if(p[i] == ',' && !is_in_delimiter(p, i))
                    ++(*n);

          ret = calloc(*n, sizeof(opt_type));

          p2 = strdup(p);

          /* Set all value in return array */
          for(i = j = 0; i < *n; ++i, p2 += ++j)
          {
               for(j = 0; j < strlen(p2) && (p2[j] != ',' || is_in_delimiter(p2, j)); ++j);
               p2[j] = '\0';

               ret[i] = str_to_opt(clean_value(p2));
          }
     }
     else
     {
          ret = calloc((*n = 1), sizeof(opt_type));
          *ret = str_to_opt(clean_value(p));
     }

     return ret;
}
