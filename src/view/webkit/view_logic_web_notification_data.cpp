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
 *
 * @file    view_logic_web_notification_data.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 *
 */

#include "view_logic_web_notification_data.h"

#include <stdint.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace ViewModule {
WebNotificationData::WebNotificationData(Ewk_Notification* ewkNotification) :
    m_notificaionId(0),
    m_data(ewkNotification)
{}

WebNotificationData::~WebNotificationData()
{}

int WebNotificationData::getNotiId()
{
    return m_notificaionId;
}

void WebNotificationData::setNotiId(const int notiId)
{
    m_notificaionId = notiId;
}

uint64_t WebNotificationData::getEwkNotiId()
{
    return ewk_notification_id_get(m_data);
}

const char* WebNotificationData::getIconUrl()
{
    return ewk_notification_icon_url_get(m_data);
}

const char* WebNotificationData::getTitle()
{
    return ewk_notification_title_get(m_data);
}

const char* WebNotificationData::getBody()
{
    return ewk_notification_body_get(m_data);
}

Ewk_Notification* WebNotificationData::getData()
{
    return m_data;
}
} //namespace ViewModule
