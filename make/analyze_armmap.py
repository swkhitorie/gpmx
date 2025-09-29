#!/usr/bin/env python3

"""
ARMCLANG Map文件解析器 - 提取内存区域使用情况并以百分比显示
专门针对armclang生成的map文件格式
"""

import re
import sys
import os

def parse_armclang_map(map_file_path):
    """
    解析armclang生成的map文件, 提取内存区域使用情况
    
    Args:
        map_file_path: map文件路径
    """
    
    # 存储区域信息
    regions = []
    
    # 更精确的正则表达式模式
    # 匹配: Load Region LR_IROM1 (Base: 0x08000000, Size: 0x0000f618, Max: 0x00200000, ...)
    load_region_pattern = re.compile(
        r'Load Region\s+(\w+)\s+\([^)]*Size:\s+(0x[0-9a-fA-F]+)[^)]*Max:\s+(0x[0-9a-fA-F]+)',
        re.IGNORECASE
    )
    
    # 匹配: Execution Region ER_IROM1 (Exec base: 0x08000000, Load base: 0x08000000, Size: 0x0000eae4, Max: 0x00200000, ...)
    exec_region_pattern = re.compile(
        r'Execution Region\s+(\w+)\s+\([^)]*Size:\s+(0x[0-9a-fA-F]+)[^)]*Max:\s+(0x[0-9a-fA-F]+)',
        re.IGNORECASE
    )
    
    try:
        with open(map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        print("analyzing armclang map...")
        
        # 查找所有Load Region
        load_regions = load_region_pattern.findall(content)
        for region_name, used_hex, max_hex in load_regions:
            used_bytes = int(used_hex, 16)
            max_bytes = int(max_hex, 16)
            percentage = (used_bytes / max_bytes) * 100 if max_bytes > 0 else 0
            regions.append(('Load:' + region_name, used_bytes, max_bytes, percentage))
        
        # 查找所有Execution Region  
        exec_regions = exec_region_pattern.findall(content)
        for region_name, used_hex, max_hex in exec_regions:
            used_bytes = int(used_hex, 16)
            max_bytes = int(max_hex, 16)
            percentage = (used_bytes / max_bytes) * 100 if max_bytes > 0 else 0
            regions.append(('Exec:' + region_name, used_bytes, max_bytes, percentage))
        
        return regions
        
    except Exception as e:
        print(f"error occur in file analyze: {e}")
        return None

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
    
    print("\n")
    
    # 解析map文件
    regions = parse_armclang_map(map_file_path)
    
    if regions is None:
        print("analyze failed, please check map file format")
        sys.exit(1)
    
    # 显示结果
    display_memory_usage(regions)

if __name__ == '__main__':
    main()