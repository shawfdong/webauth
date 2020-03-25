# How to install WebAuth server on a CentOS 7 host

## Install the prerequisites

Install the required RPM packages:

```
yum -y groupinstall "Development tools"
yum -y install wget httpd php openssl mod_ssl \
       httpd-devel libcurl-devel openssl-devel libgcrypt libgcrypt-devel 
```

WebAuth depends upon a lot of Perl modules. In general I install the RPM package if a Perl module is available as one; otherwise I use `cpanm` to install the missing Perl modules.

```
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

## Build WebKDC and WebAuth

Here we build both the server components (*WebKDC*) and the client components (*mod_webauth* & *mod_webauthldap*):

```
git clone https://github.com/shawfdong/webauth.git
cd webauth
./configure --enable-webkdc
make
make check
make install
echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf
ldconfig
```

Copy weblogin scripts to `/var/www`:

```
cp -r /usr/local/share/weblogin /var/www/
```

and modify them to your heart's delight.

## Install mod_fcgid

Weblogin scripts are Perl FastCGI scripts, so we install `mod_fcgid` to provide FastCGI support (optional):

```
yum -y install mod_fcgid
```  

## Install memcached

This is also optional. But we can use `memcached` to speed up WebKDC. First install memcached:

```
yum -y install memcached libmemcached
```

Then modify `/etc/sysconfig/memcached` as follows:

```
PORT="11211"
USER="memcached"
MAXCONN="2048"
CACHESIZE="4096"
OPTIONS="-l 127.0.0.1 -U 0"
```

Note here I disable the UDP port. Then enable and start the memcached service:

```
systemctl enable memcached
systemctl start memcached
```

Add the following lines to WebKDC configuration (`/etc/webkdc/webkdc.conf`):

```
@MEMCACHED_SERVERS = ('127.0.0.1:11211');
$REPLAY_TIMEOUT = 300;
$RATE_LIMIT_THRESHOLD = 5;
```

## Kerberos Keytabs

A number of Kerberos keytabs are present on webauth701:

* `/etc/krb5.keytab` is the host keytab (`host/webauth701.slac.stanford.edu@SLAC.STANFORD.EDU`);
* `/etc/webauth/keytab` is the webauth client keytab (`webauth/webauth701.slac.stanford.edu@SLAC.STANFORD.EDU`)
* `/etc/httpd/conf/webkdc/keytab` is the webkdc keytab (`service/webkdc@SLAC.STANFORD.EDU`)
* `/etc/httpd/conf/http.keytab` is the HTTP keytab (`HTTP/webauth701.slac.stanford.edu@SLAC.STANFORD.EDU`)

## Apache Configurations

* `/etc/httpd/conf/http.conf`: the main configuration
* `/etc/httpd/conf.d/fcgid.conf`: FastCGI configuration
* `/etc/httpd/conf.d/webkdc.conf`: WebKDC configuration
* `/etc/httpd/conf.d/webauth.conf`: webauth configuration 
