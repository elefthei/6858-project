Progressive Authentication for Linux
====================================

The PAL system sets up a progressive authentication scheme for Linux
systems.  Upon login, users are given a fresh, anonymous user with no
privileges, and users can request additional privileges to be given to
them, by requesting additional groups to be added to their shell.  A
centralized authentication daemon handles these authentication
requests.

Installation
------------

Before installing, ensure that the following prerequisite tools are
set up:

 + `sudo` can correctly authenticate users and allow them root access
 + OpenSSH's `sshd` is set up to allow access from the outside world
   via public-key authentication.

### Setting up the Login System

In short, to set up the login system, you need to:

 + Copy the `incperm` subdirectory to `/srv/incperm/`
 + Create the `incperm` user with home directory `/srv/incperm/home/`
 + Add the appropriate `authorized_keys` entries
 + Edit the `sudoers` file.

In the future, an install script will be available, but for now
installation must be done manually.  Luckily, it's not very hard.
Here's the step-by-step.

First, copy the `incperm` subdirectory in this source distribution to
`/srv/incperm/`; for now, this path is hardcoded, though it might be
changable in the future.

    sudo cp -r incperm /srv/incperm/
    
Next, create a user and group which will manage the progressive
authentication system; `incperm` is a good name for them.  You can set
this user's home directory to `/srv/incperm/home/`.  Ensure that all
the files are owned by `incperm` or, better yet, by root.  It's best
to give the `incperm` user some absurdly long password (100 characters
is a good start), or to edit the global SSH config to prevent
password-based authentication into this user.

    sudo adduser incperm
    sudo addgroup incperm
    sudo chown root:root /srv/incperm/

Next, edit the file `~incperm/.ssh/authorized_keys`, which manages SSH
authentication for the `incperm` user.

    sudo -e /srv/incperm/home/.ssh/authorized_keys
    
To this file, add each of the public keys that should be allowed to
connect to your server, prefixed by the string
`command="/srv/incperm/bin/on-login"`.  This string should come before
the `ssh-dss` or `ssh-rsa` part of the key -- it should be the very
start of each line.  Note that public key login is currently required
for using PAL; if you want password-based authentication, you could
always just post the encrypted private key on the public internet.
That sounds dumb, but hey, so is password-based SSH.

The login system needs root access on user login, to create a new
anonymous user and to handle internal state (which of course cannot be
user-owned).  Thus, certain commands need to be added to the `sudoers`
file to execute passwordless.  It should be possible for the `incperm`
user to execute `/srv/incperm/bin/setup-jail` and to execute any
command as a user in the `incperm` group.  These two can be achieved
by adding the following line to the sudoers file via `sudo visudo`:

    incperm   ALL=(root)     NOPASSWD: /srv/incperm/bin/setup-jail
    incperm   ALL=(%incperm) NOPASSWD: ALL
    
The first line allows the `incperm` user to execute
`/srv/incperm/bin/setup-jail` as root without a password, and the
second allows him to execute any command as any member of the
`incperm` group without a password.

Finally, test the login system by attempting to SSH into localhost:

    ssh incperm@localhost

You should be greated by something that looks like this:

    Your temporary username is incperm-65537; enjoy!
    incperm-65537@my-host:~$ 

Your default BASH prompt may, of course, be different, but the line
about the temporary username, and the strange login name, tells you
that PAL's login mechanism is set up correctly.

If logging in causes `sudo` to ask for a password, or to throw an
error, that means you did not correctly add
`/srv/incperm/bin/setup-jail` to the set of commands the `incperm`
user could execute without a password; you should fix that.  If there
are OpenSSH error indicating that the login command could not be
executed, the initial copy of the `incperm` directory to
`/srv/incperm` was unsuccessful.  If OpenSSH claims that no `incperm`
user exists, please set one up.
