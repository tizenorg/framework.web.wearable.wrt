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
 * @file    view_logic_web_notification_data.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 *
 */

#ifndef VIEW_LOGIC_WEB_NOTIFICATION_DATA_H_
#define VIEW_LOGIC_WEB_NOTIFICATION_DATA_H_

#include <stdint.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <memory>

namespace ViewModule {
class WebNotificationData
{
  public:
    WebNotificationData(Ewk_Notification* ewkNotification);
    virtual ~WebNotificationData();

    int getNotiId(void);
    void setNotiId(const int notiId);
    uint64_t getEwkNotiId(void);
    const char* getIconUrl(void);
    const char* getTitle(void);
    const char* getBody(void);
    Ewk_Notification* getData(void);

  private:
    int m_notificaionId;
    Ewk_Notification* m_data;
};
typedef std::shared_ptr<WebNotificationData> WebNotificationDataPtr;
} // namespace ViewModule
#endif // VIEW_LOGIC_WEB_NOTIFICATION_DATA_H_
