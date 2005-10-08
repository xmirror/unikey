#!/bin/sh
echo 
echo "To use UniKey, certain environment variables must be set."
echo "Do you want to set these variables in /etc/profile? (y/N)"
read x
if [ "$x" = "y" -o "$x" = "Y" ]; then
	sed -f uninstall.sed /etc/profile | sed -f install.sed > tmpfile
	mv tmpfile /etc/profile
fi
