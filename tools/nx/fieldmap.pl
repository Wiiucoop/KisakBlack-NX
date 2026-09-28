# Map a script field table's literal x86 offsets to members.
#   perl tools/nx/fieldmap.pl fields_gen.h gentity_s src/game_mp/g_spawn_mp.cpp
# fields_gen.h comes from the layoutgen loose dump (README-SWITCH, "Field
# tables"). Prints each entry, the member(s) at that x86 offset with their
# LP64 offset, and "same" / "DIFF".
my ($gen, $st, $src) = @ARGV;
my %x;
open G, $gen or die "$gen: $!";
while (<G>) { while (/X_${st}__(\w+) = (\d+), L_${st}__\w+ = (\d+)/g) { push @{$x{$2}}, "$1:$3"; } }
close G;
open S, $src or die "$src: $!";
while (<S>) {
    next unless /^\s*\{ "(\w+)", (\d+),/;
    my ($f, $o) = ($1, $2);
    my $c = $x{$o} ? join(" ", @{$x{$o}}) : "??";
    my $same = ($c =~ /:(\d+)/ && $1 == $o) ? "same" : "DIFF";
    print "$. $f x86=$o -> $c $same\n";
}
