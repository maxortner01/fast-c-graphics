#pragma once

#include "../Utility/NoCopy.hpp"
#include "../Binding.hpp"

#include "Surface.hpp"
#include "Machine.hpp"

namespace fcg
{
    template<SurfaceType _Type, typename _Data>
    class GDI : C::FCG_GDI,
        public Utility::NoCopy
    {
    public:
        GDI(Surface<_Type, _Data>& surface, Machine& machine);
        ~GDI();
    };

    template<SurfaceType _Type, typename _Data>
    GDI<_Type, _Data>::GDI(Surface<_Type, _Data>& surface, Machine& machine)
    {
        C::FCG_CreateGraphicsInstance(
            this, 
            (C::FCG_Surface*)&surface, 
            (C::FCG_Machine*)&machine
        );
    }

    template<SurfaceType _Type, typename _Data>
    GDI<_Type, _Data>::~GDI()
    {
        C::FCG_DestroyGraphicsInstance(this);
    }
}