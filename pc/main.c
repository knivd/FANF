#include "platform.h"
#include "..\generic\fanf.h"


int main(void) {
    if(platform_init) platform_init();
    FANF(PLATFORM,65536);
    return 0;
}
