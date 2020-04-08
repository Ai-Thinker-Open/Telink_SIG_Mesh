export TIME_NOW=`date +%Y-%m-%d-%H:%M:%S`
cd ..
printf "#define   BUILD_VERSION  \t" > version.h
printf "\x22Revision: " >> version.h
svnversion -n >> version.h
printf "\x22\r\n" >> version.h
printf "#define   BUILD_TIME  \t\t" >> version.h
printf "\x22" >> version.h
printf $TIME_NOW >> version.h
printf "\x22\r\n" >> version.h
