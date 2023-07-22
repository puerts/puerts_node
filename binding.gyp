{
  'targets': [{
    'target_name': 'puerts_node',
    'sources': [
      'src/CppObjectMapper.cpp',
      'src/DataTransfer.cpp',
      'src/JSClassRegister.cpp',
      'src/PesapiAddonLoad.cpp',
      'src/PesapiV8Impl.cpp'
    ],
    'include_dirs': [
    ],
    "defines": [ "MAPPER_ISOLATE_DATA_POS=2"],
    'dependencies': [
    ],
    'cflags!': [ '-fno-exceptions' ],
    'cflags_cc!': [ '-fno-exceptions' ],
    'xcode_settings': {
      'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
      'CLANG_CXX_LIBRARY': 'libc++',
      'MACOSX_DEPLOYMENT_TARGET': '10.7',
    },
    'msvs_settings': {
      'VCCLCompilerTool': { 'ExceptionHandling': 1 },
    },
    'conditions': [
      ['OS=="win"', {
        'sources': [
            'src/win32-dlfcn.cc'
        ]
      }]
    ]
  }]
}
