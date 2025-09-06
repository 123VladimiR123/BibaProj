//
// Created by admin on 8/30/2025.
//

#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include "./../../maths/Mat4.h"

struct UniformBufferObject {
    Mat4 view;
    Mat4 proj;
};

#endif //UNIFORMBUFFEROBJECT_H
