## Nginx Installation
apt update  
apt upgrade -y  
apt install nginx  

## Sensors Service

### Nginx Config 
sudo vim /etc/nginx/sites-available/sensors.conf
```   
server {
   listen 80;
   listen [::]:80;

   server_name    sensors.com;

   location / {
        proxy_pass http://localhost:9100;
    }
    location ~ /.well-known {
        allow all;
    }

}
```

`/etc/nginx/sites-enabled$` sudo ln -s ../sites-available/sensors.conf  
`/etc/nginx/sites-enabled$` ls -l  
sudo unlink /etc/nginx/sites-enabled/default  
sudo service nginx restart

### Systemd Config
sudo vim /etc/systemd/system/sensors.service  
```
[Unit]
Description=sensors.com
ConditionPathExists=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/sensors
After=network.target

[Service]
Type=simple
User=root
Group=root

WorkingDirectory=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/sensors
ExecStart=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/sensors/sensors

Restart=on-failure
RestartSec=10

ExecStartPre=/bin/mkdir -p /var/log/sensors
ExecStartPre=/bin/chown syslog:adm /var/log/sensors
ExecStartPre=/bin/chmod 775 /home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/sensors

[Install]
WantedBy=multi-user.target
```

sudo systemctl daemon-reload  
sudo systemctl start sensors   
sudo systemctl enable sensors  

systemctl status sensors 
Output:  
```
sensors.service - sensors.com
     Loaded: loaded (/etc/systemd/system/sensors.service; enabled; vendor preset: enabled)
     Active: active (running) since Mon 2023-05-29 11:25:31 -03; 11min ago
   Main PID: 5017 (sensors)
      Tasks: 6 (limit: 9230)
     Memory: 5.7M
        CPU: 20ms
     CGroup: /system.slice/sensors.service
             └─5017 /home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/sensors/sensors

May 29 11:25:31 camila-Aspire-E5-575G systemd[1]: Starting sensors.com...
May 29 11:25:31 camila-Aspire-E5-575G systemd[1]: Started sensors.com.
May 29 11:25:31 camila-Aspire-E5-575G sensors[5017]: 2023/05/29 11:25:31 Iniciando el servidor en http://localhost:9100  
```
## Users Service
### Nginx Config 
sudo vim /etc/nginx/sites-available/dashboard.conf
```   
server {
   listen 80;
   listen [::]:80;

   server_name    dashboard.com;

   location / {
        proxy_pass http://localhost:8000;
    }
    location ~ /.well-known {
        allow all;
    }

}
```

`/etc/nginx/sites-enabled$` sudo ln -s ../sites-available/dashboard.conf  
`/etc/nginx/sites-enabled$` ls -l   
```
lrwxrwxrwx 1 root root 33 Jun  1 12:36 dashboard.conf -> ../sites-available/dashboard.conf
lrwxrwxrwx 1 root root 31 May 29 12:18 sensors.conf -> ../sites-available/sensors.conf
```
sudo service nginx restart

### Systemd Config
sudo vim /etc/systemd/system/dashboard.service  
```
[Unit]
Description=dashboard.com
ConditionPathExists=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/users
After=network.target

[Service]
Type=simple
User=root
Group=root

WorkingDirectory=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/users
ExecStart=/home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/users/dashboard

Restart=on-failure
RestartSec=10

ExecStartPre=/bin/mkdir -p /var/log/dashboard
ExecStartPre=/bin/chown syslog:adm /var/log/dashboard
ExecStartPre=/bin/chmod 775 /home/camila/Documents/ICOMP/SOII/Labs/2023---soii---laboratorio-6-camilacareggio/users

[Install]
WantedBy=multi-user.target
```

sudo systemctl daemon-reload  
sudo systemctl start dashboard   
sudo systemctl enable dashboard  

systemctl status dashboard 

## Hosts
sudo vim /etc/hosts  
```
127.0.0.1   localhost sensors.com dashboard.com
```