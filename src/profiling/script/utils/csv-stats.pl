#!/usr/bin/perl -w
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

use warnings;
use diagnostics;
use strict;
use Getopt::Std;

my %options = ();
my $delimiter = ',';
getopts("crs:d:h", \%options);

if (defined $options{h})
{
    print "HELP\n";
    exit;
}

if (defined $options{d})
{
    $delimiter = $options{d};
}

my $skip_row = defined $options{r};
my $skip_col = defined $options{c};
my @stats_names;

if (defined $options{s})
{
    @stats_names = split /,/, $options{s};
}
else
{
    @stats_names = split /,/, "min,max,avg";#,std";
}

foreach(@stats_names)
{
    if (not ($_ eq "min" or $_ eq "max" or $_ eq "avg" or $_ eq "std"))
    {
        print "bad stat name: ", $_, "\n";
        print "Supported stats: min,max,avg,std\n";
        exit;
    }
}

my @FILES = @ARGV;
my $first_file;

sub ParseCSV {
    my($filename, $delimiter) = @_;


    open FILE, "<", $filename or die $!;
    my @file_rows = <FILE>;
    my @desc_fields;

    if ($skip_col)
    {
        my $description_row = shift @file_rows;
        chomp $description_row; #remove last \n
        @desc_fields = split $delimiter, $description_row;
    }

    my @description_col;
    my $rows = [];
    foreach (@file_rows)
    {
        chomp $_; #remove last \n
        my @fields = split $delimiter, $_;
        if ($skip_row)
        {
            push @description_col, shift @fields;
        }
        push @$rows, \@fields;
    }
    my $table = Table->new($rows);
    my $csv = CSV->new($table);
    if ($skip_col)
    {
        $csv->addColsDescription(\@desc_fields);
    }
    if ($skip_row)
    {
        $csv->addRowsDescription(\@description_col);
    }
    $csv;
}

my $stats = {};
foreach my $stat_name (@stats_names)
{
    my $stat_creator = createStatForName($stat_name);
    $stats->{$stat_name} = $stat_creator;
}

foreach (@FILES)
{
#    print "parsing ", $_, "\n";
    my $csv = ParseCSV($_, $delimiter);
    while (my ($key, $value) = each (%$stats))
    {
#        print "adding ", $key, " stats\n";
        $value->addResults($csv);
    }
}

while (my ($key, $value) = each (%$stats))
{
#    print "printing ", $key, " stats\n";
    $value->finish();
    $value->writeToFile(*STDOUT);
}

sub createStatForName {
    my ($name) = @_;
    if ($name eq "min") {
        MinStat->new();
    } elsif ($name eq "max") {
        MaxStat->new();
    } elsif ($name eq "avg") {
        AvgStat->new();
    }
}


package MinStat;
sub new {
    my $class = shift;
    my $self = {};
    return bless $self, $class;
}

sub addResults {
    my ($self, $csv) = @_;
    if (defined $self->{m_base_csv}) {
        my $rowNr = 0;
        foreach my $row (@{$csv->getTable()->getData()})
        {
            my $colNr = 0;
            foreach (@$row)
            {
                my $oldvalue = $self->{m_table}->getData()->[$rowNr][$colNr];
                my $newvalue = $_;
                if ($newvalue < $oldvalue) {
                    $self->{m_table}->getData()->[$rowNr][$colNr] = $newvalue;
                }
                $colNr ++;
            }
            $rowNr ++;
        }
    } else {
        my $newcsv = CSV->new(Table->new([]));
        $newcsv->addColsDescription($csv->getColsDescription());
        $newcsv->addRowsDescription($csv->getRowsDescription());
        $self->{m_base_csv} = $newcsv;
        my $table = [];
        foreach(@{$csv->getTable()->getData()})
        {
            my $new_row = [];
            @$new_row = @$_; #array copy
            push @$table, $new_row;
        }
        $self->{m_table} = Table->new($table);
    }
}

sub writeToFile {
    my ($self, $file) = @_;
    $self->{m_base_csv}->setTable($self->{m_table});
    $self->{m_base_csv}->writeToFile($file);
}

sub finish {
    my ($self) = @_;
    $self->{m_base_csv}->setTable($self->{m_table});
}

package MaxStat;
sub new {
    my $class = shift;
    my $self = {};
    return bless $self, $class;
}

sub addResults {
    my ($self, $csv) = @_;
    if (defined $self->{m_base_csv}) {
        my $rowNr = 0;
        foreach my $row (@{$csv->getTable()->getData()})
        {
            my $colNr = 0;
            foreach (@$row)
            {
                my $oldvalue = $self->{m_table}->getData()->[$rowNr][$colNr];
                my $newvalue = $_;
                if ($newvalue > $oldvalue) {
                    $self->{m_table}->getData()->[$rowNr][$colNr] = $newvalue;
                }
                $colNr ++;
            }
            $rowNr ++;
        }
    } else {
        my $newcsv = CSV->new(Table->new([]));
        $newcsv->addColsDescription($csv->getColsDescription());
        $newcsv->addRowsDescription($csv->getRowsDescription());
        $self->{m_base_csv} = $newcsv;
        my $table = [];
        foreach(@{$csv->getTable()->getData()})
        {
            my $new_row = [];
            @$new_row = @$_; #array copy
            push @$table, $new_row;
        }
        $self->{m_table} = Table->new($table);
    }
}

