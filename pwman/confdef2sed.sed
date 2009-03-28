s/[\\&,]/\\&/g
s,[\\$`],\\&,g
t clear
: clear
s,^[	 ]*#[	 ]*define[	 ][	 ]*\([^	 (][^	 (]*\)\(([^)]*)\)[	 ]*\(.*\)$,${ac_dA}\1${ac_dB}\1\2${ac_dC}\3${ac_dD},gp
t end
s,^[	 ]*#[	 ]*define[	 ][	 ]*\([^	 ][^	 ]*\)[	 ]*\(.*\)$,${ac_dA}\1${ac_dB}\1${ac_dC}\2${ac_dD},gp
: end
