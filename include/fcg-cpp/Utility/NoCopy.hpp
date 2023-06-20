#pragma once

namespace fcg::Utility
{
    struct NoCopy 
    {
        NoCopy() = default;
        NoCopy(const NoCopy&) = delete;
        NoCopy(NoCopy&) = delete;
    };
}