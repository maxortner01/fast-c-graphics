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

    template<SurfaceType _Type, typename _Data>
    class Surface : C::FCG_Surface,
        public Utility::NoCopy
    {
    public:
        inline const static SurfaceType Type = _Type;
        typedef _Data Data;

        Surface(const _Data& data);
    };

    template<SurfaceType _Type, typename _Data>
    Surface<_Type, _Data>::Surface(const _Data& data)
    {
        C::FCG_Surface_Create(this, (C::FCG_ContextType)_Type, (void*)&data);
    }

    typedef Surface<SurfaceType::Window, C::FCG_WindowData> Window;
}