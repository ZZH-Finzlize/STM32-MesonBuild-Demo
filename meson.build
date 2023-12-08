project('demo', ['c', 'cpp'],
    version: '0.1',
    license: 'GPL-3.0',
    license_files: 'LICENSE',
    default_options: ['warning_level=3']
)

# check cross compile
assert(meson.is_cross_build(),
    'This project must be cross compile!\n' +
    'add --cross-file= to switch to corss compile'
)

# check compile for STM32
target_mcu = meson.get_external_property('target_mcu', 'null')
assert(target_mcu.contains('stm32f10'),
    'This project is only for STM32 F10x series(your selection: @0@)\n'.format(target_mcu) + 
    'Please select your MCU cross file in cross_files subdir'
)

# check specific device info
capacity = meson.get_external_property('capacity', 'null')
allowed_capacity_list = ['LD', 'LD_VL', 'MD', 'MD_VL',
                         'HD', 'HD_VL', 'XL', 'CL']
if false == allowed_capacity_list.contains(capacity)
    error('Unsupport capacity (your selection: @0@)\r\n'.format(capacity),
            'supported capacities are: ', allowed_capacity_list)
endif

fs = import('fs')

# stm32 lib path
dir_prefix = 'STM32_StdLib/'

# stm32 lib include dirs
incs = [
    'CMSIS/CoreSupport',
    'CMSIS/DeviceSupport',
    'Driver/inc'
]

# peripheral drvs
stm32_mod_path = 'Driver/src/stm32f10x_@0@.cpp'
stm32_modules = {
    'adc': true, 'bkp': true, 'can': true, 'cec': true, 'crc': true,
    'dac': true, 'dbgmcu': true, 'dma': true, 'exti': true, 'flash': true,
    'fsmc': true, 'gpio': true, 'i2c': true, 'iwdg': true, 'pwr': true,
    'rcc': true, 'rtc': true, 'sdio': true, 'spi': true, 'tim': true,
    'usart': true, 'wwdg': true,
}

# other support files
stm32_std_lib_srcs = [
    # 'CMSIS/DeviceSupport/startup/startup_stm32f10x_md.s',
    'CMSIS/DeviceSupport/system_stm32f10x.cpp',
    'CMSIS/CoreSupport/core_cm3.cpp',
    'Driver/src/misc.cpp',
]

# add enabled module files to compile list
foreach module, enable : stm32_modules
    if enable
        stm32_std_lib_srcs += stm32_mod_path.format(module)
    endif
endforeach

# utils
util_srcs = [
    'mem_mana/mem_mana.cpp',
    'bitmap/bitmap.cpp',
    'usart/prints.cpp',
    'ringbuf/ringbuf.cpp',
    'tiny_console/tiny_console.cpp',
    'tiny_console/builtin_cmds/help.cpp',
    'delay/delay.cpp',
    'hash/str_hash.cpp',
    'map/map.cpp',
    'value_ops.cpp',
    'strarg.cpp',
]

# test cases
test_cases_srcs = [
    'test_frame.cpp',
    'hash/demo.cpp',
    'bitmap/bitmap_basic.cpp',
    'bitmap/bitmap_clear.cpp',
    'map/map_test_data.cpp',
    'map/map_basic_test.cpp',
    # 'map/map_detete_clear_test.cpp',
    # 'map/map_duplcate_key.cpp',
]

# start files
start_file_fmt = 'startup_stm32f10x_@0@.cpp'
start_file_srcs = [
    'stack_trace/stack_trace.cpp',
    'init_calls.s',
]

# applicaton src files
app_srcs = [
    'app/main.cpp',
    'app/console_commands.cpp',
    'app/isr.cpp',
]

srcs = app_srcs
inc_dirs = [
    'app',
    'test_cases'
]

# append start files
srcs += 'start_files' / start_file_fmt.format(capacity.to_lower())
foreach src : start_file_srcs
    srcs += 'start_files' / src
endforeach

# append utils
foreach src : util_srcs
    srcs += 'util' / src
endforeach

# append test cases
foreach src : test_cases_srcs
    srcs += 'test_cases' / src
endforeach

# append stm32 lib src files to all src list
foreach src : stm32_std_lib_srcs
    srcs += dir_prefix / src
endforeach

# append stm32 lib inc files to all include dirs
foreach inc : incs
    inc_dirs += dir_prefix / inc
endforeach

# message('srcs:', srcs)
# message('inc_dirs:', inc_dirs)

# read .config and generate marcos
configs = []
if fs.exists('.config')
    config_lines = fs.read('.config')
    # message('configs:', configs)
    foreach line: config_lines.split('\n')
        if line.contains('=')
            line = line.strip()
            # message('config line: ', line)
            configs += '-DCONFIG_' + line
        endif
    endforeach
endif

# message('using configs: ', configs)

# get cross tools from cross file
objcopy_tool = find_program('objcopy')
objdump_tool = find_program('objdump')
size_tool = find_program('size')
gdb_tool = find_program('gdb')

proj_name = meson.project_name()
src_dir = meson.current_source_dir()
build_dir = meson.current_build_dir()
exe_sufix = meson.get_external_property('exe_sufix', 'elf')

