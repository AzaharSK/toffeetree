#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>


void reply_to_method_call(DBusMessage* msg, DBusConnection* conn);

int main()
{

	DBusError dbus_error;
	DBusConnection* conn;
	int ret;

	//// initialise the errors ////////////////////////////////////////////////////
	dbus_error_init(&dbus_error);
	
	//// connect to the bus //////////////////////////////////////////////////////
	conn = dbus_bus_get(DBUS_BUS_SESSION, &dbus_error);

	if (dbus_error_is_set (&dbus_error)) { 
          fprintf(stderr, "Connection Error (%s)\n", dbus_error.message); 
          dbus_error_free(&dbus_error); 
	}
 	
	if (!conn) { exit (1); }

        //// Get a well known name //////////////////////////////////////////////////
	const char *const SERVER_BUS_NAME = "in.softprayog.add_server";
	ret = dbus_bus_request_name (conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);
	if (dbus_error_is_set (&dbus_error)){
          	fprintf(stderr, "Connection Error (%s)\n", dbus_error.message);
          	dbus_error_free(&dbus_error);
        }


   	 if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        	fprintf (stderr, "Dbus: not primary owner, ret = %d\n", ret);
        	exit (1);
    	}


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



	return 0;
}


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

/*********************************************************************************

toffeetree/dbus$ ./dbus-server 

Method called with hello

Method called with hey baby

Method called with ok

****************************************************************************/
