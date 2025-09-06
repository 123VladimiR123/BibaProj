//
// Created by admin on 8/31/2025.
//

#ifndef TEST_H
#define TEST_H
#include "engine/util/Logger.h"

inline void printMat4(Mat4& mat)
{
    Logger::get() << "MATRIX: [ ";
    for (auto& i : mat.data)
    {
        Logger::get() << i << " ";
    }
    Logger::get() << "]" << std::endl;
}

inline void findDifference(Mat4& first, Mat4& second, std::string err)
{
    for (uint8_t i{}; i < 16; i++)
    {
        if (abs(first(i / 4, i % 4) - second(i / 4, i % 4)) > 10e-5)
        {
            printMat4(first);
            printMat4(second);
            throw std::runtime_error("Mat4 test error: " + err);
        }
    }
}

inline void testMatConvert()
{
    Mat4 m{};
    Transform t{};
    t.emplaceFromMat4();
    Mat4 m2 = t.getMatrix();
    findDifference(m, m2, "Convert from mat to quat failed");
}

inline void testMatChanges()
{
    Mat4 m{};

    Transform t;
    t.emplaceFromMat4();

    t.move(std::array{5.0f, 5.0f, 5.0f}.data());
    t.move(std::array{-5.0f, -5.0f, -5.0f}.data());

    t.getQuat()->normFull();

    findDifference(m, t.getMatrix(), "Simple movement failed");
}

inline void testMatChanges2()
{
    alignas(16) float data1[16] = {
        -0.4234085381031037,
        -0.9059388637542724,
        -7.575183536001616e-11,
        0.0,
        -0.9059388637542724,
        0.4234085381031037,
        -4.821281221478735e-11,
        0.0,
        7.575183536001616e-11,
        4.821281221478735e-11,
        -1.0,
        0.0,
        -90.59386444091796,
        -24.379817962646489,
        -40.05522918701172,
        1.0
    };

    alignas(16) float data2[16] = {
        -0.4234085381031037,
        -0.9059388637542724,
        -7.575183536001616e-11,
        0.0,
        -0.9059388637542724,
        0.4234085381031037,
        -4.821281221478735e-11,
        0.0,
        7.575183536001616e-11,
        4.821281221478735e-11,
        -1.0,
        0.0,
        -90.59386444091796,
        -24.379817962646489,
        -40.05522918701172,
        1.0
    };
    Mat4 m1{data1};
    Mat4 m2{data2};
    Transform t(std::move(m1));
    t.emplaceFromMat4();

    findDifference(t.getMatrix(), m2, "GLTF matrix convert failed");
}

inline void testRun()
{
    testMatConvert();
    testMatChanges();
    testMatChanges2();

    Logger::get() << "Size of Vertex: " << sizeof(Vertex) << std::endl;
    Logger::get() << "Offset of position: " << offsetof(Vertex, position) << std::endl;
    Logger::get() << "Offset of normal: " << offsetof(Vertex, normal) << std::endl;
    Logger::get() << "Offset of tangent: " << offsetof(Vertex, tangent) << std::endl;
    Logger::get() << "Offset of bitangent: " << offsetof(Vertex, bitangent) << std::endl;
    Logger::get() << "Offset of uv: " << offsetof(Vertex, uv) << std::endl;

}

#endif //TEST_H
