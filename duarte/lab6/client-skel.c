#include "storyserver.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
 
    message m;

    
    
  
    
    int i = 0;
    
    
    printf("message: ");
    fgets(m.buffer, MESSAGE_LEN, stdin);

    /* write message */
    
    
    /* receive story */

    printf("OK\n");
    exit(0);
    
}