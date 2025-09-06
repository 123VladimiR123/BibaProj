//
// Created by ???????? on 08.08.2025.
//

#ifndef BIBAPROJ_INSTANCEWRAP_H
#define BIBAPROJ_INSTANCEWRAP_H


class InstanceWrap {
    VkInstance instance{};

public:
    InstanceWrap(const InstanceWrap &) = delete;

    InstanceWrap &operator=(const InstanceWrap &) = delete;

    InstanceWrap();

    VkInstance *get() { return &instance; }
};


#endif //BIBAPROJ_INSTANCEWRAP_H
