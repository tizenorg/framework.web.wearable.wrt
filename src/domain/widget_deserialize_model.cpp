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
 * @file    widget_deserialize_model.h
 * @author  Piotr Marcinkiewicz (p.marcinkiew@samsung.com)
 * @version 1.0
 * @brief   Widget deserialization creates WidgetModel from WidgetDAOReadOnly
 */

#include "widget_model.h"

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/widget_config.h>
#include <dpl/log/log.h>
#include <dpl/optional_typedefs.h>
// to apply widget default locales instead of calling localizeWidgetModel()
#include <dpl/localization/LanguageTagsProvider.h>

namespace Domain {
std::string getTimestamp()
{
    struct timeval tv;
    char buff[128];

    gettimeofday(&tv, NULL);
    sprintf(buff, "%lf", (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0f);
    LogDebug("timestamp: " << buff);
    return std::string(buff);
}

std::shared_ptr<WidgetModel> deserializeWidgetModel(const std::string& tizenId)
{
    std::shared_ptr<WidgetModel> model;
    DPL::String dplTizenId(DPL::FromUTF8String(tizenId));
    if (WrtDB::WidgetDAOReadOnly::isWidgetInstalled(dplTizenId)) {
        LogDebug("Widget installed - creating model");
        model.reset(new WidgetModel(tizenId));

        WrtDB::WidgetDAOReadOnly dao(dplTizenId);
        DPL::String pkgId = dao.getTizenPkgId();
        model->PersistentStoragePath.Set(
            DPL::FromUTF8String(
                WrtDB::WidgetConfig::GetWidgetPersistentStoragePath(pkgId)));
        model->TemporaryStoragePath.Set(
            DPL::FromUTF8String(
                WrtDB::WidgetConfig::GetWidgetTemporaryStoragePath(pkgId)));

        DPL::OptionalString defloc = model->defaultlocale.Get();
        if (!!defloc) {
            LanguageTagsProviderSingleton::Instance().addWidgetDefaultLocales(
                *defloc);
        }

        WrtDB::WidgetAccessInfoList widgetAccessInfoList;
        // widgetAccessInfoList is output parameter
        dao.getWidgetAccessInfo(widgetAccessInfoList);
        model->AccessList.Set(widgetAccessInfoList);

        // Widget app-control information data
        WrtDB::WidgetAppControlList widgetApplicationControlList;
        // widgetApplicationControlList is output parameter
        dao.getAppControlList(widgetApplicationControlList);
        model->AppControlList.Set(widgetApplicationControlList);

        // Set Widget Settings
        WrtDB::WidgetSettings widgetSettings;
        dao.getWidgetSettings(widgetSettings);
        model->SettingList.Set(widgetSettings);
    } else {
        LogError("Widget is not installed - model not created");
    }
    return model;
}

} //Namespace Domain

