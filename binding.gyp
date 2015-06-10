{
  'targets': [
    {
      'target_name': 'storage',
      'sources': [
        'src/storage.cc'
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      'conditions' : [
        ['OS=="win"', {
          'libraries' : ['kernel32.lib']
        }]
      ]
    }
  ]
}
