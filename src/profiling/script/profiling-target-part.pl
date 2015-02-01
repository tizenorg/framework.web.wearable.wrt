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

##### CONFIGURATION begin #####

my $WIDGET_PACKAGE_DIR = './test-widgets';
my @WIDGETS = (
  ['minimal.wgt', 'minimal' ],
  ['0Jet_Pack_Agent.wgt', 'Jet Pack Agent'],
  ['UnitTestAuto.wgt', 'UnitTest' ],
);
my $WRT_INSTALL_ENV = 'WRT_TEST_MODE=1';

my $WRT_CLIENT_LAUNCH = 'wrt-client -l {}';
my $WRT_CLIENT_QUERY = 'wrt-launcher -l 2>/dev/null';
my $WRT_CLIENT_INSTALL = "$WRT_INSTALL_ENV wrt-installer -i {}";
my $WRT_CLIENT_UNINSTALL = "$WRT_INSTALL_ENV wrt-installer -un {}";
my $COLD_START_PREPARE_COMMAND = '/sbin/sysctl vm.drop_caches=3';
my $OUTPUT_DIR = './OUTPUT';
my $PRELOAD_LIBRARIES = './utils/wrt-preloader';

##### CONFIGURATION end #####


use strict;
use Time::HiRes;
use Time::Local;
use List::Util;


##### Single test runners

# $stream may be undef - then no output
sub runWidget {
  my $widgetTizenId = shift;
  my $stream = shift;
  my $streamout = shift;
  my $fnameerr = shift;

  unless (defined $fnameerr) {
    $fnameerr = '/dev/null';
  }

  `mkdir -p $OUTPUT_DIR/tmp`;

  my $fnameerr = "$OUTPUT_DIR/tmp/additional.tmp";

  my $command = $WRT_CLIENT_LAUNCH;
  $command =~ s/\{\}/$widgetTizenId/;
  $command .= " 2>$fnameerr 3>$OUTPUT_DIR/tmp/prof.tmp & echo TEST-WIDGET-PID \$\!";

  print "$command\n";

  my ($startSec, $startUSec) = Time::HiRes::gettimeofday();

  my $f;
  open $f, "$command |";

  my $pid = undef;

  my $extralines  = "";

  while (my $line = <$f>) {
    print "The line is: $line";
    if (defined $streamout) {
      print $streamout $line;
    }
    chomp $line;
    if (my ($p) = $line =~ /^TEST-WIDGET-PID (\d+)/) {
        $pid = 0+$p;
    } elsif ($line =~ /launched$/) {
        print "launched detected $pid\n";
        kill 10, $pid;
        my $again = 1;
        while ($again) {
            my $t = `tail -1 $OUTPUT_DIR/tmp/prof.tmp`;
            if ($t =~ /###PROFILING###STOP###/) {
                $again = 0;
            }
        }
        kill 9, $pid;
        print "killed\n";
    } elsif ($line =~ /\[([0-9]*):([0-9]*):([0-9]*).([0-9]*)\][^)]*\): *([A-Za-z0-9 ]+) *profiling##(start|stop)/) { #additionally take point from debug
        my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time());
        my $name = $5;
        $hour = $1;
        $min = $2;
        $sec = $3;
        my $msec = $4;
        my $type = $6;
        my $value = timelocal($sec,$min,$hour,$mday,$mon,$year);
        my $value = $msec + 1000 * $value;
        my $insert = "$type#$name#$value${msec}#[]\n";
        $extralines = "$extralines$insert";
    }
  }

  close $f;

  #TODO: do it perl way
  `cat $OUTPUT_DIR/tmp/prof.tmp | sort -k3,3 -t"#" -n > $OUTPUT_DIR/tmp/prof.tmp.new && mv $OUTPUT_DIR/tmp/prof.tmp.new $OUTPUT_DIR/tmp/prof.tmp`;

  open my $fin, "<", "$OUTPUT_DIR/tmp/prof.tmp";
  while (my $line = <$fin>) {
    chomp $line;
    if ($line =~ /^#Profiling_Started#/) {
        if (defined $stream) {
            printf $stream "#Profiling_Started#%d%06d#[]\n", $startSec, $startUSec;
        }
    } elsif ($line =~ /###PROFILING###STOP###/) {
        if (defined $stream) {
            print $stream "$extralines###PROFILING###STOP###\n";
        }
	} else {
        if (defined $stream) {
            print $stream "$line\n";
        }
    }
  }
  close $fin;
}

# $mode:
#   simple       - just run the widget, no special preparations
#   cold         - after cache clean
#   warm         - the same widget first launched w/o logs for "warm-up"
#   double-warm  - two warm-up launches prior to proper test
#   preload      - preloaded webkit library and its dependencies
#  TODO - OTHER MODES:
#     - warming-up with ANOTHER widget?
#     - warming-up by copying all required binaries/libraries to /dev/null
#     - running with all binaries/libraries placed on tmpfs (i.e. in cache)
#     - first clean cache, then fill it with garbage, by copying lots
#       of useless data from disk to /dev/null, until `free' shows small
#       enough free memory level
sub runTest {
    my $widgetTizenId = shift;
    my $mode = shift;
    my $stream = shift;
    my $streamout = shift;
    my $fnameerr = shift;

    if ($mode eq 'simple') {
        # no preparations necessary
    } elsif ($mode eq 'cold') {
        `$COLD_START_PREPARE_COMMAND`;
    } elsif ($mode eq 'warm') {
        runWidget( $widgetTizenId, undef, undef, undef );
    } elsif ($mode eq 'double-warm') {
        runWidget( $widgetTizenId, undef, undef, undef );
        runWidget( $widgetTizenId, undef, undef, undef );
    } elsif ($mode eq 'preload') {
        `$COLD_START_PREPARE_COMMAND`;
        `$PRELOAD_LIBRARIES`;
    } else {
        die;
    }
    runWidget( $widgetTizenId, $stream, $streamout, $fnameerr );
}

