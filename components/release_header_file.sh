#record sdk version
SDK_VERSION=0.2
echo "Telink BLE SDK_VERSION = $SDK_VERSION" > ./sdk_version.txt

dir_release=../../SIG_MESH_header_file
sdk_dir=$dir_release/ble_lt_mesh
tools_dir=$dir_release/reference

rm -rf $dir_release
mkdir $dir_release
mkdir $sdk_dir
mkdir $tools_dir
mkdir $tools_dir/tl_bulk

cp -rf ../reference/tl_bulk/lib_file  $tools_dir/tl_bulk

cp -rf proj proj_lib vendor $sdk_dir
cp * .project .cproject $sdk_dir
find $sdk_dir/proj_lib -name *.c |xargs rm -rf
rm -rf $sdk_dir/release_mesh.sh

cp lib_sig_mesh/libsig_mesh.a	$sdk_dir/proj_lib
cp lib_sig_mesh_LPN/libsig_mesh_LPN.a	$sdk_dir/proj_lib

rm -rf $sdk_dir/*.IAB
rm -rf $sdk_dir/*.IAD
rm -rf $sdk_dir/*.IMB
rm -rf $sdk_dir/*.IMD
rm -rf $sdk_dir/*.PFI
rm -rf $sdk_dir/*.PO
rm -rf $sdk_dir/*.PR
rm -rf $sdk_dir/*.PRI
rm -rf $sdk_dir/*.PS
rm -rf $sdk_dir/*.SearchResults
rm -rf $sdk_dir/*.WK3
cd $dir_release
find . $dir_release/ble_lt_mesh/ -name "*.c" -maxdepth 10 | xargs rm  
find . $dir_release/ble_lt_mesh/ -name "*.link" -maxdepth 10 | xargs rm 
find . $dir_release/ble_lt_mesh/ -name "*.txt" -maxdepth 10 | xargs rm 
find . $dir_release/ble_lt_mesh/ -name "*.S" -maxdepth 10 | xargs rm 
find . $dir_release/ble_lt_mesh/ -name *.py" -maxdepth 10 | xargs rm 
find . $dir_release/ble_lt_mesh/ -name "*.bak" -maxdepth 10 | xargs rm 
find . $dir_release/ble_lt_mesh/ -name "*.pl" -maxdepth 10 | xargs rm
find . $dir_release/ble_lt_mesh/ -name "*project" -maxdepth 10 | xargs rm
find . $dir_release/ble_lt_mesh/ -name "*.a" -maxdepth 10 | xargs rm
pause
::exit