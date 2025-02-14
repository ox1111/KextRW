#include "KextRW.h"

#include <IOKit/IOService.h>

#define super IOService
OSDefineMetaClassAndFinalStructors(KextRW, IOService);

bool KextRW::start(IOService *provider) {
    if (!super::start(provider)) {
        return false;
    }

    registerService();

    return true;
}