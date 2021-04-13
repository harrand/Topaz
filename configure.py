import subprocess
import sys
import argparse
from enum import Enum

class TopazBuildConfig(Enum):
	debug = 0
	release = 1

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
	# Achieves the following:
	build_type = parse_build_type()
	build_dir = build_type.render_api.name + "_" + build_type.build_config.name
	cmd = "cmake -B \"build/%s\" -DTOPAZ_DEBUG=%d -DTOPAZ_VULKAN=%d -DTOPAZ_OGL=%d -G \"%s\"" % (build_dir, 1 if build_type.build_config == TopazBuildConfig.debug else 0, 1 if build_type.render_api == TopazRenderApi.vulkan else 0, 1 if build_type.render_api == TopazRenderApi.opengl else 0, build_type.generator)
	print("Configuring Topaz build system generation with default settings...")
	subprocess.call(cmd + " -S .")

if(__name__ == "__main__"):
	configure()