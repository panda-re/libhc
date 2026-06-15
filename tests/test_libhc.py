from pandare import Panda
import argparse
import time


def run_hypercall_test(architecture: str, ci: bool = False):
    """
    Initializes the project the PANDA project and
    test the hypercalls
    """
    panda = Panda(generic=architecture)
    class State:
        command_args = [f"./test_hc_{architecture}"]
        magic = 6767
        type_number = 42
        data = 43
        length = 44
        misc = 45
    state = State()

    @panda.queue_blocking
    def create_recording_wrapper():
        """
        Run the recording with the test binary.
        This binary run should a hypercall and confirm that the values match.
        """
        # Pass in None for snap_name since I already did the revert_sync already
        print(f"[PyPANDA TEST] Starting recording for hypercall test, copying over test_hc_{architecture} binary")
        panda.revert_sync('root')

        # 2. Wake up the guest kernel to the new hardware
        if architecture == "aarch64":
            print("[PyPANDA TEST] Forcing guest kernel to rescan PCI bus...")
            panda.run_serial_cmd("echo 1 > /sys/bus/pci/rescan")
        
            # Give the kernel a second to probe the drive and udev to create /dev/sr0
            time.sleep(2) 
        
        print("[PyPANDA TEST] Starting copy to guest...")
        try:
            panda.copy_to_guest(f"test_hc_{architecture}")
        except Exception as e:
            # If it fails, let's look at the mount points and available devices
            print("[-] Copy failed. Diagnostic info:")
            print(f"Block devices: {panda.run_serial_cmd('cat /proc/devices')}")
            print(f"LS /dev: {panda.run_serial_cmd('ls -F /dev/s*')}")
            print(f"Mounts: {panda.run_serial_cmd('mount')}")
            raise e
        print(panda.run_serial_cmd(f"./test_hc_{architecture}/test_hc_{architecture}"))
        panda.stop_run()

    @panda.hypercall(state.magic)
    def before_hc(cpu):
        print("[PyPANDA TEST] Hypercall intercepted!")
        magic = panda.arch.get_arg(cpu, 0, convention='syscall')
        type_number = panda.arch.get_arg(cpu, 1, convention='syscall')
        data = panda.arch.get_arg(cpu, 2, convention='syscall')
        length = panda.arch.get_arg(cpu, 3, convention='syscall')
        misc = panda.arch.get_arg(cpu, 4, convention='syscall')

        # Confirm the values indeed match
        assert magic == state.magic, f"Expected magic {state.magic}, got {magic}"
        assert type_number == state.type_number, f"Expected type_number {state.type_number}, got {type_number}"
        assert data == state.data, f"Expected data {state.data}, got {data}"
        assert length == state.length, f"Expected length {state.length}, got {length}"
        assert misc == state.misc, f"Expected misc {state.misc}, got {misc}"

    def record():
        """
        Start the recording
        """
        if ci:
            panda.set_complete_rr_snapshot()
        panda.run()

    record()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test running hypercalls in PANDA")
    parser.add_argument("-a", "--arch", "--architecture", type=str, choices=["i386", "x86_64", "arm", "aarch64"],
                        default="x86_64",
                        dest="architecture",
                        help="The architecture to test (default: x86_64)")
    parser.add_argument("--ci", action="store_true", default=False,
                        dest="ci",
                        help="Run in CI mode with complete RR snapshot (default: False)")
    args = parser.parse_args()
    run_hypercall_test(architecture=args.architecture, ci=args.ci)
