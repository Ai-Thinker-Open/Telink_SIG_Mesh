#!/bin/bash 
echo "*****************************************************"
echo "this is post build!! current configure is :$1"
tc32-elf-objcopy -v -O binary $2.elf  $1.bin

#par[0]:EXE path;
#par[1]:bin path; 
#par[2]: 1 means FW_CHECK_AGTHM1, 2 means FW_CHECK_AGTHM2;  other values is invalid; In encryption mode, AGTHM2 is automatically used.
#par[3]: signature private key with 32 bytes. 0 means no signature.  (Optional)
#par[4]: "2.elf" means that if there is 'key_encode_bin' in firmware, encryption mode will be enable. 0 means no encryption mode. (Optional)

../tl_auth_check_fw.exe  $1.bin  1  0  $2.elf
exec_result=$?
#echo result=${exec_result}
if [ "${exec_result}" == "0" ]
then
echo  "exec successful"
else
echo  "###########################  Error: Run tl_auth_check_fw.exe failed  ###########################"
echo  "###########################  Error: Run tl_auth_check_fw.exe failed  ###########################"
echo  "###########################  Error: Run tl_auth_check_fw.exe failed  ###########################"
echo  "Error: exec failed, exit code=${exec_result}"
rm -rf $1.bin
exit ${exec_result}
fi

echo "**************** end of post build ******************"
