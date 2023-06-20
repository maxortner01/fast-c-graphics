#pragma once

#include "../Utility/NoCopy.hpp"
#include "../Binding.hpp"

#include <cstddef>
#include <vector>
#include <string>

namespace fcg
{
    struct GraphicsDevice : C::FCG_GraphicsDevice
    {
        std::string getName() const;
    };

    class Machine : C::FCG_Machine,
        public Utility::NoCopy
    {
    public:
        Machine();
        ~Machine();

        void getGraphicsDevices(std::vector<GraphicsDevice>& devices) const;
    };
}