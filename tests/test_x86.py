from zbtools.x86 import Emulator, Register


def test_code_runs_and_registers_and_memory_are_visible() -> None:
    cpu = Emulator()
    cpu.map(0x1000, 0x1000)
    # mov eax, [0x1800]; add eax, 3; mov [0x1804], eax; hlt
    cpu.write(0x1000, bytes.fromhex("a1 00 18 00 00 83 c0 03 a3 04 18 00 00 f4".replace(" ", "")))
    cpu.write32(0x1800, 39)
    cpu.run(0x1000, 0x100D, 100)
    assert cpu.register(Register.EAX) == 42
    assert cpu.read32(0x1804) == 42


def test_instructions_in_a_range_can_be_intercepted() -> None:
    cpu = Emulator()
    cpu.map(0x1000, 0x1000)
    cpu.map(0x2000, 0x1000)
    cpu.write(0x2000, b"\xc3")  # ret
    # call 0x2000; hlt
    cpu.write(0x1000, bytes.fromhex("e8 fb 0f 00 00 f4".replace(" ", "")))
    cpu.set_register(Register.ESP, 0x1F00)
    seen = []

    def stub(address: int) -> None:
        seen.append(address)
        cpu.set_register(Register.EAX, 7)
        cpu.set_register(Register.EIP, cpu.read32(cpu.register(Register.ESP)))  # return
        cpu.set_register(Register.ESP, cpu.register(Register.ESP) + 4)

    cpu.on_execute(stub, 0x2000, 0x2000)
    cpu.run(0x1000, 0x1005, 100)
    assert seen == [0x2000]
    assert cpu.register(Register.EAX) == 7
