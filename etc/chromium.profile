# Midory browser profile

# system directories	
blacklist /sbin
blacklist /usr/sbin

# system management
blacklist ${PATH}/umount
blacklist ${PATH}/mount
blacklist ${PATH}/fusermount
blacklist ${PATH}/su
blacklist ${PATH}/sudo
blacklist ${PATH}/xinput
blacklist ${PATH}/strace
	
# HOME directory
blacklist ${HOME}/.ssh
blacklist ${HOME}/.gnome2_private
blacklist ${HOME}/.gnome2/keyrings
blacklist ${HOME}/kde4/share/apps/kwallet
blacklist ${HOME}/kde/share/apps/kwallet
blacklist ${HOME}/.pki/nssdb
blacklist ${HOME}/.gnupg
blacklist ${HOME}/.adobe
blacklist ${HOME}/.macromedia
blacklist ${HOME}/.local/share/recently-used.xbel
	
# midori
blacklist ${HOME}/.mozilla
