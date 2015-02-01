#!/usr/bin/env python
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

import os
import re


def countLines(path):
    with open(path) as f:
        return len(f.readlines())

# RETURNS: (
#    short description (string or None)
#    long decsription (array of strings or None)
#    options: stop
def parseDescr(lines):
    if len(lines) == 0:
        return (None, None, False)
    linesRest = None
    if re.match( r"!!!options!!!", lines[0] ):
        optStop = True
        linesRest = lines[1:]
    else:
        optStop = False
        linesRest = lines
    if len(linesRest) == 0:
        return(None,None,optStop)
    short = linesRest[0].rstrip()
    long = []
    for l in linesRest[1:]:
        ll = l.rstrip()
        if re.search( r"\S", ll ):
            long.append( ll )
    if len(long) == 0:
        long = None

    return (short, long, optStop)

# RETURNS a tree with nodes like: (
#    path (string)
#    short description (string or None)
#    long decsription (array of strings or None)
#    LOC (integer)
#    list of subdirs (child nodes like this one)
def parseDir(path):
    short = None
    long = None
    optStop = False
    try:
        with open( path+'/DESCRIPTION' ) as f:
            short, long, optStop = parseDescr( f.readlines() )
    except IOError:
        pass
    dirs = []
    cntLines = 0
    for fname in os.listdir(path):
        if fname != '.git' and os.path.isdir(path+'/'+fname):
            subdir = parseDir(path+'/'+fname)
            if optStop == False:
                dirs.append(subdir)
            (dummy0, dummy1, dummy2, subLines, dummy4) = subdir
            cntLines += subLines
  
        if os.path.isfile(path+'/'+fname) \
        and not os.path.islink(path+'/'+fname):
            cntLines += countLines(path+'/'+fname)

    return path, short, long, cntLines, dirs


###     ##### PRINT AS TEXT
###     
###     def printTextSub(path,indent,withLongDesc):
###         short, long, dirs, loc = parseDir(path)
###         if short == None: 
###             p = re.sub(r"^\./", '', path)
###             print '%s%s -- ' % (indent, p)
###         else:
###             p = re.sub(r"^\./", '', path)
###             print '%s%s -- %s' % (indent, p, short)
###         if withLongDesc:
###             if long != None:
###                 print ''
###                 for line in long:
###                     print '%s%s' % (indent+'    ',line)
###                 print ''
###         for dir in dirs:
###             printTextSub(path+'/'+dir, indent+'    ', withLongDesc)
###     
###     def printText(path,withLongDesc):
###         printTextSub(path,'',withLongDesc)
###     
###     def printTextWoMain(path,withLongDesc):
###         short, long, dirs, loc = parseDir(path)
###         for dir in dirs:
###             printTextSub(path+'/'+dir, '', withLongDesc)
###     

##### PRINT AS a sort of CSV delimited by '|'

# indent is a number (0..)
def printTabSub(tree,indent):
    path, short, long, loc, subdirs = tree 
    p = re.sub(r"^\./", '', path)
    m = re.search(r"/([^/]*$)", p)
    if m != None: p = m.groups()[0]
    if short == None: 
        print '%s%s|%d|' % ("        "*indent, p, loc)
    else:
        print '%s%s|%d|%s' % ("        "*indent, p, loc, short)
    for dir in subdirs:
        printTabSub(dir, indent+1)

def printTab(tree):
    printTabSub(tree,0)

def printTabWoMain(tree):
    path, short, long, loc, dirs = tree
    for dir in dirs:
        printTabSub(dir, 0)


##### MAIN

tree = parseDir('.')
printTabWoMain(tree)

