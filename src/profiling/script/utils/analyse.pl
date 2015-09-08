#!/usr/bin/env perl
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

use strict;
use warnings;
use diagnostics;

my ($regular_out_file, $range_out_file) = @ARGV;

print "Script requires two arguments,
which are file names to store the output\n
Regular points will be stored in first file,
ranges in second one.\n" and exit unless (defined $regular_out_file and defined $range_out_file);


print "Starting analysis\n";
my @lines = <STDIN>;

my @packets;
foreach my $line (@lines)
{
    chomp($line);
    my @splitted = split(/#/, $line);
    my $prefix = $splitted[0];
    my $name = $splitted[1];
    my $time = $splitted[2];
    my $description = $splitted[3];
    my %packet;
    $packet{time} = $time;
    $packet{prefix} = $prefix;
    $packet{name} = $name;
    $packet{description} = $description;
    push @packets, \%packet;
}

@packets = sort { $$a{time} <=> $$b{time} } @packets;

my $starttime;
my %tmps;
my @ranges;

foreach my $packet (@packets)
{
    my $name = $$packet{name};
    my $prefix = $$packet{prefix};

    if (not defined $starttime)
    {
        if (defined $name and $name eq "Profiling_Started")
        {
            $starttime = $$packet{time};
        }
        else
        {
            print "***** No Profiling Start Point Found *****\n";
            exit();
        }
    }
    $$packet{time} = $$packet{time} - $starttime;

    if ($prefix eq "start")
    {
        if (exists($tmps{$name}))
        {
            print "***** Doubled start point found $name *****\n";
            print "[$name]\n";
            exit();
        }
        $tmps{$name} = \%$packet;
    }
    elsif ($prefix eq "stop")
    {
        if (not exists($tmps{$name}))
        {
            print "***** No start point found *****\n";
            exit();
        }
        my %tmp = %{$tmps{$name}};
        my %newpacket = %$packet;
        $newpacket{time} = $newpacket{time} - $tmp{time};
        delete ($tmps{$name});
        my $found;
        foreach my $p (@ranges)
        {
            if ($$p{name} eq $newpacket{name})
            {
                $$p{time} += $newpacket{time};
                $found = 1;
            }
        }
        if (not defined($found))
        {
            push(@ranges, \%newpacket);
        }
    }
}

open REGULAR_OUT_FILE, ">", $regular_out_file or die $!;

print REGULAR_OUT_FILE "name,prefix,timestamp[µs],timespan[µs]\n";
my $lastTime;
foreach my $point (@packets)
{
    if (not defined $lastTime)
    {
        $lastTime = $$point{time};
    }
    my $toPrint = sprintf("%s-%s,%lu,%lu\n", $$point{name}, $$point{prefix}, $$point{time}, $$point{time} - $lastTime);
    print REGULAR_OUT_FILE $toPrint;
    $lastTime = $$point{time};
}
close REGULAR_OUT_FILE;

open RANGE_OUT_FILE, ">", $range_out_file or die $!;
print RANGE_OUT_FILE "range name,timespan[µs]\n";
foreach my $range (@ranges)
{
    my $res = sprintf("%s,%lu\n", $$range{name}, $$range{time});
    print RANGE_OUT_FILE $res;
}
close RANGE_OUT_FILE;

