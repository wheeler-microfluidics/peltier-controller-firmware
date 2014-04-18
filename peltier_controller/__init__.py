import os
import glob


def get_sketch_directory():
    '''
    Return directory containing the `peltier_controller` Arduino sketch.
    '''
    return os.path.join(os.path.abspath(os.path.dirname(__file__)),
                        'Arduino', 'peltier_controller')


def get_includes():
    '''
    Return directories containing the `peltier_controller` Arduino header files.

    Modules that need to compile against `peltier_controller` should use this function
    to locate the appropriate include directories.

    Notes
    =====

    For example:

        import peltier_controller 
        ...
        print ' '.join(['-I%s' % i for i in peltier_controller.get_includes()])
        ...

    '''
    import base_node
    return [base_node.get_includes(), get_sketch_directory()]


def get_sources():
    '''
    Return `peltier_controller` Arduino source file paths.

    Modules that need to compile against `peltier_controller` should use this function
    to locate the appropriate source files to compile.

    Notes
    =====

    For example:

        import peltier_controller 
        ...
        print ' '.join(peltier_controller.get_sources())
        ...

    '''
    import base_node
    includes = base_node.get_includes()
    includes.append(get_sketch_directory())
    sources = []
    for d in includes:
        sources.extend(glob.glob(os.path.join(d, '*.c*')))
    return sources
