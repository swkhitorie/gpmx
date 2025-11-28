#!/usr/bin/env python3

import re
import sys
import os

def parse_armclang_map(map_file_path):
    regions = []

    try:
        with open(map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        print("analyzing arm map...")

        # 解析Load Region信息
        load_region_pattern = re.compile(
            r'Load Region\s+(\w+)\s+\([^)]*Base:\s+(0x[0-9a-fA-F]+)[^)]*Size:\s+(0x[0-9a-fA-F]+)[^)]*Max:\s+(0x[0-9a-fA-F]+)',
            re.IGNORECASE
        )

        # 解析Execution Region信息  
        exec_region_pattern = re.compile(
            r'Execution Region\s+(\w+)\s+\([^)]*Base:\s+(0x[0-9a-fA-F]+)[^)]*Size:\s+(0x[0-9a-fA-F]+)[^)]*Max:\s+(0x[0-9a-fA-F]+)',
            re.IGNORECASE
        )

        # 解析压缩大小（如果有）
        compressed_pattern = re.compile(r'COMPRESSED\[(0x[0-9a-fA-F]+)\]')

        # 查找所有Load Region
        load_regions = load_region_pattern.findall(content)
        for region_name, base_hex, used_hex, max_hex in load_regions:
            used_bytes = int(used_hex, 16)
            max_bytes = int(max_hex, 16)
            
            # 检查是否有压缩大小
            region_section = re.search(rf"Load Region {region_name}.*?Execution Region", content, re.DOTALL)
            if region_section:
                compressed_match = compressed_pattern.search(region_section.group(0))
                if compressed_match:
                    # 使用压缩后的大小
                    compressed_size = int(compressed_match.group(1), 16)
                    used_bytes = compressed_size
                    print(f"Using compressed size for {region_name}: {format_size(used_bytes)}")
            
            percentage = (used_bytes / max_bytes) * 100 if max_bytes > 0 else 0
            regions.append(('Load:' + region_name, used_bytes, max_bytes, percentage))

        # 查找所有Execution Region
        exec_regions = exec_region_pattern.findall(content)
        for region_name, base_hex, used_hex, max_hex in exec_regions:
            used_bytes = int(used_hex, 16)
            max_bytes = int(max_hex, 16)
            percentage = (used_bytes / max_bytes) * 100 if max_bytes > 0 else 0
            regions.append(('Exec:' + region_name, used_bytes, max_bytes, percentage))

        # 分析详细的段信息以验证统计
        analyze_section_details(content)

        return regions

    except Exception as e:
        print(f"error occur in file analyze: {e}")
        import traceback
        traceback.print_exc()
        return None

def analyze_section_details(content):
    """分析详细的段信息 - 基于实际map文件结构修正"""
    print("\nanalyzing section details...")
    
    # 从Image component sizes部分提取准确的数据
    component_sizes_pattern = re.compile(
        r'Code\s+\(inc\. data\)\s+RO Data\s+RW Data\s+ZI Data\s+Debug.*?Grand Totals\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)',
        re.DOTALL
    )
    
    # 查找Grand Totals
    grand_totals_match = component_sizes_pattern.search(content)
    if grand_totals_match:
        total_code = int(grand_totals_match.group(1))
        total_ro_data = int(grand_totals_match.group(3))
        total_rw_data = int(grand_totals_match.group(4))  # 运行时RAM中的RW Data
        total_zi_data = int(grand_totals_match.group(5))
        
        print(f"From Grand Totals:")
        print(f"  Code:     {format_size(total_code)}")
        print(f"  RO Data:  {format_size(total_ro_data)}")
        print(f"  RW Data:  {format_size(total_rw_data)} (runtime in RAM)")
        print(f"  ZI Data:  {format_size(total_zi_data)}")
    
    # 从ELF Image Totals中提取ROM中的RW Data初始值
    elf_totals_pattern = re.compile(
        r'ELF Image Totals.*?(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)',
        re.DOTALL
    )
    
    elf_totals_match = elf_totals_pattern.search(content)
    if elf_totals_match:
        rom_rw_data = int(elf_totals_match.group(4))  # ROM中的RW Data初始值
        print(f"  RW Data in ROM: {format_size(rom_rw_data)} (initial values in Flash)")
    
    # 从ROM Totals中验证
    rom_totals_pattern = re.compile(
        r'ROM Totals.*?(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)',
        re.DOTALL
    )
    
    rom_totals_match = rom_totals_pattern.search(content)
    if rom_totals_match:
        rom_code = int(rom_totals_match.group(1))
        rom_ro_data = int(rom_totals_match.group(3))
        rom_rw_data = int(rom_totals_match.group(4))
        print(f"From ROM Totals:")
        print(f"  Code:     {format_size(rom_code)}")
        print(f"  RO Data:  {format_size(rom_ro_data)}")
        print(f"  RW Data:  {format_size(rom_rw_data)} (initial values in Flash)")
    
    # 计算正确的ROM大小
    if grand_totals_match and elf_totals_match:
        total_code = int(grand_totals_match.group(1))
        total_ro_data = int(grand_totals_match.group(3))
        rom_rw_data = int(elf_totals_match.group(4))
        
        calculated_rom_size = total_code + total_ro_data + rom_rw_data
        print(f"\nCalculated ROM Size: {format_size(calculated_rom_size)}")
        print(f"  = Code({format_size(total_code)}) + RO Data({format_size(total_ro_data)}) + RW Data initial values({format_size(rom_rw_data)})")
        
        # 显示RAM使用情况
        total_ram_size = total_rw_data + total_zi_data
        print(f"Total RAM Size: {format_size(total_ram_size)}")
        print(f"  = RW Data({format_size(total_rw_data)}) + ZI Data({format_size(total_zi_data)})")
    
    # 解析Execution Region的详细段信息
    print(f"\nDetailed Execution Region Analysis:")
    
    # 查找所有Execution Region部分
    exec_sections = re.finditer(r'Execution Region (\w+) \(.*?\)\s*\n\s*Base Addr\s+Size\s+Type\s+Attr\s+Idx\s+E Section Name\s+Object\s*\n(.*?)(?=\n\s*\n|\Z)', content, re.DOTALL)
    
    for match in exec_sections:
        region_name = match.group(1)
        section_lines = match.group(2)
        
        region_code = 0
        region_ro_data = 0
        region_data_rw = 0
        region_zero_rw = 0
        
        # 解析区域内的各个段
        lines = section_lines.strip().split('\n')
        for line in lines:
            line = line.strip()
            if line and not line.startswith('Base Addr'):
                # 使用更灵活的正则表达式来匹配段行
                pattern = r'^(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+(\w+)\s+(\w+)\s+(\d+)\s+(\S.*)$'
                match_line = re.match(pattern, line)
                
                if match_line:
                    size_hex = match_line.group(2)
                    mem_type = match_line.group(3)
                    attr = match_line.group(4)
                    
                    try:
                        size = int(size_hex, 16)
                        
                        if attr == 'RO':
                            if 'Code' in mem_type or '.ARM.Collect' in line or '.text' in line or 'i.' in line:
                                region_code += size
                            else:
                                region_ro_data += size
                        elif attr == 'RW':
                            if 'Data' in mem_type:
                                # Data RW - 有初始值的变量
                                region_data_rw += size
                            elif 'Zero' in mem_type:
                                # Zero RW - 零初始化变量
                                region_zero_rw += size
                    except ValueError:
                        continue
        
        if region_code + region_ro_data + region_data_rw + region_zero_rw > 0:
            print(f"{region_name}: Code={format_size(region_code)}, RO Data={format_size(region_ro_data)}, Data RW={format_size(region_data_rw)}, Zero RW={format_size(region_zero_rw)}")

def format_size(size_bytes):
    """格式化大小为可读的格式"""
    if size_bytes >= 1024 * 1024:
        return f"{size_bytes / (1024 * 1024):.2f} MB"
    elif size_bytes >= 1024:
        return f"{size_bytes / 1024:.2f} KB"
    else:
        return f"{size_bytes} B"

def display_memory_usage(regions):
    """显示内存使用情况表格"""
    if not regions:
        print("can not find memory info")
        return

    print("\n" + "=" * 70)
    print("Memory Region         Used Size    Region Size   %age Used")
    print("=" * 70)

    for region_name, used_bytes, max_bytes, percentage in regions:
        used_str = format_size(used_bytes)
        max_str = format_size(max_bytes)

        print(f"{region_name:<20} {used_str:>12} {max_str:>12} {percentage:>8.2f}%")

    print("=" * 70)

def main():
    """主函数"""
    if len(sys.argv) != 2:
        print("usage:   python armclang_map_parser.py <map_file>")
        print("example: python armclang_map_parser.py firmware.map")
        sys.exit(1)

    map_file_path = sys.argv[1]

    if not os.path.exists(map_file_path):
        print(f"error: file '{map_file_path}' not exist")
        sys.exit(1)

    print("ARMCLANG Map File Analyzer - Based on Actual Map Structure")

    # 解析map文件
    regions = parse_armclang_map(map_file_path)

    if regions is None:
        print("analyze failed, please check map file format")
        sys.exit(1)

    # 显示结果
    display_memory_usage(regions)

if __name__ == '__main__':
    main()