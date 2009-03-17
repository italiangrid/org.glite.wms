awk -v MAJ="1" -v MIN="2" -v REL="3" '{gsub ("MAJOR",MAJ,$0);gsub ("MINOR",MIN,$0);gsub ("RELEASE",REL,$0);print $0}' < src/server/versions.h.in >destination;mv destination src/server/versions.h 
