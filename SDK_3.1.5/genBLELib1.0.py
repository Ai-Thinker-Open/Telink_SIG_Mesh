import os, re, shutil
import subprocess, shlex


PROJ_FOLDER = 'hid_sample'

doxygenFiles = [
    'ev_buffer.h',
    'ev_task.h',    
    'mac_api.h',
    'mac_const.h',
    'nwk_api.h',
    'nwk_const.h',
    'nwk_pib.h',
    'ev_queue.h',
    'profile.h',
    'profile_attr.h',
]

lib_ble = [
	'ble/lib',
	'att',
	'gatt',
	'gatt_client',
	'gatt_server',
	'gatt_uuid',

	'll',
	'll_advertising',
	'll_ib',
	'll_master',
	'll_slave',
	'll_standby',
	'll_whitelist',
	'phy',
	'gap',
	'gap_attServer',
	'gap_peripheral',
	'gap_link',
	'gap_utils',

	'hci',
	'hci_cmd',
	'hci_data',
	'hci_event',
	'hci_host',

	'l2cap',
	'l2cap_util',
	
	'smp',
	'smp_cmd',
	'smp_key',
	'smp_table',
	'smp_utils',
	
]

lib_os = [
	'proj_lib/lib',
	'ev',
	'timer',
	'ev_event',
	'ev_queue',
	'ev_task',
]

lib_rf = [
	'rf/lib',
	'rf',
	'rf_compatible',
]


lib_proj = [
	'proj_lib/lib',
	'pm_8266',
	'rf_drv_8266',
]

inc_proj = [
	'proj_lib/lib',
	'pm',
	'rf_drv',
	'rf_drv_8266',
	'pm_8266',
]


def filePath(dirName, fileFormat):
    resultDir = []
    for root, dirs, files in os.walk(dirName):
        for fileItem in files:
            if fileItem in fileFormat:
                resultDir.append(os.path.join(root, fileItem))
    return resultDir



def runCommand(cmd):
    args = shlex.split(cmd)
    try:
        p = subprocess.Popen(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        (stdout, stderr) = p.communicate()
        status = p.returncode
        if status!=0:
            print stderr
            return None
        return stdout.strip()
    except OSError, e:
        print "Execution failed:", e
        return None

def genLibAndRmSrc(lib_dict, src_dir, lib_name, dst_dir, inc_dir):
	# gen mac lib
	macLib = [i + '.o' for i in lib_dict[1:]]
	fileList = filePath(src_dir, macLib)	
	
	# copy the files
	if os.path.exists('tmp'):
		shutil.rmtree('tmp')
	os.mkdir('tmp')
	
	print lib_dict[1:]
	print fileList
	for item in fileList:
		item = item.replace('\\', '/')
		runCommand('cp -rf %s tmp' % item)
	os.chdir('tmp')
    
	strLib = ''
	for item in os.listdir('.'):
		strLib = strLib + item + ' ' 
    
	runCommand('tc32-elf-ar -r "%s" %s' % (lib_name, strLib))
	winPath = '..'
	for item in dst_dir.split('/'):
		winPath = os.path.join(winPath, item)
	for item in lib_dict[0].split('/'):
		winPath = os.path.join(winPath, item)
	if not os.path.exists(winPath):
		os.makedirs(winPath)
	runCommand('cp -rf %s %s' % (lib_name, '../' + dst_dir + '/' + lib_dict[0]))
	os.chdir('..')
	
	if not inc_dir:
		# remove .c file 
		macLib = [i + '.c' for i in lib_dict[1:]]
		fileList = filePath(SDKPath, macLib)
		for item in fileList:
			item = item.replace('\\', '/')
			runCommand('rm -rf %s ' % item)
	else:
		# remove folder
		winPath = '.'
		for item in dst_dir.split('/'):
			winPath = os.path.join(winPath, item)
		for item in inc_dir[0].split('/'):
			winPath = os.path.join(winPath, item)

		macLib = [i + '.h' for i in inc_dir[1:]]

		for root, dirs, files in os.walk(winPath):
			for fileItem in files:
				if fileItem not in macLib:
					item = os.path.join(root, fileItem)
					item = item.replace('\\', '/')
					runCommand('rm -rf %s ' % item)
			for item in dirs:
				if len(os.listdir(os.path.join(root, item))) == 0:
					dirItem = os.path.join(root, item)
					dirItem = dirItem.replace('\\', '/')
					runCommand('rm -rf %s ' % dirItem)
			
if __name__ == '__main__': 

	SDKPath = '../SDK'
	SDKFolderList = ['proj', 'ble', 'vendor']
	SDKFileList = ['\.cproject', '\.project', 'boot.link', 'getver.sh', 'div_mod.S']
	
	winPath = '.'
	for item in SDKPath.split('/'):
		winPath = os.path.join(winPath, item)
	if os.path.exists(winPath):
		shutil.rmtree(winPath)
	os.makedirs(winPath)
	
	os.makedirs(os.path.join(winPath, 'proj_lib'))
	
	macLib = [i + '.h' for i in inc_proj[1:]]
	
	fileList = filePath('.', macLib)	

	destDir = os.path.join(winPath, 'proj_lib')
	destDir = destDir.replace('\\', '/')
	for item in fileList:
		item = item.replace('\\', '/')
		runCommand('cp -rf %s %s' % (item, destDir))

	for item in SDKFolderList:
		runCommand('cp -rf %s %s' % (item, SDKPath))
		
	for item in SDKFileList:
		runCommand('cp -rf %s %s' % (item, SDKPath))
		
		

	genLibAndRmSrc(lib_ble, PROJ_FOLDER, 'libble.a', SDKPath, None)
	
	genLibAndRmSrc(lib_proj, PROJ_FOLDER, 'libproj.a', SDKPath, None)
	genLibAndRmSrc(lib_os, PROJ_FOLDER, 'libos.a', SDKPath, None)
	#genLibAndRmSrc(lib_rf, 'gap_test', 'librf.a', SDKPath, None)
	
	

	


