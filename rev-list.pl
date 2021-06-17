#! /usr/bin/perl -w

my $ln = <>;
$ln =~ s/\s*//g;
$ln = $ln * 2 + 2;

while (<>) {
	chop;
	if ($ln & 1) {
		print $_, "\r\n";
	}
	else {
		s/commit\s*//;
		printf "%04x $_ ", $ln>>1;
	}
	$ln = $ln - 1;
}
