function (build_asm asm_path out_path)
    file(MAKE_DIRECTORY "${out_path}/bin")
    file(MAKE_DIRECTORY "${out_path}/dumped")

    file(GLOB files ${asm_path})
    foreach(file ${files})
        get_filename_component(filename ${file} NAME_WE)
        set(filename_bin "${filename}.bin")
        set(filename_dump "${filename}.dump")

        exec_program("riscv64-unknown-elf-gcc -Ttests/link.ld -Iriscv-tests/env/p -Iriscv-tests/isa/macros/scalar -nostdlib -ffreestanding -march=rv64g -mabi=lp64 -nostartfiles -O0 -o temp ${file}")
        exec_program("riscv64-unknown-elf-objcopy -O binary temp ${out_path}/bin/${filename_bin}")
        exec_program("riscv64-unknown-elf-objdump --disassemble-all temp > ${out_path}/dumped/${filename_dump}")
    endforeach()

    file(REMOVE temp)
endfunction()

exec_program("rm -rf testbins")
file(MAKE_DIRECTORY testbins)

build_asm("riscv-tests/isa/rv64ui/*.S" "testbins/rv64ui")
build_asm("riscv-tests/isa/rv64um/*.S" "testbins/rv64um")
build_asm("riscv-tests/isa/rv64ua/*.S" "testbins/rv64ua")
build_asm("riscv-tests/isa/rv64uf/*.S" "testbins/rv64uf")
build_asm("riscv-tests/isa/rv64ud/*.S" "testbins/rv64ud")
build_asm("riscv-tests/isa/rv64uc/*.S" "testbins/rv64uc")

build_asm("riscv-tests/isa/rv64mi/*.S" "testbins/rv64mi")
build_asm("riscv-tests/isa/rv64si/*.S" "testbins/rv64si")
