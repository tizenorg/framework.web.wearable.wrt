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
/**
 * @file    view_logic_web_notification_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @brief   Implementation file of web Notification API used by ViewLogic
 */

#include "view_logic_web_notification_support.h"
#include "view_logic_web_notification_data.h"

#include <unistd.h>

#include <stdint.h>
#include <string>
#include <map>
#include <unistd.h>
#include <dpl/log/secure_log.h>
#include <dpl/assert.h>
#include <dpl/exception.h>
#include <dpl/wrt-dao-ro/widget_config.h>

#include <notification.h>
#include <notification_internal.h>
#include <pcrecpp.h>
#include <sstream>
#include <curl/curl.h>

namespace ViewModule {
namespace {
const char* PATTERN_CHECK_EXTERNAL = "^http(s)?://\\w+.*$";

// Function declare
bool isExternalUri(const std::string &uri);
size_t curlWriteData(void* ptr, size_t size, size_t nmemb, FILE* stream);

bool isExternalUri(const std::string &uri)
{
    pcrecpp::RE_Options pcreOpt;
    pcreOpt.set_caseless(true);
    pcrecpp::RE re(PATTERN_CHECK_EXTERNAL, pcreOpt);

    return re.FullMatch(uri);
}

size_t curlWriteData(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
} // anonymous namespace

// Implementation class
class WebNotificationSupportImplementation
{
  private:
    class Exception
    {
      public:
        DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
        DECLARE_EXCEPTION_TYPE(Base, InitError)
        DECLARE_EXCEPTION_TYPE(Base, NotificationShowError)
        DECLARE_EXCEPTION_TYPE(Base, DownloadImageError)
    };

    bool m_initialized;
    std::string m_persistentPath;

    typedef std::map<uint64_t, WebNotificationDataPtr> NotiMap;
    typedef std::map<uint64_t, WebNotificationDataPtr>::iterator NotiMapIt;
    NotiMap m_notiDataMap;

    std::string createDownloadPath(const std::string& iconUrl)
    {
        // Make valid filename
        // If there is already exist filename, rename to "filename_%d"

        std::string downloadPath = m_persistentPath;
        std::string fileName = iconUrl.substr(iconUrl.rfind('/') + 1);
        _D("fileName from URL: %s", fileName.c_str());
        std::string rename = fileName;
        unsigned int renameSuffixNb = 0;
        while (0 == access((m_persistentPath + rename).c_str(), F_OK)) {
            std::ostringstream suffixOstr;
            suffixOstr.str("");
            suffixOstr << fileName << "_" << renameSuffixNb++;

            rename = fileName;
            rename.insert(rename.find('.'), suffixOstr.str());
        }

        downloadPath += rename;
        _D("Valid file path : %s", downloadPath.c_str());
        return downloadPath;
    }

    std::string downloadImage(const std::string& iconUrl)
    {
        if (iconUrl.empty()) {
            _W("Icon url is empty");
            return std::string();
        }
        std::string downloadPath = createDownloadPath(iconUrl);

        // Download image by curl
        FILE *fp = NULL;
        CURL *curl = NULL;

        Try {
            curl = curl_easy_init();
            if (NULL == curl) {
                _W("fail to curl_easy_init");
                Throw(Exception::InitError);
            }
            fp = fopen(downloadPath.c_str(), "wb");
            if (fp == NULL) {
                _W("fail to open");
                Throw(Exception::InitError);
            }

            curl_easy_setopt(curl, CURLOPT_URL, iconUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteData);

            CURLcode err = curl_easy_perform(curl);
            if (0 != err) {
                _W("fail to curl_easy_perform: %d", err);
                Throw(Exception::DownloadImageError);
            }
            fclose(fp);
            curl_easy_cleanup(curl);
        } Catch(DPL::Exception) {
            fclose(fp);
            curl_easy_cleanup(curl);
            return std::string();
        }
        _D("Download success");
        return downloadPath;
    }

