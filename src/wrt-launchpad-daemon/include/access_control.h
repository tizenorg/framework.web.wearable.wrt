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

#ifndef __ACCESS_CONTROL_H_
#define __ACCESS_CONTROL_H_

#ifdef DAC_ACTIVATE

#include <privilege-control.h>
#include <sys/smack.h>


#define INHOUSE_UID     5000

static int __add_smack_rule(const char* subject, const char* object, const char *access){
    int ret = 0;
    struct smack_accesses *rules = NULL;

    if( (ret = smack_accesses_new(&rules)) != 0 ){
        _E("error smack_accesses_new %d", ret);
        return -1;
    }
    if( (ret = smack_accesses_add(rules, subject, object, access)) != 0){
        _E("error smack_accesses_add %d", ret);
        goto end;;
    }
    if( (ret = smack_accesses_apply(rules)) != 0){
        _E("error smack_accesses_apply %d", ret);
    }

end:
    smack_accesses_free(rules);
    return ret;
}

static inline int __set_access(const char* pkg_name,
                               const char* pkg_type,
                               const char* app_path)
{
    return perm_app_set_privilege(pkg_name, pkg_type, app_path);
}

#else

static inline int __set_access(const char* pkg_name,
                               const char* pkg_type,
                               const char* app_path)
{
    return 0;
}

#endif

#endif //__ACCESS_CONTROL_H_

