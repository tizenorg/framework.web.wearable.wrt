#!/bin/bash
# Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# resets all databases and security daemon
wrt_reset_db.sh "$@"
pkill -9 wrt-security
pkill -9 security-serv
security-server </dev/null 1>/dev/null 2>/dev/null &
wrt-security-daemon </dev/null 1>/dev/null 2>/dev/null &

