# How to install WebAuth server on a CentOS 7 host

## Install the prerequisites

Install the required RPM packages:

```
yum -y groupinstall "Development tools"
yum -y install wget httpd php openssl mod_ssl \
       httpd-devel libcurl-devel openssl-devel 
```

WebAuth depends upon a lot of Perl modules. In general I install the RPM package if a Perl module is available as one; otherwise I use `cpanm` to install the missing Perl modules.

```
yum -y install perl-CGI-Application perl-libwww-perl perl-Crypt-SSLeay \
               perl-Template-Toolkit perl-URI perl-XML-Parser \
               perl-Cache-Memcached perl-FreezeThaw perl-Time-Duration

yum -y install perl-App-cpanminus
cpanm Test::More
cpanm CGI::Application::Plugin::AutoRunmode
cpanm CGI::Application::Plugin::Forward
cpanm CGI::Application::Plugin::Redirect
cpanm CGI::Application::Plugin::TT
```

## Build WebKDC and WebAuth

Here we build both the server components (*WebKDC*) and the client components (*mod_webauth* & *mod_webauthldap*):

```
mkdir -p /opt/src
cd /opt/src
git clone https://github.com/shawfdong/webauth.git
cd webauth
./configure --prefix=/opt/webauth
make
make check
make install
```