# c_args apply to all src files
c_args = [
    '-mcpu=' + host_machine.cpu(),
    '-DSTM32F10X_' + capacity,
    '-DUSE_STDPERIPH_DRIVER',
    '-DPROJECT_VERSION=' + '"' + meson.project_version() + '"',
    '-fdata-sections',
    '-ffunction-sections',
    '-Wall', '-Wextra',
]
c_args += configs

# cpp_args apply to all src files
cpp_args = c_args

# specify language stadard
c_args += '-std=gnu99'
cpp_args += '-std=gnu++20'

# link args apply to the project
link_args = [
    '-mcpu=' + host_machine.cpu(),
    '-nostartfiles',
]

# get memory configuration from cross file
mem_cfg = {
    'flash_base': meson.get_external_property('flash_base'),
    'flash_size': meson.get_external_property('flash_size'),
    'ram_base': meson.get_external_property('ram_base'),
    'ram_size': meson.get_external_property('ram_size'),
    'stack_size': meson.get_external_property('stack_size', 0x400),
}

# all target description
target_dict = {
    'dbg': {
        'c_args': [
            '-Og',
            '-D__DEBUG',
            '-DVECT_TAB_SRAM',
            '-g3',
            '-Wa,-g3'
        ],
        'cpp_args': [],
        'link_args': ['-Wl,-Map,memory_ram.map', '--specs=nano.specs'],
        'linker_script': 'linker_ram_new.ld',
        'debug': true,
    },
    'rel': {
        'c_args': ['-O2'],
        'cpp_args': [],
        'link_args': ['-Wl,-Map,memory.map', '--specs=nano.specs'],
        'linker_script': 'linker_new.ld',
        'download': true,
    }
}

top_objs = []
dbg_target = ''
dl_target = ''

# generate all target build setup
foreach target_name, options : target_dict
    target_srcs = srcs + options.get('srcs', [])
    exe_name = proj_name + '_@0@.@1@'.format(target_name, exe_sufix)
    lds_name = target_name + '_' + options['linker_script']

    lds = configure_file(
        input: 'linker_sct' / options['linker_script'],
        output: lds_name,
        configuration: mem_cfg
    )

    lds_arg = '-T' + (build_dir / lds_name)

    exe = executable(exe_name, target_srcs,
        include_directories: inc_dirs,
        link_depends: lds,
        c_args: c_args + options.get('c_args', []),
        cpp_args: cpp_args + options.get('c_args', []) + options.get('cpp_args', []),
        link_args: link_args + lds_arg + options.get('link_args', [])
    )

    dis = custom_target(target_name + '_dis',
        depends: exe,
        capture: true,
        command: [
            objdump_tool.full_path(),
            '-SD',
            build_dir / exe.name()
        ],
        output: fs.replace_suffix(exe.name(), '.s')
    )

    size = run_target(target_name + '_size',
        depends: dis,
        command: [
            size_tool.full_path(),
            build_dir / exe.name(),
        ]
    )

    top_objs += dis

    bin = custom_target(target_name + '_bin',
        depends: dis,
        command: [
            objcopy_tool.full_path(),
            '-O', 'binary',
            build_dir / exe.name(),
            build_dir / '@OUTPUT@'
        ],
        output: fs.replace_suffix(exe.name(), '.bin')
    )

    if options.get('debug', false)
        dbg_target = exe
    endif

    if options.get('download', false)
        dl_target = bin
    endif

endforeach

# custom target to build all target in the 'target_dict'
run_target('top_all',
    depends: top_objs,
    command: ['echo', 'all target built done']
)

# generate vscode workspace file
cross_file = 'cross_files' / target_mcu + '.ini'
dfp_file = meson.get_external_property('dfp_file')
device_name = meson.get_external_property('device_name')
debugger_file = meson.get_external_property('debugger_file')
configure_file(
    input: 'proj_files' / 'workspace_demo.json',
    output: proj_name + '.code-workspace',
    configuration: {
        'debug_file': dbg_target.name(),
        'gdb_path': gdb_tool.full_path(),
        'cross_file': cross_file,
        'dfp_file': dfp_file,
        'device_name': device_name,
        'debugger_file': debugger_file,
        'debugger_file_ini': fs.replace_suffix(debugger_file, '.ini')
    },
)

# generate download script
configure_file(
    input: 'proj_files' / 'download_demo.sh',
    output: 'download.sh',
    configuration: {
        'flash_base': mem_cfg['flash_base']
    }
)

# download target
dl_cmd = 'flash write_image erase '
dl_cmd += dl_target.full_path() + ' '
dl_cmd += mem_cfg['flash_base'] + ' '

supported_tools = {
    'stlink': 'stlink.cfg',
    'dap': 'cmsis-dap.cfg'
}

foreach tool, cfg : supported_tools
    run_target(tool + ' - download',
        depends: dl_target,
        command: [
            'openocd',
            '-f', '..' / 'openocd_cfg' / cfg,
            '-f', '..' / 'openocd_cfg' / 'stm32f1x.cfg',
            '-c init;reset halt;wait_halt;',
            '-c', dl_cmd,
            '-c reset; shutdown;'
        ]
    )
endforeach
