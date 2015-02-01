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

WARM_FILE=$1
COLD_FILE=$2
PRELOAD_FILE=$3
OUTPUT_FILE=$4
PLOT_TITLE=$5
FONT_SIZE=$6
WIDTH=$7
HEIGHT=$8
MAXTICS=$9
DIFFFILE=${10}

gnuplot \
    -e "warm_file='$WARM_FILE'" \
    -e "cold_file='$COLD_FILE'" \
    -e "preload_file=\"$PRELOAD_FILE\"" \
    -e "output_file='$OUTPUT_FILE'" \
    -e "plot_title='$PLOT_TITLE'" \
    -e "font_size='$FONT_SIZE'" \
    -e "width='$WIDTH'" \
    -e "height='$HEIGHT'" \
    -e "maxtics='$MAXTICS'" \
    -e "difffile='$DIFFFILE'" \
    ./utils/plot-generator.gplot
