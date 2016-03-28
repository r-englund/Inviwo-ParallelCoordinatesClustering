import os
import sys

workspaces = ["performance_measurements_out5d.inv", "performance_measurements_data.txt.inv", "performance_measurements_low_ratio_data.inv", "performance_measurements_low_ratio_data_2.inv", "performance_measurements_gps.inv", "performance_measurements_radar.inv"]

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

    os.system('python convert.py ' + os.getcwd() + "/" + w[25:-4] + ".html")
    # parseHtml(os.getcwd() + "/" + w[25:-4] + ".html")