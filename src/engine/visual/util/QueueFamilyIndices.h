//
// Created by ???????? on 09.08.2025.
//

#ifndef BIBAPROJ_QUEUEFAMILYINDICES_H
#define BIBAPROJ_QUEUEFAMILYINDICES_H

#include <cstdint>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() &&
               presentFamily.has_value(); // todo mb required minimum
    }
};

#endif //BIBAPROJ_QUEUEFAMILYINDICES_H
