//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 2016 by Gareth Nelson (gareth@garethnelson.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// The Lambda engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//
// $Log:$
//
// DESCRIPTION:
//     A simple unix shell written in C
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <readline/readline.h>
#include <readline/history.h>

static char* unix_prompt = "%s@%s:%s$ ";
static char prompt_str[4096];
static char hostname_str[1024];
static int running=1;
static wordexp_t we;

void spawn_cmd() {
     int pid=fork();
     if(pid==0) { // child
        execvp(we.we_wordv[0],we.we_wordv);
     } else {     // parent
        int stat;
        waitpid(pid,&stat,0);
     }
}

int main() {
    char* input_line;
    char* cwd;
    
    using_history();
    
    wordexp("",&we,0); // first call, so wordexp() allocates memory
    
    while(running) {
       // generate prompt
       cwd = getwd(NULL);
       gethostname(hostname_str,1024);
       snprintf(prompt_str, 4096, unix_prompt,getlogin(),hostname_str,cwd);
       free(cwd);
       
       // read a line from input
       input_line = readline(prompt_str);
       add_history(input_line);
       
       // expand that line
       int retval=wordexp(input_line, &we, WRDE_REUSE|WRDE_SHOWERR|WRDE_UNDEF);
       free(input_line);
       
       switch(retval) {
          case WRDE_BADCHAR:
            // TODO - strtok the line and parse different pipe stages seperately, avoid complaints over | chars
            // TODO - respect &
            // TODO - stdio redirection
            fprintf(stderr,"Bad character in input\n");
          break;
          
          case WRDE_BADVAL:
            fprintf(stderr,"Undefined variable\n");
          break;
          
          case WRDE_NOSPACE:
            fprintf(stderr,"Could not allocate memory!\n");
          break;
          
          case WRDE_SYNTAX:
            fprintf(stderr,"Syntax error\n");
          break;
       }
       
       if(we.we_wordc >= 1) {
          if(strncmp(we.we_wordv[0],"cd",2)==0) {
             chdir(we.we_wordv[1]);
          } else if(strncmp(we.we_wordv[0],"quit",4)==0) {
             exit(0);
          } else if(strncmp(we.we_wordv[0],"exit",4)==0) {
             exit(0);
          } else {
             spawn_cmd();
          }

       }
    }

}
