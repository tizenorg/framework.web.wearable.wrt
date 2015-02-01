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
 * @file    widget_model.h
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Header file for widget model
 */
#ifndef SRC_DOMAIN_WIDGET_MODEL_H
#define SRC_DOMAIN_WIDGET_MODEL_H

#include <memory>
#include <string>

#include <dpl/event/model.h>
#include <dpl/event/property.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/wrt_db_types.h> // definition of WidgetHandle

#include "widget_data_types.h"

/**
 * @brief Widget model
 *
 * Widget model is the core object that hold information about
 * properties of widget. After wrt launch each widget contained in database is
 * mapped to WidgetModel.
 *
 * Widget model is a type of MVC model, so it is possible to listen for it's
 * changes.
 *
 */
class WidgetModel : public DPL::Event::Model
{
  public:

    /**
     * @brief Tizen id
     *
     *  ex> "TizenIDabc.appname"
     *
     *  - TizenId / AppId : "TizenIDabc.appname"
     *  - TzPkgId         : "TizenIDabc"
     *  - App name        : "appname"
     *
     */
    DPL::String TizenId;
    DPL::Event::Property<WrtDB::TizenPkgId,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> TzPkgId;

    /**
     * @brief Widget type
     *
     * Note: This is a readonly property
     */
    DPL::Event::Property<WrtDB::WidgetType,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> Type;

    /**
     * @brief Config file based csp policy
     */
    DPL::Event::Property<DPL::OptionalString,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> CspPolicy;

    /**
     * @brief Config file based csp policy - report only
     */
    DPL::Event::Property<DPL::OptionalString,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached>
    CspReportOnlyPolicy;

    /**
     * @brief Start URL for widget
     */
    DPL::Event::Property<DPL::OptionalString> StartURL;

    /**
     * @brief Start URL information for widget
     */
    DPL::Event::Property<OptionalWidgetStartFileInfo> StartFileInfo;

    /**
     * @brief Prefix URL for widget
     *
     * This is a prefix address of html file that widget is displaying.
     * The whole address is PrefixURL + StartURL.
     */
    DPL::Event::Property<DPL::String, DPL::Event::PropertyReadOnly> PrefixURL;

    /**
     * @brief Install path for widget
     *
     * Gets path in which files of widget are being kept
     */
    DPL::Event::Property<DPL::String,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> InstallPath;

    /**
     * @brief Path to widget's persistent storage.
     *
     * Gets path in which widget may store its persistent private data.
     */
    DPL::Event::Property<DPL::String> PersistentStoragePath;

    /**
     * @brief Path to widget's temporary storage.
     *
     * Gets path in which widget may store its temporary private data.
     */
    DPL::Event::Property<DPL::String> TemporaryStoragePath;

    /**
     * @brief Widget defaultlocale
     */
    DPL::Event::Property<DPL::OptionalString,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached>
    defaultlocale;

    /**
     * @brief Widget name
     */
    DPL::Event::Property<DPL::OptionalString> Name;

    /**
     * @brief Widget short name
     */
    DPL::Event::Property<DPL::OptionalString> ShortName;

    /**
     * @brief Widget description
     */
    DPL::Event::Property<DPL::OptionalString> Description;

    /**
     * @brief Widget license
     */
    DPL::Event::Property<DPL::OptionalString> License;

    /**
     * @brief Widget license href
     */
    DPL::Event::Property<DPL::OptionalString> LicenseHref;

    /**
     * @brief Widget icon
     */
    DPL::Event::Property<OptionalWidgetIcon> Icon;

    /**
     * @brief Widget splash image src
     */
    DPL::Event::Property<DPL::OptionalString,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> SplashImg;

    /**
     * @brief window mode
     */
    DPL::Event::Property<WrtDB::WindowModeList,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamic> WindowModes;

    //    /**
    //     * @brief Value of network element.
    //     */
    //    DPL::Event::Property<bool,
    //                  DPL::Event::PropertyReadOnly> AccessNetwork;

    //    /**
    //     * @brief Does widget contain WARP definitions.
    //     */
    //    DPL::Event::Property<bool> WarpDefinitionEmpty;

    /**
     * @brief Is back supported
     */
    DPL::Event::Property<bool,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached>
    BackSupported;

    /**
     * @brief Widget access list
     */
    DPL::Event::Property<WidgetAccessList> AccessList;

    DPL::Event::Property<WidgetSettingList> SettingList;

    /**
     * @brief Widget app-control list
     */
    DPL::Event::Property<WrtDB::WidgetAppControlList> AppControlList;

    DPL::Event::Property<WrtDB::WidgetSecurityModelVersion,
                         DPL::Event::PropertyReadOnly,
                         DPL::Event::PropertyStorageDynamicCached> SecurityModelVersion;

    WidgetModel(const std::string &tizenId);

  private:
    DPL::String getTizenId() const;
};
typedef std::shared_ptr<WidgetModel> WidgetModelPtr;

#endif // SRC_DOMAIN_WIDGET_MODEL_H
