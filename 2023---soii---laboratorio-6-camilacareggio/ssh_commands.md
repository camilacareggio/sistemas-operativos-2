sudo cat /etc/shadow | grep '^[^:]*:[^\*!]'

## Delete user
sudo su - 
sudo userdel newuser
Ctrl+D para salir del modo root

## Create user
comando curl
passwd newuser
ssh newuser@dashboard.com