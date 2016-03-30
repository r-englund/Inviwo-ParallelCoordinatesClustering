import os
import sys

workspaces = [
 #    "performance_measurements_out5d_32.inv",
	# "performance_measurements_out5d_256.inv",
	# "performance_measurements_out5d_512.inv",
	# "performance_measurements_out5d_768.inv",
	# "performance_measurements_out5d_1024.inv",
 #    "performance_measurements_data.txt_32.inv", 
	# "performance_measurements_data.txt_256.inv", 
	# "performance_measurements_data.txt_512.inv", 
	# "performance_measurements_data.txt_768.inv", 
	# "performance_measurements_data.txt_1024.inv", 
 #    "performance_measurements_gps_32.inv",
	# "performance_measurements_gps_256.inv",
	# "performance_measurements_gps_512.inv",
	# "performance_measurements_gps_768.inv",
	# "performance_measurements_gps_1024.inv",
 #    "performance_measurements_radar_32.inv",
	# "performance_measurements_radar_256.inv",
	# "performance_measurements_radar_512.inv",
	# "performance_measurements_radar_768.inv",
	# "performance_measurements_radar_1024.inv",
    "performance_measurements_synthetic_32.inv",
    "performance_measurements_synthetic_256.inv",
    "performance_measurements_synthetic_512.inv",
    "performance_measurements_synthetic_768.inv",
    "performance_measurements_synthetic_1024.inv"
]

inviwoExe = "inviwo-cli.exe"
if len(sys.argv) == 2:
    inviwoExe = sys.argv[1]
print("Inviwo Executable: " + inviwoExe)

scriptPath = "perform_measurement.py"
print("Running script: " + scriptPath)

logFile = os.getcwd() + "/log.html"
print("Logfile: " + logFile)

for w in workspaces:
    try:
        os.remove(os.getcwd() + "/" + w[25:-4] + ".html")
    except:
        pass

    print("Workspace: " + w)
    cmd = inviwoExe + " -p " + scriptPath + " -l " + logFile + " -w " + w + " -n -q"
    os.system(cmd)

    os.rename(logFile, os.getcwd() + "/" + w[25:-4] + ".html")

    #os.system('python convert.py ' + os.getcwd() + "/" + w[25:-4] + ".html")
    # parseHtml(os.getcwd() + "/" + w[25:-4] + ".html")