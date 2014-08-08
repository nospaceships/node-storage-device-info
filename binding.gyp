{
  'targets': [
    {
      'target_name': 'storage',
      'sources': [
        'src/storage.cc'
      ],
      'conditions' : [
        ['OS=="win"', {
          'libraries' : ['kernel32.lib']
        }]
      ]
    }
  ]
}
