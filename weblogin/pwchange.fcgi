#!/usr/bin/perl
#
# pwchange.fcgi -- WebLogin password change page for WebAuth.
#
# This is the page to change user password for weblogin.  It accepts
# information from the user, passes it to the WebKDC for authentication, and
# changes password if credentials match.
#
# It should use FastCGI if available, using the Perl CGI::Fast module's
# ability to fall back on regular operation if FastCGI isn't available.
#
# Written by Jon Robertson <jonrober@stanford.edu>
# Copyright 2010, 2011, 2012
#     The Board of Trustees of the Leland Stanford Junior University
#
# See LICENSE for licensing terms.

##############################################################################
# Modules and declarations
##############################################################################

require 5.006;

use strict;
use warnings;

use CGI::Fast;
use WebLogin;

# Set to true in our signal handler to indicate that the script should exit
# once it finishes processing the current request.
our $EXITING = 0;

# The names of the page templates, relative to the template path configured in
# the WebLogin configuration file.  This is set in this driver so that a
# modified driver script can use different template names, allowing multiple
# login interfaces with different UIs.
our %PAGES = (
    login       => 'login.tmpl',
    logout      => 'logout.tmpl',
    confirm     => 'confirm.tmpl',
    pwchange    => 'pwchange.tmpl',
    multifactor => 'multifactor.tmpl',
    error       => 'error.tmpl',
);

# The main loop.  If we're not running under FastCGI, CGI::Fast will detect
# that and only run us through the loop once.  Otherwise, we live in this
# processing loop until the FastCGI socket closes.
while (my $q = CGI::Fast->new()) {
    local $SIG{TERM} = sub { $EXITING = 1 };
    if (!defined $q->param('rm')) {
        $q->param('rm', 'pwchange');
    }
    my $weblogin = WebLogin->new(
        PARAMS => { pages => \%PAGES },
        QUERY  => $q
    );
    $weblogin->run();
}

# Done on each pass through the FastCGI loop.  Restart the script if we've
# been signaled or the script modification time has changed.
continue {
    if ($EXITING || -M $ENV{SCRIPT_FILENAME} < 0) {
        exit;
    }
}
