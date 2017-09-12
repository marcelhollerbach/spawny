check = dependency('check')

src = [
  'test.c',
  'test.h',
  'test_array.c',
  'test_manager.c',
  'test_templatereg.c',
  'test_spawnregistery.c',
  'test_utils.c',
  '../sp-daemon/log.h',
  '../sp-daemon/log.c',
  '../sp-daemon/array.h',
  '../sp-daemon/array.c',
  '../sp-daemon/manager.h',
  '../sp-daemon/manager.c',
  '../sp-daemon/spawnregistery.c',
  '../sp-daemon/spawnregistery.h',
  '../sp-daemon/sessionmgt.h',
  '../sp-daemon/templatereg.h',
  '../sp-daemon/templatereg.c',
  '../sp-daemon/utils.h',
  '../sp-daemon/utils.c',
]

suite = executable('test_suite', src,
   dependencies : [sp_protocol, sp_client, sp_user_db, inih.get_variable('inih'), check, m]
)
test('test', suite)
