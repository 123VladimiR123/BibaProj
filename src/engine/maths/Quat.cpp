#include "Quat.h"

Quat::Quat(const float input[4])
{
    _mm_store_ps(real, _mm_loadu_ps(input));
}

void Quat::setReal(const float input[4])
{
    _mm_store_ps(real, _mm_loadu_ps(input));
}

const float (& Quat::getReal() const)[4]
{
    return this->real;
}
