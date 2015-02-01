/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        evas_object.cpp
 * @author      Lukasz Wrzosek (l.wrzosel@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation for Evas_Object wrapper from
 * Efl.
 */

#include "evas_object.h"
#include <dpl/foreach.h>

Evas_Object* EvasObject::IConnection::GetEvasObject()
{
    return m_object->GetObject();
}

void EvasObject::IConnection::Disconnect()
{
    m_object->DisconnectCallback(this);
}

EvasObject::IConnection::IConnection(EvasObject::EvasObjectShared* object) :
    m_object(object)
{}

void EvasObject::IConnection::SmartCallbackWrapper(void* data,
                                                   Evas_Object* /*object*/,
                                                   void* event_info)
{
    Assert(data);
    IConnection* Calle = static_cast<IConnection*>(data);
    Calle->Call(event_info);
}

void EvasObject::IConnection::EvasCallbackWrapper(void* data,
                                                  Evas* /*evas*/,
                                                  Evas_Object* /*object*/,
                                                  void* event_info)
{
    Assert(data);
    IConnection* Calle = static_cast<IConnection*>(data);
    Calle->Call(event_info);
}

Evas_Object* EvasObject::EvasObjectShared::GetObject()
{
    return m_object;
}

EvasObject::EvasObjectShared::SmartConnectionBase::SmartConnectionBase(
    const std::string& name,
    EvasObject::EvasObjectShared* object) :
    IConnection(object),
    m_callbackName(name)
{}

void EvasObject::EvasObjectShared::SmartConnectionBase::ConnectPrv()
{
    evas_object_smart_callback_add(GetEvasObject(),
                                   m_callbackName.c_str(),
                                   &IConnection::SmartCallbackWrapper, this);
}

void EvasObject::EvasObjectShared::SmartConnectionBase::DisconnectPrv()
{
    evas_object_smart_callback_del(GetEvasObject(),
                                   m_callbackName.c_str(),
                                   &IConnection::SmartCallbackWrapper);
}

EvasObject::EvasObjectShared::EvasConnectionBase::EvasConnectionBase(
    Evas_Callback_Type type,
    EvasObject::EvasObjectShared* object) :
    IConnection(object),
    m_callbackType(type)
{}

void EvasObject::EvasObjectShared::EvasConnectionBase::ConnectPrv()
{
    evas_object_event_callback_add(
        GetEvasObject(), m_callbackType, &IConnection::EvasCallbackWrapper,
        this);
}

void EvasObject::EvasObjectShared::EvasConnectionBase::DisconnectPrv()
{
    evas_object_event_callback_del_full(
        GetEvasObject(), m_callbackType, &IConnection::EvasCallbackWrapper,
        this);
}

EvasObject::EvasObjectShared::EvasObjectShared() :
    m_object(NULL)
{}

EvasObject::EvasObjectShared::EvasObjectShared(Evas_Object* object) :
    m_object(object)
{
    Assert(m_object);
    evas_object_event_callback_add(m_object,
                                   EVAS_CALLBACK_DEL,
                                   &StaticOnDelEvent,
                                   this);
}

void EvasObject::EvasObjectShared::SetObject(Evas_Object* object)
{
    Assert(m_object == NULL);
    Assert(object != NULL);
    m_object = object;
    evas_object_event_callback_add(m_object,
                                   EVAS_CALLBACK_DEL,
                                   &StaticOnDelEvent,
                                   this);
}

EvasObject::EvasObjectShared::~EvasObjectShared()
{
    if (m_object) {
        DisconnectAll();
        evas_object_event_callback_del(m_object,
                                       EVAS_CALLBACK_DEL,
                                       &StaticOnDelEvent);
        m_object = NULL;
    }
}

bool EvasObject::EvasObjectShared::DisconnectCallback(IConnection* connection)
{
    IConnectionsSet::iterator it = m_connections.find(connection);
    if (it != m_connections.end()) {
        (*it)->DisconnectPrv();
        delete connection;
        m_connections.erase(it);
        return true;
    }
    return false;
}

void EvasObject::EvasObjectShared::DisconnectAll()
{
    FOREACH(it, m_connections)
    {
        (*it)->DisconnectPrv();
        delete *it;
    }
    m_connections.clear();
}

void EvasObject::EvasObjectShared::StaticOnDelEvent(void* data,
                                                    Evas* /*e*/,
                                                    Evas_Object* /*o*/,
                                                    void* /*ev*/)
{
    Assert(data);
    EvasObjectShared* This = static_cast<EvasObjectShared*>(data);
    if (This->m_object) {
        evas_object_event_callback_del(This->m_object,
                                       EVAS_CALLBACK_DEL,
                                       &StaticOnDelEvent);
        This->DisconnectAll();
        This->m_object = NULL;
    }
}

EvasObject::EvasObject() :
    m_object(new EvasObjectShared())
{}

EvasObject::EvasObject(Evas_Object* object) :
    m_object(new EvasObjectShared(object))
{}

EvasObject::EvasObject(const EvasObject& other) :
    m_object(other.m_object)
{}

//this destructor must be here to let pimpl with shared_ptr work without warning
EvasObject::~EvasObject()
{}

EvasObject& EvasObject::operator=(const EvasObject& other)
{
    Assert(m_object);
    m_object = other.m_object;
    return *this;
}

EvasObject* EvasObject::operator=(Evas_Object* object)
{
    Assert(m_object);
    m_object->SetObject(object);
    return this;
}

bool EvasObject::DisconnectCallback(IConnection* connection)
{
    Assert(m_object);
    return m_object->DisconnectCallback(connection);
}

void EvasObject::DisconnectAll()
{
    Assert(m_object);
    m_object->DisconnectAll();
}

EvasObject::operator Evas_Object *()
{
    Assert(m_object);
    return m_object->GetObject();
}
