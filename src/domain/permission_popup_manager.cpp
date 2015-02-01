/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
/**
 * @file    permission_popup_manager.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "permission_popup_manager.h"

#include <dpl/availability.h>
#include <dpl/foreach.h>
#include <dpl/log/secure_log.h>
#include <dpl/singleton_safe_impl.h>
#include <Evas.h>

IMPLEMENT_SAFE_SINGLETON(PermissionPopupManager)

PermissionPopupManager::PermissionPopupManager()
{}

PermissionPopupManager::~PermissionPopupManager()
{}

void PermissionPopupManager::registerPopup(Evas_Object* webview, Evas_Object* popup)
{
    _D("register %p %p", webview, popup);

    if (!webview || !popup)
    {
        _W("Wrong input argument");
        return;
    }
    addWebview(webview);
    addPopup(popup);
    m_pairList.push_back(Pair(webview, popup));
}

void PermissionPopupManager::unregisterWebview(Evas_Object* webview)
{
    _D("unegister webview[%p]", webview);

    FOREACH(it, m_pairList)
    {
        if (it->first == webview)
        {
            removePopup(it->second);
            evas_object_del(it->second);

            // erase iterator
            PairList::iterator next = it;
            ++next;
            m_pairList.erase(it);
            it = next;
        }
    }
    removeWebview(webview);
}

void PermissionPopupManager::unregisterPopup(Evas_Object* popup)
{
    _D("unegister popup[%p]", popup);

    Evas_Object* webview = NULL;

    FOREACH(it, m_pairList)
    {
        if (it->second == popup)
        {
            webview = it->first;
            removePopup(it->second);

            // erase iterator
            PairList::iterator next = it;
            ++next;
            m_pairList.erase(it);
            it = next;
        }
    }

    if (webview)
    {
        FOREACH(it, m_pairList)
        {
            if (it->first == webview)
            {
                // PairList still has webview usage.
                // Do not clean-up webview data.
                return;
            }
        }
        removeWebview(webview);
    }
}

void PermissionPopupManager::addWebview(Evas_Object* webview)
{
    evas_object_event_callback_add(webview, EVAS_CALLBACK_DEL, deleteWebviewCallback, this);
}

void PermissionPopupManager::addPopup(Evas_Object* popup)
{
    evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, deletePopupCallback, this);
}

void PermissionPopupManager::removeWebview(Evas_Object* webview)
{
    evas_object_event_callback_del(webview, EVAS_CALLBACK_DEL, deleteWebviewCallback);
}

void PermissionPopupManager::removePopup(Evas_Object* popup)
{
    evas_object_event_callback_del(popup, EVAS_CALLBACK_DEL, deletePopupCallback);
}

void PermissionPopupManager::deleteWebviewCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo)
{
    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(e);
    DPL_UNUSED_PARAM(eventInfo);

    PermissionPopupManager* This = static_cast<PermissionPopupManager*>(data);
    This->unregisterWebview(obj);
}

void PermissionPopupManager::deletePopupCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo)
{
    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(e);
    DPL_UNUSED_PARAM(eventInfo);

    PermissionPopupManager* This = static_cast<PermissionPopupManager*>(data);
    This->unregisterPopup(obj);
}

