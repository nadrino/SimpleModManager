/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LOGGING_H
#define __LOGGING_H

#include <iostream>
#include "nxlink.h"

#define VERBOSE 0
#define INFO 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

extern int verbose_level;
extern char log_level_color[5][16];

#define LOG(level) std::cout << "\n" << (nxlink ? log_level_color[level] : "")

#define VLOG_IS_ON(verboselevel) (verboselevel <= verbose_level)
#define VLOG(verboselevel) if(VLOG_IS_ON(verboselevel)) std::cout << "\n" << (nxlink ? log_level_color[VERBOSE]: "" )

#endif /* __LOGGING_H */
