#pragma once

#include "../Utility/NoCopy.hpp"
#include "../Binding.hpp"

#include "Surface.hpp"
#include "Machine.hpp"

namespace fcg
{
    template<SurfaceType _Type>
    class GDI : C::FCG_GDI,
        public Utility::NoCopy
    {
    public:
        GDI(Surface<_Type>& surface, Machine& machine);
        ~GDI();
    };

    template<SurfaceType _Type>
    GDI<_Type>::GDI(Surface<_Type>& surface, Machine& machine)
    {
        C::FCG_CreateGraphicsInstance(
            this, 
            (C::FCG_Surface*)&surface, 
            (C::FCG_Machine*)&machine
        );
    }

    template<SurfaceType _Type>
    GDI<_Type>::~GDI()
    {
        C::FCG_DestroyGraphicsInstance(this);
    }
}