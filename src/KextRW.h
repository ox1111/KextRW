#ifndef KEXTRW_H
#define KEXTRW_H

#include <IOKit/IOService.h>

class KextRW: public IOService {
    OSDeclareFinalStructors(KextRW);
public:
    virtual bool start(IOService *provider) override;
};

#endif // KEXTRW_H