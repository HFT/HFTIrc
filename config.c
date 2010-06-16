#include "hftirc.h"

#define SSTRCPY(dest, src) if(src) strcpy((dest), (src))

void
config_server(void)
{
     int i, j, n = 0;
     struct conf_sec **serv;
     struct opt_type *opt;
     ServInfo defsi = { "Hft", "irc.hft-community", "", 6667, "hftircuser", " ", "HFTIrcuser", "HFTIrcuser"};

     serv = fetch_section(fetch_section_first(NULL, "servers"), "server");

     if(!(hftirc->conf.nserv = fetch_section_count(serv)))
     {
          hftirc->conf.serv[0] = defsi;

          return;
     }

     if(hftirc->conf.nserv > NSERV)
     {
          ui_print_buf(0, "HFTIrc configuratin: Too much servs (limit: %d)", NSERV);
          hftirc->conf.nserv = NSERV;
     }

     for(i = 0; i < hftirc->conf.nserv; ++i)
     {

          SSTRCPY(hftirc->conf.serv[i].adress,   fetch_opt_first(serv[i], "irc.hft-community.org", "adress").str);
          SSTRCPY(hftirc->conf.serv[i].name,     fetch_opt_first(serv[i], hftirc->conf.serv[i].adress, "name").str);
          SSTRCPY(hftirc->conf.serv[i].password, fetch_opt_first(serv[i], "", "password").str);
          SSTRCPY(hftirc->conf.serv[i].nick,     fetch_opt_first(serv[i], "hftircuser", "nickname").str);
          SSTRCPY(hftirc->conf.serv[i].username, fetch_opt_first(serv[i], "", "username").str);
          SSTRCPY(hftirc->conf.serv[i].realname, fetch_opt_first(serv[i], "", "realname").str);
          hftirc->conf.serv[i].port = fetch_opt_first(serv[i], "6667", "port").num;

          opt = fetch_opt(serv[i], "", "channel_autojoin");

          if((n = fetch_opt_count(opt)))
          {
               if((hftirc->conf.serv[i].nautojoin = n) > 127)
                    ui_print_buf(0, "HFTIrc configuration: section serv (%d), too many channel_autojoin (%d).", i, n);
               else
                    for(j = 0; j < n; ++j)
                         SSTRCPY(hftirc->conf.serv[i].autojoin[j], opt[j].str);
          }

          free(opt);
     }

     free(serv);

     return;
}

void
config_parse(char *file)
{
     struct conf_sec *misc;

     if (get_conf(file) == -1)
     {
          ui_print_buf(0, "parsing configuration file (%s) failed.", file);
          get_conf(CONFPATH);
     }


     /* Misc section */
     misc = fetch_section_first(NULL, "misc");

     hftirc->conf.bell = fetch_opt_first(misc, "false", "bell").boolp;

     free(misc);

     /* Servers section */
     config_server();

     return;
}
