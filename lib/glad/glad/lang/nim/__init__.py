from glad.lang.nim.generator import NimGenerator
from glad.lang.nim.loader.gl import OpenGLNimLoader

# TODO finish converting the egl, glx & wgl loaders to Nim

_specs = {
#    'egl': EGLNimLoader,
    'gl': OpenGLNimLoader
#    'glx': GLXNimLoader,
#    'wgl': WGLNimLoader
}

_generators = {
    'nim': NimGenerator,
}


def get_generator(name, spec):
    gen = _generators.get(name)
    loader = _specs.get(spec)

    return gen, loader
