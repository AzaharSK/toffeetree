# Linux-D-Bus / libdbus IPC

D-Bus is a mechanism for interprocess communication under Linux , Developed by freedesktop.org

D-Bus has a layered architecture.
```
Highest Layer 3:  dbus-daemon                               - Message bus daemon
Middle  Layer 2:  libdbus                                   - C API
Lowest Layer  1:  D-Bus protocol ( Use socket mechanism)
 ```


```
~/toffeetree$ dbus-daemon --version
D-Bus Message Bus Daemon 1.12.16
Copyright (C) 2002, 2003 Red Hat, Inc., CodeFactory AB, and others
This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

 
* The most common usage of D-Bus is in form of messgae passing daemon           
  in between server -client processes using the **`$dbus-daemon`**  command

*  Processes can connect to a D-Bus daemon and exchange messages. 
*  There may be multiple D-Bus daemons running in the system at a time
*  There are two standard message bus instances:
        **`[1] system bus`** **`[2] Session bus`** - Each bus instance is managed by **`$dbus-daemon`** 
        
 * dbus-daemon is used for both of these instances but with a different configuration file.
* The systemwide daemon is normally launched by an init script, standardly called simply "messagebus".




### System-bus
* system D-BUS daemon for communication between the kernel, system-wide services and the user.   
* The systemwide message bus installed on many systems as the "messagebus" init service .       
* Dedicated to system sevices , catching and broadcasting system low level events such - 
  as connection to a network , Adding/Removing usb devices etc .

* Use the standard configuration file for the systemwide message bus.
  
  
  $ dbus-daemon --system 
  
  --system option is equivalent to "--config-file=/usr/local/share/dbus-1/system.conf"


### Session-Bus
session D-BUS daemon, primarily intended for communication between processes for the desktop applications of the logged-in user.



libdbus is a low level API from the freedesktop.org project. 
There are other higher level implementations, viz.,     
```
GDBus for Glibc     
QtDBus for Qt     
dbus-java       
sd-bus for the systemd software suite.
```


## Common Code :
A lot of the code is common no matter what you want to do on the Bus. 
First you need to connect to the bus. 
There is normally a system and a session bus. 
The DBUS config may restrict who can connect to the system bus. 

Secondly, you need to request a name on the bus. 
For simplicity I don't cope with the situation where someone already owns that name.


```
DBusError dbus_error;
DBusConnection *conn;
int ret;
   
   // initialise the errors   
dbus_error_init (&dbus_error);

// connect to the bus
conn = dbus_bus_get (DBUS_BUS_SESSION, &dbus_error);

 if (dbus_error_is_set (&dbus_error)) { 
          fprintf(stderr, "Connection Error (%s)\n", err.message); 
          dbus_error_free(&dbus_error); 
 }
 if (!conn)
       { exit (1); }
       
       
       ```
