# How to use WebAuth for authentication on a CentOS 7 host

## Build WebAuth on a CentOS 7 host

First install the prerequisites:

```
yum -y groupinstall "Development tools"
yum -y install wget httpd php openssl mod_ssl \
       httpd-devel libcurl-devel openssl-devel libgcrypt libgcrypt-devel 
yum -y install perl-CGI-Application perl-libwww-perl perl-Crypt-SSLeay \
               perl-Template-Toolkit perl-URI perl-XML-Parser perl-LWP-Protocol-https \
               perl-Cache-Memcached perl-FreezeThaw perl-Time-Duration
yum -y install perl-App-cpanminus
cpanm Test::More
cpanm CGI::Application::Plugin::AutoRunmode
cpanm CGI::Application::Plugin::Forward
cpanm CGI::Application::Plugin::Redirect
cpanm CGI::Application::Plugin::TT
cpanm Crypt::GCrypt
```

Then build WebAuth:

```
git clone https://github.com/shawfdong/webauth.git
cd webauth
./configure
make
make check
make install
```

## Keytab

WebAuth requires a *webauth* keytab. Below is an example:

```
$ ktutil -k /tmp/webauth702_webauth.keytab list
Vno  Type                     Principal                                               Aliases
  1  aes256-cts-hmac-sha1-96  webauth/webauth702.slac.stanford.edu@SLAC.STANFORD.EDU
  1  des3-cbc-sha1            webauth/webauth702.slac.stanford.edu@SLAC.STANFORD.EDU
  1  arcfour-hmac-md5         webauth/webauth702.slac.stanford.edu@SLAC.STANFORD.EDU
```

Create such a keytab, then copy it to `/etc/webauth`:

```
mkdir /etc/webauth
mv /tmp/webauth702_webauth.keytab /etc/webauth/keytab
chown apache:root /etc/webauth/keytab
```

## Apache configuration

At SLAC, use the following WebAuth configuration for Apache HTTP server (`/etc/httpd/conf.d/webauth.conf`):

```
# Basic configuration for the WebAuth module.

LoadModule webauth_module /usr/local/libexec/apache2/modules/mod_webauth.so

WebAuthKeyring /var/lib/webauth/keyring
WebAuthKeytab /etc/webauth/keytab
WebAuthServiceTokenCache /var/lib/webauth/service_token_cache
WebAuthSSLRedirect on

WebAuthKeyringAutoUpdate on
WebAuthKeyringKeyLifetime 30d

WebAuthLoginURL "https://webauth.slac.stanford.edu/login-auto"
WebAuthWebKdcURL "https://webauth.slac.stanford.edu/webkdc-service/"

WebAuthWebKdcPrincipal service/webkdc

WebAuthWebkdcSSLCertCheck on

WebAuthDebug off
LogLevel warn
```
