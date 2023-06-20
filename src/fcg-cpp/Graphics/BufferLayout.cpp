#include <fcg-cpp/Graphics/BufferLayout.hpp>
#include <fcg-cpp/Graphics/Machine.hpp>

namespace fcg
{
    BufferLayout::BufferLayout(const Machine& machine) :
        _machine(machine)
    {
        C::FCG_Data_InitLayout(this);
    }

    BufferLayout::~BufferLayout()
    {
        C::FCG_Data_DestroyLayout(this);
    }

    Buffer& BufferLayout::createBuffer()
    {
        return *(Buffer*)C::FCG_Data_CreateBuffer((const C::FCG_Machine*)&_machine, this);
    }
}