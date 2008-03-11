IFS=",";
grep "AC_INIT" configure.ac > output;
read foo version<output;
IFS="[.]) ";
echo $version>output;
IFS=" ";
read maj min rel<output;
rm output;
awk -v MAJ=$maj -v MIN=$min -v REL=$rel '{gsub ("MAJOR",MAJ,$0);gsub ("MINOR",MIN,$0);gsub ("RELEASE",REL,$0);print $0}' < src/server/versions.h.in >destination;mv destination src/server/versions.h 
