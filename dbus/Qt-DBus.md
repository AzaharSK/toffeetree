#  Qt-DBus

* D-Bus is an Inter-Process Communication (IPC) and Remote Procedure Calling (RPC) mechanism originally developed for Linux 
* Intention was to replace existing and competing IPC solutions with one unified protocol. 

* It has also been designed to allow communication between system-level processes (such as printer and hardware driver services) and normal user processes.

* It uses a fast, binary message-passing protocol via libdbus (C Socket API), 
  which is suitable for same-machine communication due to its low latency and low overhead. 
  Its specification is currently defined by the freedesktop.org project, and is available to all parties.

* Communication in general happens through a central server application, called the "bus" (hence the name), 
  but direct application-to-application communication is also possible. 

* When communicating on a bus, applications can query which other applications and services are available, as well as activate one on demand.


# The BUS

## Where ?

* D-Bus buses are used to when many-to-many communication is desired.

## HOW ?
* In order to achieve that, a central server is launched before any applications can connect to the bus:   
  this server is responsible for keeping track of the applications that are connected and for properly routing messages from their source to their destination.
  i.e Sever app should be run first.
  
  
 *  D-Bus defines two well-known buses, called the system bus and the session bus. 
 
 * These buses are special in the sense that they have well-defined semantics: some services are defined to be found in one or both of these buses.

   For example, an application wishing to query the list of hardware devices attached to the computer will probably communicate to a service available on the system bus, while the service providing opening of the user's web browser will be probably found on the session bus.

On the system bus, one can also expect to find restrictions on what services each application is allowed to offer. Therefore, one can be reasonably certain that, if a certain service is present, it is being offered by a trusted application.





```
/////////// Server ////////////////////////////////////////////////////////////////////////////////////////
STEP [1] 

Lets say we want to provide service for two method to be exposed and called from client process 

 double multiplay(double x , double y);
 double divide(double x , double y)
 
STEP [2] Create a class of those method

class SlaveCalculator : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.blikoon.CalculatorInterface")
public:
    explicit SlaveCalculator(QObject *parent = nullptr);

signals:

public slots:
    double multiplay(double x , double y);
    double divide(double x , double y);

};

STEP [3] Implement those method return 
double SlaveCalculator::multiplay(double x, double y) { return x*y; }
double SlaveCalculator::divide(double x, double y) { return x/y; }


STEP [4] Generate slavecalculator.xml file out of class definition for Interfacing 
/home/suddin/Qt/5.15.2/gcc_64/bin/qdbuscpp2xml -M -s slavecalculator.h -o slavecalculator.xml

//Output:
slavecalculator.xml


STEP [5] Generate Dbus-Adapter code from that slavecalculator.xml

~/cpp_practices/QDbusClientServer/server$ /home/suddin/Qt/5.15.2/gcc_64/bin/qdbusxml2cpp -a calculatoradapter slavecalculator.xml 

//Output:
calculatoradapter.h
calculatoradapter.cpp



class CalculatorInterfaceAdaptor: public QDBusAbstractAdaptor
{

   // This class will have slavecalculator.xml content along with exposed method details , 
   // It handles client invoked methods and input arguments as well as return value
   //    QMetaObject::invokeMethod(parent(), "divide", Q_RETURN_ARG(double, out0), Q_ARG(double, x), Q_ARG(double, y));
   //    QMetaObject::invokeMethod(parent(), "multiplay", Q_RETURN_ARG(double, out0), Q_ARG(double, x), Q_ARG(double, y));

};



STEP #6  Server APP main() {

    Add  "calculatoradapter.h" class


    //Create Instance of server class & methods
    slaveCalculator = new SlaveCalculator(this);

    //Bound the slaveCalculator object with underlind DBus infrastructure
    new CalculatorInterfaceAdaptor(slaveCalculator);

    //Get sessionBus 
    QDBusConnection connection =  QDBusConnection::sessionBus();

    //Pass a Object you want to expose to DBUS
    connection.registerObject("/CalcServicePath", slaveCalculator);

    //Announce service name to other process
    connection.registerService( "com.blikoon.CalculatorService");
    
    
    }
    
    
/////////////////////// Client /////////////////////////////////////////////////////////////////////////////////////
cp slavecalculator.xml  ~/cpp_practices/QDbusClientServer/client/

/cpp_practices/QDbusClientServer/client$ /home/suddin/Qt/5.15.2/gcc_64/bin/qdbusxml2cpp -p calculatorinterface slavecalculator.xml 

-p  : for proxy interface file

Out:
calculatorinterface.h
calculatorinterface.cpp

//calculatorinterface.h   :  * Proxy class for interface com.blikoon.CalculatorInterface  

class ComBlikoonCalculatorInterfaceInterface: public QDBusAbstractInterface  //inherited from   public QDBusAbstractInterface
{

    const char *staticInterfaceName()
   { return "com.blikoon.CalculatorInterface"; }
   
   Also List exposed methods:
  inline QDBusPendingReply<double> divide(double x, double y) { }
  inline QDBusPendingReply<double> multiplay(double x, double y)

namespace com {
  namespace blikoon {
    typedef ::ComBlikoonCalculatorInterfaceInterface CalculatorInterface;
  }
  
  
 }
  
  




//Client Main() APP 
Add 

calculatorinterface.h
calculatorinterface.cpp

com::blikoon::CalculatorInterface* calcHandle = new com::blikoon::CalculatorInterface("com.blikoon.CalculatorService", "/CalcServicePath", QDBusConnection::sessionBus());


 if(calcHandle->isValid()) {
        
        calcHandle->Service_MyMethod(arg1 , arg2, ....argN);  //Server Service
}


```
  
