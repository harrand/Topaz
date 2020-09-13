import subprocess
import sys

def configure():
	# Achieves the following:
	print("Configuring Topaz build system generation with default settings...")
	cmd = "cmake -B \"build/debug\" -DTOPAZ_DEBUG=1"
	if(len(sys.argv) > 1):
		generator = ""
		for i in range(1, len(sys.argv)):
			generator += sys.argv[i]
			if(i != (len(sys.argv) - 1)):
				generator += " "
		# We have an additional argument. We expect it to be the generator. Ignore all subsequent args.
		print("Note: Using the \"" + generator + "\" generator.")
		cmd += " -G \"" + generator + "\""
	subprocess.call(cmd + " -S .")

if(__name__ == "__main__"):
	configure()
