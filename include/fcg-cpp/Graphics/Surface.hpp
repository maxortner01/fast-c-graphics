#pragma once

#include "../Utility/NoCopy.hpp"
#include "../Binding.hpp"

namespace fcg
{
    enum class SurfaceType
    {
        Window = C::FCG_SURFACE_WINDOW
    };
    
    typedef C::FCG_WindowData WindowData;

namespace Data
{
    template<SurfaceType _SurfaceType>
    struct SurfaceCreateInfo {};

    template<>
    struct SurfaceCreateInfo<SurfaceType::Window>
    { typedef WindowData DataType; };

    template<SurfaceType _SurfaceType>
    using SufaceDataType = typename SurfaceCreateInfo<_SurfaceType>::DataType;
}

    template<SurfaceType _Type>
    class Surface : C::FCG_Surface,
        public Utility::NoCopy
    {
    public:
        inline const static SurfaceType Type = _Type;

        Surface(const Data::SufaceDataType<_Type>& data);
    };

    template<SurfaceType _Type>
    Surface<_Type>::Surface(const Data::SufaceDataType<_Type>& data)
    {
        C::FCG_Surface_Create(this, (C::FCG_ContextType)_Type, (void*)&data);
    }

    typedef Surface<SurfaceType::Window> Window;
}