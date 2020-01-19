#!/bin/sh

echo
echo V1541Commander setup
echo
echo This script will install icons, a menu entry and filetype associations for
echo V1541Commander, which will be used by desktop environments conforming to
echo freedesktop.org, like KDE, GNOME and many others.
echo

if test $(id -u) = 0 ; then
	echo This script must not be run as root, exiting.
	echo
	exit 1;
fi

missingtool=n
missingtesttool=n

if ! tool="$(type xdg-icon-resource 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-icon-resource\' not found in path.
	missingtool=y
fi

if ! tool="$(type xdg-desktop-menu 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-desktop-menu\' not found in path.
	missingtool=y
fi

if ! tool="$(type xdg-mime 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-mime\' not found in path.
	missingtool=y
fi

if ! tool="$(type sed 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`sed\' not found.
	missingtool=y
fi

if ! tool="$(type dirname 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`dirname\' not found.
	missingtesttool=y
fi

if ! tool="$(type realpath 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`realpath\' not found.
	missingtesttool=y
fi

if test "$missingtesttool" = "y"; then
	exit 1;
fi

V1541CMD=$(dirname $0 | realpath)/v1541commander
if test ! -x $V1541CMD; then
	echo ERROR: v1541commander not found. Please make sure to place this
	echo "       script in the directory where v1541commander is located."
	missingtool=y
fi

if test "$missingtool" = "y"; then
	exit 1;
fi

read -p "Do you want to continue (y/n)? " yn
case $yn in
	[Yy]* ) break;;
	* ) exit;;
esac

echo Updating desktop file ...
sed -e "s:^Exec=v1541commander:Exec=$V1541CMD:" \
	<v1541commander.desktop >v1541commander-local.desktop
echo Installing icons ...
xdg-icon-resource install --novendor --mode user \
	--size 16 icons/16x16/v1541commander.png
xdg-icon-resource install --novendor --mode user \
	--size 32 icons/32x32/v1541commander.png
xdg-icon-resource install --novendor --mode user \
	--size 48 icons/48x48/v1541commander.png
xdg-icon-resource install --novendor --mode user \
	--size 256 icons/256x256/v1541commander.png
echo Installing menu entry ...
xdg-desktop-menu install --novendor --mode user v1541commander-local.desktop
echo Installing file type associations ...
xdg-mime install --novendor --mode user mime/v1541commander.xml
echo All done.

