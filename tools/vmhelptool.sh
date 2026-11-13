
script_dir="$(cd "$(dirname "$0")" && pwd)"
top_dir=${script_dir}/../
compiler_os=$(uname)
makefile_os=${compiler_os}
target_app_config=$1

proj_name=$(grep '^PROJ_NAME\s*:=' "$target_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_tc=$(grep '^PROJ_TC\s*:=' "$target_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_bin=${top_dir}/bin/${proj_name}_${proj_tc}_0.bin
proj_hex=${top_dir}/bin/${proj_name}_${proj_tc}_0.hex
proj_elf=${top_dir}/bin/${proj_name}_${proj_tc}_0.elf

if [ ! -f ${proj_bin} ] || [ ! -f ${proj_hex} ] || [ ! -f ${proj_elf} ];then
    echo "proj file not exist"
    exit 1
fi

isvm=$(systemd-detect-virt)
if [ ${isvm} == "vmware" ]
then
    shared_path=/mnt/hgfs/
elif [ ${isvm} == "virualbox" ]
then
    shared_path=/mnt/media/
else
    echo "unsupported vm platform ${isvm}"
    exit 1
fi

sub_shared_path=("$shared_path"/*/)

# default 
target_shared_path=${sub_shared_path[0]}

if [ ! -d ${shared_path} ] || [ ! -d ${target_shared_path} ];then
    echo "vm shared path not existed"
    exit 1
fi

echo "Copy to shared path ${target_shared_path}"
cp ${proj_bin} ${target_shared_path}
cp ${proj_hex} ${target_shared_path}
cp ${proj_elf} ${target_shared_path}

exit 0
