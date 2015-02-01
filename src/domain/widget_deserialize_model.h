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
 * @brief   Widget deserialization creates WidgetModel from WidgetDAO
 */
#ifndef WRT_ENGINE_SRC_DOMAIN_WIDGET_DESERIALIZE_MODEL_H_
#define WRT_ENGINE_SRC_DOMAIN_WIDGET_DESERIALIZE_MODEL_H_

#include <memory>
#include <string>
#include <widget_model.h>
#include <dpl/wrt-dao-ro/wrt_db_types.h>
#include <dpl/optional_typedefs.h>

namespace Domain {
/**
 * @brief Creates widget model associated with selected
 * @param[in] tizenId
 * @param[in] service index, NULL for widget content
 * @retval WidgetModel
 */
 std::shared_ptr<WidgetModel> deserializeWidgetModel(const std::string& tizenId);
} //Namespace Domain

#endif // ifndef WRT_ENGINE_SRC_DOMAIN_WIDGET_DESERIALIZE_MODEL_H_

