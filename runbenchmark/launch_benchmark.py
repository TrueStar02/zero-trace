import subprocess

# Launch the prepare command and print the process ID
prepare_process = subprocess.Popen(['sysbench', '--threads=4', '--report-interval=4', '--time=300', '--test=fileio', '--file-num=4', '--file-total-size=4G', '--file-test-mode=rndrw', 'prepare'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
print("Prepare process ID:", prepare_process.pid)
prepare_process.wait()

# Launch the run command and print the process ID
run_process = subprocess.Popen(['sysbench', '--threads=4', '--report-interval=4', '--time=300', '--test=fileio', '--file-num=4', '--file-total-size=4G', '--file-test-mode=rndrw', 'run'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
print("Run process ID:", run_process.pid)
run_process.wait()

# Launch the cleanup command and print the process ID
cleanup_process = subprocess.Popen(['sysbench', '--threads=4', '--report-interval=4', '--time=300', '--test=fileio', '--file-num=4', '--file-total-size=4G', '--file-test-mode=rndrw', 'cleanup'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
print("Cleanup process ID:", cleanup_process.pid)
cleanup_process.wait()
