from path_helpers import path


def package_path():
    return path(__file__).parent


def get_sketch_directory():
    '''
    Return directory containing the `peltier_controller` Arduino sketch.
    '''
    return package_path().joinpath('Arduino', 'peltier_controller')


def get_includes():
    '''
    Return directories containing the `peltier_controller` Arduino header
    files.

    Modules that need to compile against `peltier_controller` should use this
    function to locate the appropriate include directories.

    Notes
    =====

    For example:

        import peltier_controller
        ...
        print ' '.join(['-I%s' % i for i in peltier_controller.get_includes()])
        ...

    '''
    return get_sketch_directory()


def get_sources():
    '''
    Return `peltier_controller` Arduino source file paths.

    Modules that need to compile against `peltier_controller` should use this
    function to locate the appropriate source files to compile.

    Notes
    =====

    For example:

        import peltier_controller
        ...
        print ' '.join(peltier_controller.get_sources())
        ...

    '''
    return get_sketch_directory().files('*.c*')


def get_firmwares():
    '''
    Return `peltier_controller` compiled Arduino hex file paths.

    This function may be used to locate firmware binaries that are available
    for flashing to [Arduino Uno][1] boards.

    [1]: http://arduino.cc/en/Main/arduinoBoardUno
    '''
    return [f.abspath() for f in
            package_path().joinpath('firmware').walkfiles('*.hex')]
