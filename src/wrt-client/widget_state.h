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
 * @file    core_module.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   File contains declaration of WidgetState enum.
 */

#ifndef WIDGET_STATE_H
#define WIDGET_STATE_H

/**
 * @brief State of widget
 *
 * This enumerator describes a state that widget is in.
 * Notice, that not all combinations of state changes are correct.
 * In current architecture, only following state changes are corrent and can
 * occur:
 *
 * oldState: WidgetState_Stopped     -> newState: WidgetState_Authorizing
 * oldState: WidgetState_Authorizing -> newState: WidgetState_Running
 * oldState: WidgetState_Authorizing -> newState: WidgetState_Stopped
 * oldState: WidgetState_Running     -> newState: WidgetState_Stopped
 * oldState: WidgetState_Running     -> newState: WidgetState_Suspended
 * oldState: WidgetState_Suspended   -> newState: WidgetState_Running
 * oldState: WidgetState_Stopped     -> newState: WidgetState_Uninstalling
 *
 * WidgetState_Stopped      - Widget is in stopped state. No view is currently
 *                            attached to WidgetModel.
 * WidgetState_Authorizing  - Widget is in authorizing state. No view is
 *                            currently attached to WidgetModel.
 * WidgetState_Running      - Widget is in running state. There is a view
 *                            attached to WidgetModel.
 * WidgetState_Suspended    - Widget is in running state but its java script has
 *                            been suspended. There is a view attached to
 *                            WidgetModel.
 * WidgetState_Uninstalling - Widget is currently being uninstalled.
 * WidgetState_Running_Nested_Loop - Widget has created nested loop. We cannot
 *                            close widget in this state. We must wait until
 *                            widget will be in WidgetState_Running
 */
enum WidgetState
{
    WidgetState_Stopped,
    WidgetState_Authorizing,
    WidgetState_Running,
    WidgetState_Suspended,
    WidgetState_Uninstalling,
    WidgetState_Running_Nested_Loop
};

#endif /* WIDGET_STATE_H */

