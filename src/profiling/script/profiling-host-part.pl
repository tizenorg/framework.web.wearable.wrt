#!/usr/bin/perl
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

print "Stage 1:\n";
my @files = `find -name prof-raw-data 2>/dev/null`;
for my $file (@files) {
 chomp $file;
  my $foutPoints = $file; $foutPoints =~ s/raw-data/-data-points/g;
  my $foutRanges = $file; $foutRanges =~ s/raw-data/-data-ranges/g;
  #print ">$file< >$fout<\n";
  #print ">>>~/git/wrt-engine/src/profiling/script/analyse.pl $foutPoints $foutRanges <<<\n";
  print "grep -v \"PROFILING\" $file | ./utils/analyse.pl $foutPoints $foutRanges\n";
  `grep -v \"PROFILING\" $file | ./utils/analyse.pl $foutPoints $foutRanges`;
}

print "Stage 2:\n";

for my $dir (qw{OUTPUT/minimal/cold OUTPUT/minimal/warm OUTPUT/minimal/preload OUTPUT/Jet_Pack_Agent/cold OUTPUT/Jet_Pack_Agent/warm OUTPUT/Jet_Pack_Agent/preload OUTPUT/UnitTest/cold OUTPUT/UnitTest/warm OUTPUT/UnitTest/preload})
{
    print $dir, "\n";
    print "./utils/csv-stats.pl -c -r -s avg \`find $dir -name prof--data-points 2>/dev/null\`  > $dir/outpucik-points\n";
    print "`./utils/csv-stats.pl -c -r -s avg \`find $dir -name prof--data-ranges 2>/dev/null\`  > $dir/outpucik-ranges\n";
    `./utils/csv-stats.pl -c -r -s avg \`find $dir -name prof--data-points 2>/dev/null\`  > $dir/outpucik-points`;
    `./utils/csv-stats.pl -c -r -s avg \`find $dir -name prof--data-ranges 2>/dev/null\`  > $dir/outpucik-ranges`;
}

print "Stage 3:\n";
for my $dir (qw{OUTPUT/minimal OUTPUT/Jet_Pack_Agent OUTPUT/UnitTest})
{
    my $name = substr($dir, 7);
    my $file = "$name.svg";
    my $warm = "$dir/warm/outpucik-points";
    my $cold = "$dir/cold/outpucik-points";
    my $preload = "$dir/preload/outpucik-points";
    my $difffile = "$dir/diff-points";
    `paste -d, $cold $warm $preload > $difffile`;
    my $title = "\"Performance profiling of WebRuntime - $name Widget\"";

    my $font = 12;

    my $wheader = 213;
    my $hheader = 199;
    my $max = 8100;

    print "+++++++++++++++++++++++++++ $name\n";


    my $maxtime = `cat $dir/cold/outpucik-points | tail -n1 | cut -d, -f2`;
    my $nroftics = int($maxtime/100000 + 1);
    my $maxtics = $nroftics * 100000;
    print "NROFTIC: $nroftics\n";
    my $height = int(20.7 * $nroftics + $hheader);
    if ($height > $max)
    {
        $height = $max;
    }

    my $lines_count = `wc -l $warm`;
    $lines_count = $lines_count - 2;
    print "lc: $lines_count\n";
    my $width = int(23.5 * $lines_count + $wheader);
    print "wi: $width\n";
    if ($width > $max)
    {
        my $newfont = int ($font * $max/$width);
        $width = $max;
        $height = $hheader + int (($height - $hheader)*$newfont/$font);
        $font = $newfont
    }

    print "Generating $file W: $width H: $height F: $font\n\n";
    print "./utils/generate-plot.sh $warm $cold $preload $file $title $font $width $height $maxtics\n\n\n";
    `./utils/generate-plot.sh $warm $cold $preload $file $title $font $width $height $maxtics $difffile`;
}

