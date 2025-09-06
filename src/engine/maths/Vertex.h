//
// Created by admin on 8/19/2025.
//

#ifndef VERTEX_H
#define VERTEX_H
#include "Tangent4.h"
#include "Vector2.h"
#include "Vector3.h"

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Tangent4 tangent;
    Vector3 bitangent;
    Vector2 uv;
};

#endif //VERTEX_H