sub finish {
    my ($self) = @_;
    $self->{m_base_csv}->setTable($self->{m_table});
}

sub writeToFile {
    my ($self, $file) = @_;
    $self->{m_base_csv}->writeToFile($file);
}

package AvgStat;
sub new {
    my $class = shift;
    my $self = {};
    return bless $self, $class;
}

use Scalar::Util 'looks_like_number';
sub addResults {
    my ($self, $csv) = @_;
    if (defined $self->{m_base_csv}) {
        my $rowNr = 0;
        foreach my $row (@{$csv->getTable()->getData()})
        {
            my $colNr = 0;
            foreach my $coldata (@$row)
            {
#                if (not $coldata gt 0) {
#                    die ("LINE: " . $rowNr . "  COL: " . $colNr . "   DATA: " . $coldata);
#                }
                $self->{m_table_with_mean}->getData()->[$rowNr][$colNr] += $coldata;
                $self->{m_table_with_count}->getData()->[$rowNr][$colNr] ++;
                $colNr ++;
            }
            $rowNr ++;
        }
    } else {
        my $newcsv = CSV->new(Table->new([]));
        $newcsv->addColsDescription($csv->getColsDescription());
        $newcsv->addRowsDescription($csv->getRowsDescription());
        $self->{m_base_csv} = $newcsv;
        my $table = [];
        my $table_2 = [];
        foreach(@{$csv->getTable()->getData()})
        {
            my $new_row = [];
            @$new_row = @$_; #array copy
            push @$table, $new_row;
            my $new_row_2 = [];
            my $len = @$_;
            @$new_row_2 = ((1) x $len);
            push @$table_2, $new_row_2;
        }
        $self->{m_table_with_mean} = Table->new($table);
        $self->{m_table_with_count} = Table->new($table_2);
        foreach my $row (@{$self->{m_table_with_count}->getData()})
        {
            foreach my $field (@$row)
            {
                $field = 1;
            }
        }
    }
}

sub finish {
    my ($self) = @_;
    my $rowNr = 0;
    foreach my $row (@{$self->{m_table_with_count}->getData()})
    {
        my $colNr = 0;
        foreach (@$row)
        {
            my $tmp = $self->{m_table_with_mean}->getData()->[$rowNr][$colNr];
#            print "DIV: ", $tmp, " : ", $_, "\n";
            $tmp = $tmp / $_;
            $tmp = sprintf ("%.0f", $tmp);
            $self->{m_table_with_mean}->getData()->[$rowNr][$colNr] = $tmp;
            $colNr ++;
        }
        $rowNr ++;
    }
    $self->{m_base_csv}->setTable($self->{m_table_with_mean});
}

sub writeToFile {
    my ($self, $file) = @_;
    $self->{m_base_csv}->writeToFile($file);
}


package Table;
sub new {
    my $class = shift;
    my $self = {
        m_data => shift,
    };
    return bless $self, $class;
}

sub setField {
    my ($self, $row, $col, $value) = @_;
    $self->{m_data}[$row][$col] = $value;
}

sub getField {
    my ($self, $row, $col) = @_;
    $self->{m_data}[$row][$col];
}

sub addField {
    my ($self, $row, $col, $value) = @_;
    $self->{m_data}[$row][$col] += $value;
}

sub incField {
    my ($self, $row, $col) = @_;
    $self->{m_data}[$row][$col] ++;
}

sub getData {
    my ($self) = @_;
    $self->{m_data};
}

package CSV;
sub new {
    my $class = shift;
    my $self = { m_data => shift, };
    return bless $self, $class;
}

sub addColsDescription {
    my ($self, $desc) = @_;
    $self->{m_cols_desc} = $desc;
}

sub addRowsDescription {
    my ($self, $desc) = @_;
    $self->{m_rows_desc} = $desc;
}

sub getColsDescription {
    my ($self) = @_;
    $self->{m_cols_desc};
}

sub getRowsDescription {
    my ($self) = @_;
    $self->{m_rows_desc};
}

sub getNrRows {
    my ($self) = @_;
    my $ret = @{$self->{m_data}->getData()};
}

sub getNrCols {
    my ($self) = @_;
    my $ret = @{@{$self->{m_data}->getData()}[0]};
}

sub getTable {
    my ($self) = @_;
    $self->{m_data};
}

sub setTable {
    my ($self, $table) = @_;
    $self->{m_data} = $table;
}

sub writeToFile {
    my ($self, $file) = @_;
    if (defined $self->{m_cols_desc}) {
        print $file join(',', @{$self->{m_cols_desc}}), "\n";
    }
    my $rowNr = 0;
    foreach(@{$self->{m_data}->getData()})
    {
        my @row = @$_;
        if (defined $self->{m_rows_desc}) {
            print $file $self->{m_rows_desc}->[$rowNr], ',';
        }
        print $file join(',', @row), "\n";
        $rowNr ++;
    }
}
