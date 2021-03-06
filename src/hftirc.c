/*
 * Copyright (c) 2010 Martin Duquesnoy <xorg62@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "hftirc.h"

void
signal_handler(int signal)
{
     int b[2], u[2];
     (void)u;

     switch(signal)
     {
          /* Term resize sig */
          case SIGWINCH:

               b[0] = LINES;
               b[1] = COLS;
               endwin();
               refresh();

               getmaxyx(stdscr, u[0], u[1]);
               ui_init();
               ui_get_input();
               ui_print_buf(hftirc.statuscb, "[HFTIrc] *** Terminal resized: (%dx%d -> %dx%d)",
                         b[0], b[1], LINES, COLS);
               ui_buf_set(hftirc.selcb->id);
	
              break;
     }

     return;
}

int
main(int argc, char **argv)
{
    struct sigaction sig;
    int i, n, maxfd = 0;
    fd_set iset;
    static struct timeval tv;
    IrcSession *is;
    ChanBuf *cb;

    snprintf(hftirc.conf.path, FILENAME_MAX, "%s/"DEF_CONF, getenv("HOME"));

    while((i = getopt(argc, argv, "hvc:")) != -1)
    {
         switch(i)
         {
              case 'h':
              default:
                   printf("usage: %s [-hv] [-c <file>]\n"
                          "   -h         Show this page\n"
                          "   -v         Show version\n"
                          "   -c <file>  Load a configuration file\n", argv[0]);
                   exit(EXIT_SUCCESS);
                   break;

              case 'v':
                   printf("HFTIrc version: "HFTIRC_VERSION"\n");
                   exit(EXIT_SUCCESS);
                   break;

              case 'c':
                   strcpy(hftirc.conf.path, optarg);
                   break;
         }
    }

    /* Primary allocation / set */
    hftirc.ft = 1;

    /* Signal initialisation */
    sig.sa_handler = signal_handler;
    sig.sa_flags   = 0;
    sigaction(SIGWINCH, &sig, NULL);

    hftirc.running = 1;

    config_parse();
    ui_init();
    update_date();
    irc_init();
    ui_refresh_curpos();

    while(hftirc.running)
    {
         if(hftirc.running < 0)
              ++hftirc.running;

         tv.tv_sec = 0;
         tv.tv_usec = 250000;

         FD_ZERO(&iset);

         FD_SET(STDIN_FILENO, &iset);

         maxfd = STDIN_FILENO;

         for(n = 0, is = hftirc.sessionhead; is; is = is->next, ++n)
              if(is->sock > 0 && is->connected)
              {
                   if(maxfd < is->sock)
                        maxfd = is->sock;

                   FD_SET(is->sock, &iset);
              }

         if(select(maxfd + n + 1, &iset, NULL, NULL, &tv) > 0)
         {
              if(FD_ISSET(STDIN_FILENO, &iset))
                   ui_get_input();
              else
                  for(is = hftirc.sessionhead; is; is = is->next)
                       if(irc_run_process(is, &iset))
                            is->connected = 0;
         }

         /* Updating date */
         update_date();

         /* Update status win with date/chan act/user info */
         ui_update_statuswin();

         /* topic win and nicklist updated only if needed with umask */
         ui_update_topicwin();
         ui_update_nicklistwin();
    }

    endwin();

    free(hftirc.conf.serv);

    for(is = hftirc.sessionhead; is; is = is->next)
         free(is);

    for(cb = hftirc.cbhead; cb; cb = cb->next)
         ui_buf_close(cb);

    return 0;
}
