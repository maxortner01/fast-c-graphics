#pragma once

#include "../Utility/NoCopy.hpp"
#include "../Binding.hpp"

#include <vector>

namespace fcg
{
    typedef C::FCG_Data_Attribute Attribute;
    class Buffer;

    template<typename T>
    struct MemList : public Utility::NoCopy
    {
        MemList(T*& ptr, U32& count) :
            _count(&count),
            _ptr(&ptr)
        {   }
        
        void push(const T& attr)
        {
            *_count += 1;
            *_ptr = (T*)realloc(*_ptr, sizeof(T) * *_count);
            assert(*_ptr);

            memcpy(*_ptr + *_count - 1, &attr, sizeof(T)); 
        }

        void remove(const U32& index)
        {
            assert(index < *_count);
            memmove(*_ptr + index, *_ptr + index + 1, sizeof(T) * (*_count - index - 1));

            *_count -= 1;
            *_ptr = (T*)realloc(*_ptr, sizeof(T) * *_count);
            assert(*_ptr);
        }

        T& operator[](const U32& index)
        {
            assert(index < *_count);
            return (*_ptr)[index];
        }

        auto count() const { return *_count; }

    private:
        U32* _count;
        T**  _ptr;
    };
    
    typedef MemList<Attribute> AttributeList;

    class Buffer : C::FCG_Data_Buffer
    {
    public:
        AttributeList getAttributes();
        void setData(void* data, const U32& byteSize);
    };
}