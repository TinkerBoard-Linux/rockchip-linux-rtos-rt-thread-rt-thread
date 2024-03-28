#!/usr/bin/env python3

import sys
import os
import subprocess

def get_vectors_section_address(elf_file):
    try:
        # 使用readelf命令获取ELF文件的section信息
        output = subprocess.check_output(['readelf', '-S', elf_file], universal_newlines=True)

        # 解析readelf命令的输出
        for line in output.split('\n'):
            if '.vectors' in line:
                # 提取.vectors section的起始地址
                address = line.split()[4]
                return int(address, 16)

        return None
    except FileNotFoundError:
        print(f"{elf_file} not found")
        return None
    except subprocess.CalledProcessError as e:
        print(f"readelf error: {e}")
        return None

def find_part_offset(file_path, search_string):
    with open(file_path, 'r') as file:
        lines = file.readlines()

        for i in range(len(lines)):
            if search_string in lines[i]:
                offset_line = lines[i-3]
                if offset_line.startswith('PartOffset='):
                    offset_value = offset_line.split('=')[1].strip()
                    return offset_value

    return None

def check_xip_addr(elf_file, setting_file):
    # 获取.vectors section的起始地址
    vectors_address = get_vectors_section_address(elf_file)

    if vectors_address < 0x11000000 or vectors_address >= 0x20000000 :
        print("vectors address is not in the XIP range, check ignore")
        return -1

    if vectors_address is not None:
        print(f".vectors section addr of {elf_file}: 0x{vectors_address:x}")
    else:
        print("can not found .vectors section")
        return -1

    # 要搜索的关键字
    if 'rtt0.elf' in elf_file :
        search_string = 'File=../../rtt0.bin'
    else :
        search_string = 'File=../../rtt1.bin'

    # 查找并打印PartOffset的值
    part_offset = find_part_offset(setting_file, search_string)
    if part_offset is None :
        print(f"not found PartOffset for {elf_file}")
        return -1

    part_offset = 512 * int(part_offset, 16)
    part_offset += 0x11000000
    print(f"PartOffset: 0x{part_offset:x}")

    if part_offset == vectors_address :
        print("part_offset=vectors_address")
        return 0
    else:
        print("part_offset!=vectors_address")
        return -1

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: check_xip_addr.py <setting.ini>")
        sys.exit(1)

    setting_file = sys.argv[1]
    if os.path.exists('./rtt0.elf') :
        ret = check_xip_addr('./rtt0.elf', setting_file)
        if ret != 0 :
            print("check rtt0.elf xip addr failed, please check setting.ini")
            sys.exit(1)
    if os.path.exists('./rtt1.elf') :
        ret = check_xip_addr('./rtt1.elf', setting_file)
        if ret != 0 :
            print("check rtt1.elf xip addr failed, please check setting.ini")
            sys.exit(1)
    print("check xip addr success")
    sys.exit(0)
