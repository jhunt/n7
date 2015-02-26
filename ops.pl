#!/usr/bin/perl

use strict;
use warnings;

my %ops = ();
while (<>) {
	$ops{$_}++ for m/\((m0:[^\s\)]+)/g;
}

for (sort keys %ops) {
	print "$_ $ops{$_}\n";
}
