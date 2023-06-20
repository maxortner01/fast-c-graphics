#pragma once

#include "Buffer.hpp"

namespace fcg
{
    class Machine;

    class BufferLayout : C::FCG_Data_BufferLayout,
        public Utility::NoCopy
    {
        const Machine& _machine;

    public:
        BufferLayout(const Machine& machine);
        ~BufferLayout();

        Buffer& createBuffer();
    };
}