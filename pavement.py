from pprint import pprint

from paver.easy import task, needs, path, sh, cmdopts, options
from paver.setuputils import setup, find_package_data

import version
import peltier_controller

peltier_controller_files = find_package_data(package='peltier_controller',
                                             where='peltier_controller',
                                             only_in_packages=False)
pprint(peltier_controller_files)

setup(name='wheeler.peltier-controller-firmware',
      version=version.getVersion(),
      description='Arduino-based peltier controller firmware and Python API.',
      author='Ryan Fobel',
      author_email='ryan@fobel.net',
      url='https://github.com/wheeler-microfluidics/peltier-controller-firmware',
      license='GPLv2',
      packages=['peltier_controller'],
      package_data=peltier_controller_files,
      install_requires=['wheeler.base-node'])


@task
def create_config():
    sketch_directory = path(peltier_controller.get_sketch_directory())
    sketch_directory.joinpath('Config.h.skeleton').copy(sketch_directory
                                                        .joinpath('Config.h'))


@task
@needs('create_config')
@cmdopts([('sconsflags=', 'f', 'Flags to pass to SCons.')])
def build_firmware():
    sh('scons %s' % getattr(options, 'sconsflags', ''))


@task
@needs('generate_setup', 'minilib', 'build_firmware',
       'setuptools.command.sdist')
def sdist():
    """Overrides sdist to make sure that our setup.py is generated."""
    pass
