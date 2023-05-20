{
  'targets': [
    {
      'target_name': 'addon',
      'sources': [
          'src-c/binding.cpp',
          'src-c/simdjson.cpp',
          'src-c/simdjson.h',
        ],
      'configurations': {
        'Release': {
          'cflags': ['-O3', '-flto', '-march=native'],
          'ldflags': ['-flto'],
          'msvs_settings' : {
            'VCCLCompilerTool' : {
              'AdditionalOptions' : ['/Ox', '/GL', '/GF', '/Gy']
            },
            'VCLinkerTool' : {
              'AdditionalOptions' : ['/LTCG']
            },
          },
          'defines': ['NDEBUG'],
          'xcode_settings': {
            'GCC_OPTIMIZATION_LEVEL': '3',
            'OTHER_CFLAGS': ['-flto'],
            'OTHER_LDFLAGS': ['-flto'],
          },
          'conditions': [
            ["target_arch in 'ia32 x64'", {
              'cflags': ['-mavx'],
              'xcode_settings': {
                'OTHER_CFLAGS': ['-mavx'],
                'OTHER_CXXFLAGS': ['-mavx'],
              },
              'msvs_settings' : {
                'VCCLCompilerTool' : {
                  'AdditionalOptions' : ['/arch:AVX']
                },
              },
            }]
          ],
        }
      }
    }
  ]
}
