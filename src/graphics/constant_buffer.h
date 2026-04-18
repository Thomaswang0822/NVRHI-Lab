#pragma once

#include <nvrhi/nvrhi.h>
#include "../utils/logging.h"

namespace nvrhi_lab {

template<typename T>
class ConstantBuffer {
public:
    ConstantBuffer() = default;
    ~ConstantBuffer() = default;

    ConstantBuffer(const ConstantBuffer&) = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;

    ConstantBuffer(ConstantBuffer&& other) noexcept
        : m_Buffer(std::move(other.m_Buffer))
        , m_Device(other.m_Device)
    {
        other.m_Device = nullptr;
    }

    ConstantBuffer& operator=(ConstantBuffer&& other) noexcept {
        if (this != &other) {
            m_Buffer = std::move(other.m_Buffer);
            m_Device = other.m_Device;
            other.m_Device = nullptr;
        }
        return *this;
    }

    bool Create(nvrhi::IDevice* device, const char* debugName = "ConstantBuffer") {
        if (!device) {
            LogError("ConstantBuffer: Device is null");
            return false;
        }

        m_Device = device;

        m_Buffer = device->createBuffer(
            nvrhi::BufferDesc()
                .setByteSize(sizeof(T))
                .setIsConstantBuffer(true)
                .enableAutomaticStateTracking(nvrhi::ResourceStates::ConstantBuffer)
                .setDebugName(debugName));

        if (!m_Buffer) {
            LogError("ConstantBuffer: Failed to create buffer");
            return false;
        }

        return true;
    }

    void Update(nvrhi::ICommandList* commandList, const T& data) {
        if (m_Buffer && commandList) {
            commandList->writeBuffer(m_Buffer, &data, sizeof(T));
        }
    }

    nvrhi::BufferHandle GetBuffer() const { return m_Buffer; }

    bool IsValid() const { return m_Buffer != nullptr; }

    operator bool() const { return IsValid(); }

private:
    nvrhi::BufferHandle m_Buffer;
    nvrhi::IDevice* m_Device = nullptr;
};

}
