get_version()
{
    local __base=${1:-1}
    echo $((`git rev-list --branches|wc -l` + $__base))
}

VER_FILE=version.in
#echo -n " .equ BUILD_VERSION, " > $VER_FILE
#please set "BUILD_VERSION" in version.h to modify firmware version.//   echo 0x362e3256 >> $VER_FILE

#note: 0x362e3256 means V2.6