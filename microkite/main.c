#include "platform.h"   /* platform-dependent functions */
#include "../generic/fanf.h"


int main(void) {
    if(platform_init) platform_init();
    FANF(PLATFORM,(NVM_SIZE/1024));
    return 0;
}
