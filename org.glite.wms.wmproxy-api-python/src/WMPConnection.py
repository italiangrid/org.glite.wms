#!/usr/bin/env python2.2
import httplib
import socket
import time
import os
import glob
import sys

try:
    from OpenSSL import SSL
    from OpenSSL import crypto
except:
    print "Error: Unable to find pyOpenSSL module"
    sys.exit(1)

g_sHostName = "unknown"
g_cCtx = 0

def verifyCB( conn, cert, errnum, depth, ok):
    global g_sHostName
    if depth == 0 and ok == 1:
        sCN = cert.get_subject().commonName
        if g_sHostName == sCN or "host/%s" % g_sHostName == sCN:
            return 1
        else:
            return ok
    return ok

def initializeClientSSL( ):
    # Initialize context
    ctx = SSL.Context( SSL.SSLv23_METHOD )
    ctx.set_verify(SSL.VERIFY_PEER|SSL.VERIFY_FAIL_IF_NO_PEER_CERT, verifyCB) # Demand a certificate


    # Loads the user proxy
    try:
                 sProxyLocation = os.environ['X509_USER_PROXY']
    except:
                 sProxyLocation = '/tmp/x509up_u'+ repr(os.getuid())

    ctx.use_certificate_chain_file( sProxyLocation )
    ctx.use_privatekey_file(  sProxyLocation )

    # Looks for certificates inside /etc/grid-security/certificates
    CApaths = glob.glob( '/etc/grid-security/certificates/*.?')

    for path in CApaths:
             ctx.load_verify_locations(path)


    ctx.set_session_id( "HTTPSConnection%s" % str( time.time() ) )
    return ctx

class FakeSocket:

    def __getattr__(self, name):
        return getattr( self.sock, name )

    def __init__(self, sock):
        self.iCopies = 0
        self.sock = sock

    def close(self):
        if self.iCopies == 0:
            self.sock.shutdown()
            self.sock.close()
        else:
            self.iCopies -= 1

    def makefile(self, mode, bufsize=None):
        self.iCopies += 1
        return socket._fileobject( self.sock, mode, bufsize)


class WMPConnection( httplib.HTTPConnection ):
    iMaximumRecursionLevel = 10
    
    def __init__( self, host, cert_file, key_file, bAllowCertificates = False, port = None ):
        global g_cCtx, g_sHostName
        httplib.HTTPConnection.__init__( self, host, port )
        if host.find(':') == -1:
            g_sHostName = socket.getfqdn( host )
        else:
            g_sHostName = socket.getfqdn( host[ : host.find(':') ] )
        if not g_cCtx:
            g_cCtx = initializeClientSSL( )
    
    def close( self ):
        self.sock.close()

    def connect( self, recursionLevel = 0 ):
        global g_cCtx
        self.socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
        self.sock = FakeSocket( SSL.Connection( g_cCtx, self.socket )  )
        self.sock.connect( ( self.host, self.port ) )
        self.sock.set_connect_state()
        try:
            self.sock.do_handshake()
        except SSL.Error, v:
            raise "\n".join( [ stError[2] for stError in v.args[0] ] )


class WMPHTTPS(httplib.HTTPS):

    _connection_class = WMPConnection

 
