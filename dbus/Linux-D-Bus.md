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

## DBUS common operations:
Ex-1: Sending a Signal - Receiving a Signal.             
Ex-2: Calling a Method -Exposing a Method to be called -




## Common Code :
A lot of the code is common no matter what you want to do on the Bus. 
First you need to connect to the bus. 
There is normally a system and a session bus. 
The DBUS config may restrict who can connect to the system bus. 

Secondly, you need to request a name on the bus. 
For simplicity I don't cope with the situation where someone already owns that name.


```
#include <dbus/dbus.h>


DBusError dbus_error;
DBusConnection *conn;
int ret;
```


// initialise the errors   

```
dbus_error_init (&dbus_error);

```


// connect to the bus
```
conn = dbus_bus_get (DBUS_BUS_SESSION, &dbus_error);

 if (dbus_error_is_set (&dbus_error)) { 
          fprintf(stderr, "Connection Error (%s)\n", dbus_error.message); 
          dbus_error_free(&dbus_error); 
 }
 if (!conn)
       { exit (1); }
 ```      

 // Get a well known name
```
   const char *const SERVER_BUS_NAME = "in.softprayog.add_server";

   ret = dbus_bus_request_name (conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);

    if (dbus_error_is_set (&dbus_error)) {
          fprintf(stderr, "Connection Error (%s)\n", err.message); 
          dbus_error_free(&dbus_error);
          }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf (stderr, "Dbus: not primary owner, ret = %d\n", ret);
        exit (1);
    }

```
After you have finished with the bus you should close the connection. Note that this is a shared connection to the bus so you probably only want to do this just before the application terminates at main() END.

```
   dbus_connection_close(conn);
  
```

## Exposing a Method to be called

To expose a method which may be called by other DBUS applications you have to listen for messages as above,
then when you get a method call corresponding to the method you exposed you parse out the parameters, 
construct a reply message from the original and populate its parameters with the return value. 
Finally you have to send and free the reply.

```
	// loop, testing for new messages : Handle request from clients

while(true) {

		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);
		DBusMessage *msg = dbus_connection_pop_message(conn);


		// Loop again
		if (NULL == msg) {
        		sleep(1);
         		continue;
      		}

               //// Exposing a Method to be called //////////////////////////////////////////////////

		const char *const INTERFACE_NAME = "in.softprayog.dbus_example";
		const char *const METHOD_NAME = "add_numbers";

		// check this is a method call for the right interface and method
		if (dbus_message_is_method_call(msg, INTERFACE_NAME, METHOD_NAME))
         		reply_to_method_call(msg, conn);   //// Prepare reply to the method  /////////////////

		// free the message
      		dbus_message_unref(msg);
	}
   
   ```


### Prepare reply to the method


```

void reply_to_method_call(DBusMessage* msg, DBusConnection* conn)
{

	DBusMessage* reply;
   	DBusMessageIter args;

	char* param = "";
	char message[200] = "Hi client; you have send : "; 
	char *response_string;
      

	// read the arguments
   	if (!dbus_message_iter_init(msg, &args))
     		 fprintf(stderr, "Message has no arguments!\n");
   
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
      		fprintf(stderr, "Argument is not string!\n");
   	else
		dbus_message_iter_get_basic(&args, &param);
   		
	printf("Method called with %s\n", param);

        strcat(message, param);
        response_string=message;

	// create a reply from the message
   	reply = dbus_message_new_method_return(msg);


	// add the arguments to the reply
   	dbus_message_iter_init_append(reply, &args);


		
        if (!dbus_message_iter_append_basic (&args, DBUS_TYPE_STRING, &response_string)) {
                        fprintf (stderr, "Error in dbus_message_iter_append_basic\n");
                        exit (1);
                    }

	 // send the reply && flush the connection
   	if (!dbus_connection_send(conn, reply, NULL)) { 
      		fprintf(stderr, "Out Of Memory!\n"); 
      		exit(1);
   	}
   	
	dbus_connection_flush(conn);

	/// free the reply
   	dbus_message_unref(reply);
}

```

REF:
http://www.matthew.ath.cx/misc/dbus                             
http://www.matthew.ath.cx/projects.git/dbus-example.c