    int showNotification(WebNotificationDataPtr notiData)
    {
        Assert(notiData);
        notification_h noti_h = NULL;
        int error = NOTIFICATION_ERROR_NONE;

        Try {
            // create notification
            noti_h = notification_new(
                    NOTIFICATION_TYPE_NOTI,
                    NOTIFICATION_GROUP_ID_DEFAULT,
                    NOTIFICATION_PRIV_ID_NONE);
            if (!noti_h) {
                _E("Fail to notification_new");
                Throw(Exception::NotificationShowError);
            }

            // set notification title
            error = notification_set_text(
                    noti_h,
                    NOTIFICATION_TEXT_TYPE_TITLE,
                    notiData->getTitle(),
                    NULL,
                    NOTIFICATION_VARIABLE_TYPE_NONE);
            if (error != NOTIFICATION_ERROR_NONE) {
                _E("Fail to set title");
                Throw(Exception::NotificationShowError);
            }

            // set notification content
            error = notification_set_text(
                    noti_h,
                    NOTIFICATION_TEXT_TYPE_CONTENT,
                    notiData->getBody(),
                    NULL,
                    NOTIFICATION_VARIABLE_TYPE_NONE);
            if (error != NOTIFICATION_ERROR_NONE) {
                _E("Fail to set content: %d", error);
                Throw(Exception::NotificationShowError);
            }

            //check uri is "http", https" or not
            if (notiData->getIconUrl()) {
                std::string iconPath;
                if (isExternalUri(notiData->getIconUrl())) {
                    //download image from url
                    iconPath = downloadImage(notiData->getIconUrl());
                }

                //set image
                // If fail to download external image, skip to set image.
                // In this case, set to default package image.
                if (!iconPath.empty()) {
                    error = notification_set_image(
                            noti_h,
                            NOTIFICATION_IMAGE_TYPE_ICON,
                            iconPath.c_str());
                    if (error != NOTIFICATION_ERROR_NONE) {
                        _E("Fail to free notification: %d", error);
                        Throw(Exception::NotificationShowError);
                    }
                }
            }

            // insert notification
            int privId = NOTIFICATION_PRIV_ID_NONE;
            error = notification_insert(noti_h, &privId);
            if (error != NOTIFICATION_ERROR_NONE) {
                _E("Fail to insert notification : %d", error);
                Throw(Exception::NotificationShowError);
            }
            _D("ewkId=[%u], notiId=[%d]", notiData->getEwkNotiId(), privId);

            if (noti_h) {
                error = notification_free(noti_h);
                if (error != NOTIFICATION_ERROR_NONE) {
                    _E("Fail to free notification : %d", error);
                    Throw(Exception::NotificationShowError);
                }
                noti_h = NULL;
            }
            notiData->setNotiId(privId);
        } Catch(Exception::NotificationShowError) {
            notification_free(noti_h);
            noti_h = NULL;
            return false;
        } Catch(DPL::Exception) {
            _E("Fail to show notification");
            return false;
        }
        return true;
    }

    bool hideNotification(uint64_t ewkNotiId)
    {
        int error = NOTIFICATION_ERROR_NONE;
        NotiMapIt it = m_notiDataMap.find(ewkNotiId);
        error =
            notification_delete_by_priv_id(NULL,
                                           NOTIFICATION_TYPE_NOTI,
                                           it->second->getNotiId());
        if (error == NOTIFICATION_ERROR_NONE) {
            _D("Success to hide");
            return true;
        } else if (error == NOTIFICATION_ERROR_NOT_EXIST_ID) {
            _D("Success to hide. Not exist noti");
            return true;
        } else {
            _W("Error to hide : %d", error);
            return false;
        }
    }

    // manage noti data
    bool isExistData(uint64_t ewkNotiId)
    {
        if (m_notiDataMap.find(ewkNotiId) == m_notiDataMap.end()) {
            return false;
        }
        return true;
    }

    void insertData(WebNotificationDataPtr notiData)
    {
        m_notiDataMap[notiData->getEwkNotiId()] = notiData;
    }

    void removeData(uint64_t ewkNotiId)
    {
        m_notiDataMap.erase(ewkNotiId);
    }

    WebNotificationDataPtr getData(uint64_t ewkNotiId)
    {
        return m_notiDataMap.find(ewkNotiId)->second;
    }

  public:
    WebNotificationSupportImplementation() :
        m_initialized(false)
    {
    }

    void initialize(WrtDB::TizenPkgId pkgId)
    {
        // icon download path
        // /opt/apps/tizen_id/data + '/' + filename
        m_persistentPath =
            WrtDB::WidgetConfig::GetWidgetPersistentStoragePath(pkgId) + '/';
        _D("path %s", m_persistentPath.c_str());
        m_initialized = true;
    }

    void deinitialize(void)
    {
        _D("called");
        m_initialized = false;
    }

    bool show(WebNotificationDataPtr notiData)
    {
        Assert(m_initialized);
        if (isExistData(notiData->getEwkNotiId())) {
            // Web Notifications (http://www.w3.org/TR/notifications/)
            // 4.7 Replacing a notification
            //   3. If old is in the list of pending notifications, queue a
            //      task to replace old with new, in the same position, in the
            //      list of pending notifications, and fire an event named
            //      close on old.
            //   4. Otherwise, queue a task to replace old with new, in the
            //      same position, in the list of active notifications, fire
            //      an event named close on old, and fire an event named show
            //      on new.
            hideNotification(notiData->getEwkNotiId());
            removeData(notiData->getEwkNotiId());
        }
        if (showNotification(notiData)) {
            insertData(notiData);
            return true;
        }
        return false;
    }

    void* hide(uint64_t ewkNotiId)
    {
        Assert(m_initialized);
        if (!isExistData(ewkNotiId)) {
            _W("Noti isn't exist");
            return NULL;
        }
        if (!hideNotification(ewkNotiId)) {
            return NULL;
        }
        WebNotificationDataPtr data = getData(ewkNotiId);
        removeData(ewkNotiId);
        return static_cast<void*>(data->getData());
    }
};

WebNotificationSupport::WebNotificationSupport() :
    m_impl(new WebNotificationSupportImplementation())
{
}

WebNotificationSupport::~WebNotificationSupport()
{
}

void WebNotificationSupport::initialize(WrtDB::TizenAppId appId)
{
    m_impl->initialize(appId);
}

void WebNotificationSupport::deinitialize(void)
{
    m_impl->deinitialize();
}

bool WebNotificationSupport::show(WebNotificationDataPtr notiData)
{
    return m_impl->show(notiData);
}

void* WebNotificationSupport::hide(uint64_t ewkNotiId)
{
    return m_impl->hide(ewkNotiId);
}
} //namespace ViewModule
