#include <fcg-cpp/Graphics/Buffer.hpp>

#include <iostream>

namespace fcg
{
    AttributeList Buffer::getAttributes()
    {
        return AttributeList(this->attributes, this->attribute_count);
    }

    void Buffer::setData(void* data, const U32& byteSize)
    {
        C::FCG_Data_LoadData(this, data, byteSize, this->attribute_count, this->attributes);
    }
}