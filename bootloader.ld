MEMORY
{
    ram : ORIGIN = 0x0000, LENGTH = 0x1000000
}

SECTIONS
{
    .text : { *(.text*) } > ram

    .bss : { *(.bss*) } > ram

    .common : { *(.common*) } > ram
    
    .heap : {
        __heap_start__ = .;
        end = __heap_start__;
        _end = end;
        __end = end;

        KEEP(*(.heap))
        __heap_end__ = .;
        __HeapLimit = __heap_end__;
    } > ram
}
