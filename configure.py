import subprocess
import sys
import argparse
from enum import Enum

class TopazBuildConfig(Enum):
	debug = 0
	release = 1
	profile = 2

class TopazRenderApi(Enum):
	vulkan = 0
	opengl = 1

class TopazBuildType:
	def __init__(self, build_config, render_api, generator):
		self.build_config = build_config
		self.render_api = render_api
		self.generator = generator

def parse_build_type():
	parser = argparse.ArgumentParser(description = 'Generate a CMake output using the given settings.')
	parser.add_argument('build_config', help='build configuration you want, e.g debug')
	parser.add_argument('render_api', help='render api to use, e.g vulkan')
	parser.add_argument('generator', help='generator for CMake to use e.g \'MinGW Makefiles\'')
	args = parser.parse_args()

	build_cfg = args.build_config
	render_api = args.render_api
	return TopazBuildType(TopazBuildConfig[build_cfg], TopazRenderApi[render_api], args.generator)
		

def configure():
	build_type = parse_build_type()
	build_config_str = build_type.build_config.name
	render_api_str = build_type.render_api.name
	build_dir = build_type.render_api.name + "_" + build_type.build_config.name
	cmd = "cmake -B \"build/{0}\" -DTOPAZ_BUILDCONFIG={1} -DTOPAZ_RENDERAPI={2} -G \"{3}\"".format(build_dir, build_config_str, render_api_str, build_type.generator)
	print("Configuring Topaz build system generation with default settings...")
	subprocess.call(cmd + " -S .")

if(__name__ == "__main__"):
	configure()
