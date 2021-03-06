#include <cstring>

#include "interface_object.h"

InterfaceObject::~InterfaceObject()
{
    if (_properties != nullptr)
        delete[] _properties;
}

void InterfaceObject::readPropertyDescription(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access)
{
    PropertyDescription* descriptions = propertyDescriptions();
    uint8_t count = propertyDescriptionCount();

    numberOfElements = 0;
    if (descriptions == nullptr || count == 0)
        return readPropertyDescription2(propertyId, propertyIndex, writeEnable, type, numberOfElements, access);

    PropertyDescription* desc = nullptr;

    // from KNX spec. 03.03.07 Application Layer (page 56) - 3.4.3.3  A_PropertyDescription_Read-service
    // Summary: either propertyId OR propertyIndex, but not both at the same time
    if (propertyId != 0)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            PropertyDescription d = descriptions[i];
            if (d.Id != propertyId)
                continue;

            desc = &d;
            propertyIndex = i;
            break;
        }
    }
    else
    {
        // If propertyId is zero, propertyIndex shall be used.
        // Response: propertyIndex of received A_PropertyDescription_Read
        if (propertyIndex < count)
        {
            desc = &descriptions[propertyIndex];
        }
    }

    if (desc != nullptr)
    {
        propertyId = desc->Id;
        writeEnable = desc->WriteEnable;
        type = desc->Type;
        numberOfElements = desc->MaxElements;
        access = desc->Access;
    }
    else
        return readPropertyDescription2(propertyId, propertyIndex, writeEnable, type, numberOfElements, access);
}

void InterfaceObject::readPropertyDescription2(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access)
{
    uint8_t count = _propertyCount;

    numberOfElements = 0;
    if (_properties == nullptr || count == 0)
        return;

    Property* prop = nullptr;

    // from KNX spec. 03.03.07 Application Layer (page 56) - 3.4.3.3  A_PropertyDescription_Read-service
    // Summary: either propertyId OR propertyIndex, but not both at the same time
    if (propertyId != 0)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            Property* p = _properties[i];
            if (p->Id() != propertyId)
                continue;

            prop = p;
            propertyIndex = i;
            break;
        }
    }
    else
    {
        // If propertyId is zero, propertyIndex shall be used.
        // Response: propertyIndex of received A_PropertyDescription_Read
        if (propertyIndex < count)
        {
            prop = _properties[propertyIndex];
        }
    }

    if (prop != nullptr)
    {
        propertyId = prop->Id();
        writeEnable = prop->WriteEnable();
        type = prop->Type();
        numberOfElements = prop->MaxElements();
        access = prop->Access();
    }
}

void InterfaceObject::readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data)
{
    Property* prop = property(id);
    if (prop == nullptr)
    {
        count = 0;
        return;
    }

    count = prop->read(start, count, data);
}

void InterfaceObject::writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count)
{
    Property* prop = property(id);
    if (prop == nullptr)
    {
        count = 0;
        return;
    }

    count = prop->write(start, count, data);
}

uint8_t InterfaceObject::propertySize(PropertyID id)
{
    Property* prop = property(id);
    if (prop == nullptr)
    {
        return 0;
    }

    return prop->ElementSize();
}

uint8_t InterfaceObject::propertyDescriptionCount()
{
    return 0;
}

PropertyDescription* InterfaceObject::propertyDescriptions()
{
    return nullptr;
}


void InterfaceObject::initializeProperties(size_t propertiesSize, Property** properties)
{
    _propertyCount = propertiesSize / sizeof(Property*);
    _properties = new Property*[_propertyCount];
    memcpy(_properties, properties, propertiesSize);
}


Property* InterfaceObject::property(PropertyID id)
{
    for (int i = 0; i < _propertyCount; i++)
        if (_properties[i]->Id() == id)
            return _properties[i];

    return nullptr;
}


uint8_t* InterfaceObject::save(uint8_t* buffer)
{
    for (int i = 0; i < _propertyCount; i++)
    {
        Property* prop = _properties[i];
        if (!prop->WriteEnable())
            continue;
        
        buffer = prop->save(buffer);
    }
    return buffer;
}


const uint8_t* InterfaceObject::restore(const uint8_t* buffer)
{
    for (int i = 0; i < _propertyCount; i++)
    {
        Property* prop = _properties[i];
        if (!prop->WriteEnable())
            continue;

        buffer = prop->restore(buffer);
    }
    return buffer;
}


uint16_t InterfaceObject::saveSize()
{
    uint16_t size = 0;

    for (int i = 0; i < _propertyCount; i++)
    {
        Property* prop = _properties[i];
        if (!prop->WriteEnable())
            continue;

        size += prop->saveSize();
    }
    return size;
}


const Property* InterfaceObject::property(PropertyID id) const
{
    for (int i = 0; i < _propertyCount; i++)
        if (_properties[i]->Id() == id)
            return _properties[i];

    return nullptr; 
}
