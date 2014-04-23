from paver.easy import task, needs
from paver.setuputils import setup

import version

# Setup script for path

setup(name='wheeler.peltier_controller',
      version=version.getVersion(),
      description='Arduino-based peltier controller firmware and Python API.',
      author='Ryan Fobel',
      author_email='ryan@fobel.net',
      url='http://microfluidics.utoronto.ca/git/firmware___peltier_controller.git',
      license='GPLv2',
      packages=['peltier_controller'],
      package_data={'peltier_controller': ['Arduino/peltier_controller/*.*']},
      install_requires=['wheeler.base_node'])


@task
@needs('generate_setup', 'minilib', 'setuptools.command.sdist')
def sdist():
    """Overrides sdist to make sure that our setup.py is generated."""
    pass
