#include <stdio.h>
#include "tag.h"


int main(int argc, char** argv) {

    printf("tag_get: %d\n", tag_get(1,1,1));
    printf("tag_send: %d\n", tag_send(1,1,NULL, sizeof(void)));
    printf("tag_receive: %d\n", tag_receive(1,1,NULL, sizeof(void)));
    printf("tag_ctl: %d\n", tag_ctl(1,1));

}