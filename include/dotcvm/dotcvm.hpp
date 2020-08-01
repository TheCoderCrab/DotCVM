#ifndef DC_DOTCVM_H
#define DC_DOTCVM_H

typedef void* device_ptr;

#define DC_CONNECTION_INAGREEMENT       0x10
#define DC_CONNECTION_INEXISTANT        0x20

extern device_ptr get_device(uint requesting_device_id, uint requested_device_id);

#endif /* DC_DOTCVM_H */