##### Widget installation

# returns \% a map: name->tizenId
sub dropAndInstallAll {
    my $pwidgets = shift;

    {
        print "Uninstalling all widgets\n";
        my @out = `$WRT_CLIENT_QUERY`;
        for my $line (@out) {
            if (my ($tizenId) = $line =~ /^\s+\d+.*\s+([A-Za-z0-9]+)$/) {
                my $command = $WRT_CLIENT_UNINSTALL;
                $command =~ s/\{\}/$tizenId/g;
                print "    uninstalling widget $tizenId [$command]\n";
                `$command > /dev/null 2>/dev/null`;
            }
        }
    }
    {
        print "Checking if widget list is empty\n";
        my @out = `$WRT_CLIENT_QUERY`;
        for my $line (@out) {
            if (my ($tizenId) = $line =~ /^\s+\d+.*\s+([A-Za-z0-9]+)$/) {
                print "WIDGET LIST NOT EMPTY!!!\n";
                die;
            }
        }
    }
    {
        print "Installing all test widgets\n";
        for my $pwidgetData (@$pwidgets) {
            my ($package, $name) = @$pwidgetData;
            my $command = $WRT_CLIENT_INSTALL;
            $command =~ s/\{\}/$WIDGET_PACKAGE_DIR\/$package/g;
            print "    installing $name ($package) [$command]\n";
            `$command > /dev/null 2>/dev/null`;
        }
    }
    {
        print "Checking if all widgets are installed, determining tizenIds\n";
        my %widgetMap = ();
        for my $pwidgetData (@$pwidgets) {
            my ($package, $name) = @$pwidgetData;
            $widgetMap{$name} = undef;
        }
        my @out = `$WRT_CLIENT_QUERY`;
        for my $line (@out) {
            if (my ($name, $trash, $tizenId) = $line =~ /^\s*\d+\s+(([^ ]| [^ ])+)\s+.*\s+([A-Za-z0-9]{10}\.[^\s]+)\s*$/) {
                print "    found $name (tizenId $tizenId)\n";
                $widgetMap{$name} = $tizenId;
            }
        }
        for my $name (keys %widgetMap) {
            unless (defined $widgetMap{$name}) {
                print "    MISSING $name\n";
                die;
            }
        }
        return \%widgetMap;
}
}


##### Test schemes

# PARAMS:
#   \@ list of widgets  (can be undef, then will use @WIDGETS)
#   \@ list of modes (modes can be as in runTest)
#   number of repetitions of one widget/mode pair
sub fullRandomTest {
    my $pwidgets = shift; $pwidgets = \@WIDGETS unless defined $pwidgets;
my $pmodes = shift;
my $repetitions = shift;

my $pwidgetMap = dropAndInstallAll( $pwidgets );

my @testList = ();

print "Preparing the test list\n";
for my $pwidget (@$pwidgets) {
    my ($package, $name) = @$pwidget;
    for my $mode (@$pmodes) {
        for (my $number = 0; $number < $repetitions; $number++) {
            push @testList, [$name, $mode];
        }
    }
}

@testList = List::Util::shuffle( @testList );

print "Clearing the output dir\n";
`rm -rf $OUTPUT_DIR`;

print "Running tests\n";
my %runNumbers = ();
my $globalTestNumber = 0;
my $totalTestCount = @testList;
for my $ptest (@testList) {
    # find test parameters
    my ($name, $mode) = @$ptest;
    my $widgetTizenId = $$pwidgetMap{$name};
    my $runNo = $runNumbers{"$mode|$widgetTizenId"};
    print "next\n";
    if (defined $runNo)
    { $runNo += 1; }
    else
    { $runNo = 0; }
    $runNumbers{"$mode|$widgetTizenId"} = $runNo;
    $globalTestNumber += 1;
    print "    Test $globalTestNumber of $totalTestCount: ".
    "$name ($widgetTizenId) run $runNo, $mode\n";

    # remove and create output dir
    my $prepName = $name;
    $prepName =~ s/ /_/g;
    my $dirName = sprintf "$OUTPUT_DIR/$prepName/$mode/run%03d", $runNo;
    `mkdir -p $dirName`;

    # create output files
    open my $outFile, ">", "$dirName/prof-raw-data" or die;
    open my $dumpOutFile, ">", "$dirName/dump-stdout" or die;

    # run test
    runTest( $widgetTizenId, $mode, $outFile, $dumpOutFile, "$dirName/dump-stderr" );

    # close output files
    close $outFile;
    close $dumpOutFile;
    print "Done\n";
}
}

##### MAIN #####

fullRandomTest( undef, ['cold', 'warm', 'preload'], 10 );
