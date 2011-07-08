cat ../hbc/d2x-cios-installer/meta.xml | tr -d '\n\r\t' | sed -r "s/ [ ]+//" | sed -r "s/<release_date>[0-9]+<\/release_date>//" | sed -r "s/<version>.*<\/version>//" | sed -r "s/<\/app>$//" > meta.xml.tmp
sh -c "echo -n '<version>'" >> meta.xml.tmp
grep "^#define VERSION " d2x-cios-installer.h | cut --delimiter=" " --fields=3 | tr -d '"\n\r' >> meta.xml.tmp
sh -c "echo -n '</version><release_date>'" >> meta.xml.tmp
sh -c "ls -l --time-style=+%s d2x-cios-installer.elf" | cut --delimiter=" " --fields=6 | tr -d '\n\r' >> meta.xml.tmp
sh -c "echo -n '</release_date></app>'" >> meta.xml.tmp
mv -f d2x-cios-installer.dol ../hbc/d2x-cios-installer/boot.dol
mv -f meta.xml.tmp ../hbc/d2x-cios-installer/meta.xml
wiiload d2x-cios-installer.elf
rm d2x-cios-installer.elf