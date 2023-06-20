#include <fcg-cpp/Graphics/Machine.hpp>

namespace fcg
{
    std::string GraphicsDevice::getName() const
    {
        return std::string(this->name);
    }

    Machine::Machine()
    {
        C::FCG_InitializeMachine(this);
    }

    Machine::~Machine()
    {
        C::FCG_DestroyMachine(this);
    }

    void Machine::getGraphicsDevices(std::vector<GraphicsDevice>& devices) const
    {
        devices.resize(this->graphics_device_count);
        memcpy(&devices[0], this->graphics_devices, sizeof(C::FCG_GraphicsDevice));
    }
}