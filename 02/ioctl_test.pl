#!/usr/bin/perl -w
use strict;

if ($#ARGV + 1 < 4) {
	print "Usage: $0 /dev/something cmd arg arg_as_pointer\n\n";
	print "  arg_as_pointer ... 0 to pass arg to ioctl (set value)\n";
	print "                     1 to pass &arg to ioctl (get value)\n";
	print "  I.e. it performs:\n";
	print "    ioctl(/dev/something, cmd, arg)  if arg_as_pointer = 0\n";
	print "    ioctl(/dev/something, cmd, &arg) otherwise\n";
	exit 1;
}

my $dev = shift @ARGV;
my $cmd = shift @ARGV;
my $arg = shift @ARGV;
my $ptr = shift @ARGV;

$cmd = oct($cmd) if $cmd =~ /^0/;

open(DEV, "+<", $dev) || die "open failed";

die "$dev not a character device\n" unless -c DEV;

printf "ioctl(%x, %s%x)\n", $cmd, $ptr ? "&" : "", $arg;
my $bin_arg = $ptr ? pack "L", $arg : int $arg;
ioctl(DEV, $cmd, $bin_arg) || die "ioctl failed";

if ($ptr) {
	$arg = unpack "L", $bin_arg;
	printf "arg is now %x\n", $arg;
}

close DEV;
