#!/usr/bin/python
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

import os,sys

def main():
  if sys.version[0] != "2":
    exit(1)

  f = open('ranges', 'r')
  # Array of ranges.
  ranges = []
  for line in f:
    r = line.rstrip().split(':')
    r.append(0)
    r.append(0)
    r.append(0)
    ranges.append(r)
  f.close()

  # find directories with results
  for root, dirs, files in os.walk('OUTPUT'):
    path = root.rsplit('/',1)
    if path[len(path)-1] not in ['cold','warm','preload']:
      continue

    # open file with measurements
    print '\n' + root
    output = open(root + '/outpucik-points', 'r')

    # find matching points
    for r in ranges:
      output.seek(0)
      for l in output:
        if r[1] in l:
          r[3] = l.split(',')[1]
        if r[2] in l:
          r[4] = l.split(',')[1]

    output.close()

    # calculate the difference
    for r in ranges:
      r[5] = int(r[4]) - int(r[3])

    # save calculations
    results = open(root + '/outpucik-results','w')
    for r in ranges:
      print " " + r[0] + ": " + str(float(r[5])/1000) + "ms"
      results.write(r[0] + ": " + str(float(r[5])/1000) + "ms\n")
    results.close()

if __name__ == "__main__":
  main()

