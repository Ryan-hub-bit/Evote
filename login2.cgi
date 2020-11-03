#!/usr/bin/env python3

import cgi
from http.cookies import SimpleCookie
import hashlib
import time
import os
import html

PATH_TO_PASSWD = "./machine_passwd"
redirectURL = "./admin.cgi"

def render_login(failure=False, logout=False):
    print("Content-Type: text/html")
    if logout:
        print("Set-Cookie: user=LOGGEDOUT; expires=Thu, 01 Jan 1970 00:00:00 GMT")
        time.sleep(20)
        f = open("./adminip", "w")
        f.write("null")
        f.close
    print()
    print('<link rel="stylesheet" href="https://spar.isi.jhu.edu/teaching/443/main.css">')
    print('<h2 id="dlobeid-etovucca-voting-machine">DLOBEID EtovUcca Voting Machine</h2>')
    print('<h1 id="admin-interface-login">Admin Interface Login</h1><br>')
    if failure:
        print('<b>Login Failed.</b>')
    print('<form method="post">')
    print('<label for="passwd">Admin Password:</label>')
    print('<input type="password" id="passwd" name="passwd">')
    print('<input type="submit" value="Login">')
    print('</form>')
    print("<a href='./home.cgi'>Return to Homepage</a>")

form = cgi.FieldStorage()

ipf = open("./adminip", "r")
tempip = ipf.readline()
ipf.close()

ipf = open("./test", "w")
ipf.write(html.escape(os.environ["REMOTE_ADDR"]))
ipf.close()

if html.escape(os.environ["REMOTE_ADDR"]) == tempip:
       #print('Content-Type: text/html')
       #print('Location: %s' % redirectURL)
        C = SimpleCookie()
        C['user'] = open(PATH_TO_PASSWD).read(32)
        ipf = open("./test", "w")
        ipf.write("We're in")
        ipf.close()
        print(C)
        # CGI Redirect: https://stackoverflow.com/a/6123179
        print('Content-Type: text/html')
        print('Location: %s' % redirectURL)
        print(C)
        print('')
        print('<html>')
        print('<head>')
        print('<link rel="stylesheet" href="https://spar.isi.jhu.edu/teaching/443/main.css">')
        print('<meta http-equiv="refresh" content="0;url=%s" />' % redirectURL)
        print('<title>You are going to be redirected</title>')
        print('</head>')
        print('<body>')
        print('Redirecting... <a href="%s">Click here if you are not redirected</a>' % redirectURL)
        print('</body>')
        print('</html>')

else:
        ipf = open("./test", "w")
        ipf.write("RIP")
        ipf.close()

try:
    if 'passwd' in form:
        f = open("./adminip", "w")
        f.write(html.escape(os.environ["REMOTE_ADDR"]))
        f.close()
        # Please don't ever actually do this.
        h = hashlib.new('md5')  # U+1F914
        h.update(form.getvalue('passwd').encode('utf-8'))
        with open(PATH_TO_PASSWD) as f:
            stored_hash = f.read(32)
            if h.hexdigest() == stored_hash:
                # CGI Redirect: https://stackoverflow.com/a/6123179
                print('Content-Type: text/html')
                print('Location: %s' % redirectURL)
                C = SimpleCookie()
                C['user'] = h.hexdigest() # U+1F914
                print(C)
                print('')
                print('<html>')
                print('<head>')
                print('<link rel="stylesheet" href="https://spar.isi.jhu.edu/teaching/443/main.css">')
                print('<meta http-equiv="refresh" content="0;url=%s" />' % redirectURL)
                print('<title>You are going to be redirected</title>')
                print('</head>')
                print('<body>')
                print('Redirecting... <a href="%s">Click here if you are not redirected</a>' % redirectURL)
                print('</body>')
                print('</html>')
            else:
                time.sleep(20)
                f = open("./adminip", "w")
                f.write(tempip)
                f.close
                raise ValueError('incorrect hash')
    elif 'logout' in form:
        render_login(logout=True)
    else:
        render_login()
except Exception:
    render_login(failure=True)
