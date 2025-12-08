#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json

TYPE_SIZES = {
    "uint8_t": 1,
    "uint16_t": 2,
    "uint32_t": 4,
}

def to_func_name(name):
    """SC_MOVE_START -> SendMoveStart"""
    parts = name.replace('SC_', '').split('_')
    return 'Send' + ''.join(p.capitalize() for p in parts)

def generate_proxy(packets, header_code):
    # Header
    h_lines = [
        '// Auto-generated Proxy.h',
        '#pragma once',
        '#include <cstdint>',
        '',
    ]
    
    # Cpp
    cpp_lines = [
        '// Auto-generated Proxy.cpp',
        '#include "stdafx.h"',
        '#include "Proxy.h"',
        '#include "Packet.h"',
        '#include "Protocol.h"', 
        '#include "Express.h"',
        '',
    ]
    
    for pkt in packets:
        func_name = to_func_name(pkt['name'])
        fields = pkt['fields']
        send_method = pkt['send']
        
        params = []
        send_call = ''
        session_param_name = '' # toSession ¶Ç´Â exceptSession

        if send_method == 'Unicast':
            session_param_name = 'toSession'
            params.append(f'int {session_param_name}')
            send_call = f'Express::GetInstance().Unicast({session_param_name}, &pkt);'
        elif send_method == 'BroadcastExcept':
            session_param_name = 'exceptSession'
            params.append(f'int {session_param_name}')
            send_call = f'Express::GetInstance().BroadcastExcept({session_param_name}, &pkt);'
        elif send_method == 'Broadcast':
            send_call = 'Express::GetInstance().Broadcast(&pkt);'
        else:
            raise ValueError(f"Unknown send method: {send_method}") 
            
        for f in fields:
            params.append(f'{f["type"]} {f["name"]}')
        
        full_params = ', '.join(params)
        
        size = sum(TYPE_SIZES[f['type']] for f in fields)
        
        # Header
        h_lines.append(f'void {func_name}({full_params}) noexcept;')
        
        # Cpp
        cpp_lines.append(f'void {func_name}({full_params}) noexcept {{')
        cpp_lines.append('    Packet pkt;')
        cpp_lines.append(f'    pkt << (uint8_t){header_code} << (uint8_t){size} << (uint8_t)Type::{pkt["name"]};')
        
        for f in fields:
            cpp_lines.append(f'    pkt << {f["name"]};')
        
        cpp_lines.append(f'    {send_call}')
        cpp_lines.append('}')
        cpp_lines.append('')
    
    return '\n'.join(h_lines), '\n'.join(cpp_lines)


def main():
    try:
        with open('packets.json', 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        print("Error: 'packets.json' file not found.")
        return
    except json.JSONDecodeError:
        print("Error: Invalid JSON format in 'packets.json'.")
        return
        
    header_code = data['config']['header_code']
    packets = data['packets']
    
    try:
        header, cpp = generate_proxy(packets, header_code)
    except ValueError as e:
        print(f"Error during proxy generation: {e}")
        return
        
    with open('Proxy_gen.h', 'w') as f:
        f.write(header)
    print('Generated: Proxy_gen.h')
    
    with open('Proxy_gen.cpp', 'w') as f:
        f.write(cpp)
    print('Generated: Proxy_gen.cpp')


if __name__ == '__main__':
    main()