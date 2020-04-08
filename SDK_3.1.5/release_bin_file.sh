#record sdk version
SDK_VERSION=0.2
echo "Telink BLE SDK_VERSION = $SDK_VERSION" > ./sdk_version.txt

dir_release=../release_bin_V2.6.1
rm -rf $dir_release
mkdir $dir_release
cp -rf ./8269_mesh_master_dongle/8269_mesh_master_dongle.bin  $dir_release/	
cp -rf ./8269_mesh/8269_mesh.bin  $dir_release/
cp -rf ./8269_mesh_gw/8269_mesh_gw.bin  $dir_release/
cp -rf ./8269_mesh_LPN/8269_mesh_LPN.bin  $dir_release/
cp -rf ./8269_mesh_switch/8269_mesh_switch.bin  $dir_release/

cp -rf ./8258_mesh/8258_mesh.bin  $dir_release/	
cp -rf ./8258_mesh_gw/8258_mesh_gw.bin  $dir_release/	
cp -rf ./8258_mesh_LPN/8258_mesh_LPN.bin  $dir_release/	
cp -rf ./8258_mesh_switch/8258_mesh_switch.bin  $dir_release/	
cp -rf ./8258_mesh_gw_node/8258_mesh_gw_node.bin  $dir_release/	
cp -rf ./8258_spirit_LPN/8258_spirit_LPN.bin  $dir_release/	
cp -rf ./8258_gw_node_homekit/8258_gw_node_homekit.bin  $dir_release/	
cp -rf ./8258_gw_node_homekit_nonMfi/8258_gw_node_homekit_nonMfi.bin  $dir_release/	
cp -rf ./8258_gw_node_homekit_softAuth/8258_gw_node_homekit_softAuth.bin  $dir_release/	

cp -rf ./8278_mesh/8278_mesh.bin  $dir_release/
cp -rf ./8278_mesh_gw/8278_mesh_gw.bin  $dir_release/
cp -rf ./8278_mesh_LPN/8278_mesh_LPN.bin  $dir_release/
cp -rf ./8278_mesh_switch/8278_mesh_switch.bin  $dir_release/
pause
::exit