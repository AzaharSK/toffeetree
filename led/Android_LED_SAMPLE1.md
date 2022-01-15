# Android LED SAMPLE
          *Android FRAMEWORK
          *Android HAL C++
          *KERNEL changes
          
          
          
          

### Android FRAMEWORK 

frameworks/base/services/core/java/com/android/server/led/LedService.java
```
+led/LedService.java
```
frameworks/base/services/java/com/android/server/
```
SystemServer.java
```
frameworks/base/services/core/jni/
```
+com_android_server_led_LedService.cpp
 Android.mk 
 onload.cpp
```


### Android HAL C++

hardware/libhardware/modules

```
Android.mk
```


hardware/libhardware/modules/led/

```
+ led.cpp
+ Android.mk
+ CleanSpec.mk
```

hardware/libhardware/include/hardware/

```
+ led.h
```
device/OrganisationName/project/project.mk 

```
 PRODUCT_PACKAGES +=  
```

### KERNEL changes

device/qcom/common/rootdir/etc/init.qcom.rc





#                        				  FRAMEWORK CHANGES 						

frameworks/base/services/core/java/com/android/server/led/LedService.java

```
+led/LedService.java
```

frameworks/base/services/java/com/android/server/
```
SystemServer.java
```
frameworks/base/services/core/jni/ 

```
+com_android_server_led_LedService.cpp
 Android.mk 
 onload.cpp
```


LedService.java
```
// This file implements the LedService class, which provides one very basic method calls                             
public void setRed(int value) to the outside world ,		                                      		            

public void setRed(int value) 
{ 
      Log.d(TAG,"setRed: value="+value); 
      set_red(value);    
} 
```
As we can see this system service’s setRed() function apprently call a native function set_red() which is itself declared as follows in the LedService class:        
```
private static native int set_red(int value);                                                                                                       

```
frameworks/base/services/core/java/com/android/server/led/LedService.java

```
package com.android.server.led; 
import com.android.server.SystemService; 

public class LedService extends SystemService { 
 
    private ConnectivityManager mConnectivityManager; 
    private final Context mContext; 
    private static final String TAG = "LedService"; 
    private static final int LED_OFF = 0; 
    private static final int LED_ON = 1; 
    private static final int LED_FLASH = 2; 
    private int redFd; 
    private String red = "Red"; 
    public LedService(Context context) { 
        super(context); 
        if (DEBUG) { 
            Log.d(TAG, "LedService constructor"); 
        } 
        mContext = context; 

    /** 
     * Called when service is started by the main system service 
     */ 
    @Override 
    public void onStart() { 
        Log.d(TAG, "LedService onStart"); 
        init_native(); 
        mConnectivityManager = (ConnectivityManager)  			 	 	mContext.getSystemService(Context.CONNECTIVITY_SERVICE); 
        	mConnectivityManager.registerDefaultNetworkCallback(networkCallback);    
    } 
 
    public void setRed(int value) { 
      Log.d(TAG,"setRed: value="+value); 
      set_red(value); 
    } 

    private final ConnectivityManager.NetworkCallback networkCallback = new 	ConnectivityManager.NetworkCallback() { 
        @Override 
        public void onAvailable(Network network) { 
            super.onAvailable(network); 
            // this ternary operation is not quite true, because non-				metered doesn't yet mean, that it's wifi 
            // nevertheless, for simplicity let's assume that's true 
            Log.d(TAG, "mConnectivityManager.isActiveNetworkMetered()=" + 		  mConnectivityManager.isActiveNetworkMetered()); 
            
		  setRed(LED_OFF); 
              } 
 
        @Override 
        public void onLost(Network network) { 
            super.onLost(network); 
            Log.d(TAG, "losing active connection"); 
            setRed(LED_OFF); 
        } 
    }; 

    } 
 } 
    private static native int init_native(); 
    private static native void set_red(int value); 
} 

```
Note:

frameworks/base/services/java/com/android/server/SystemServer.java   
```
import com.android.server.pm.Installer; 
import com.android.server.pm.LauncherAppsService; 
...

+import com.android.server.led.LedService; 

 private void startOtherServices() 
{ 
  ...
 mSystemServiceManager.startService(DeviceStorageMonitorService.class); 
 ....
 +mSystemServiceManager.startService(LedService.class); 
 
```

By declaring the method as native , we instruct the compiler not to look for the method in any Java code. Instead, it’ll be provided to Dalvik at runtime through JNI. 

if you look at the frameworks/base/services/core/jni/ directory, 
you’ll notice that Android.mk and onload.cpp have been modified to take into account a new com_android_server_led_LedService.cpp 

The latter has a register_android_server_led_LedService() function which is called at the loading of libandroid_servers.so, which is itself
generated by the Android.mk . 


frameworks/base/services/core/jni/com_android_server_led_LedService.cpp

```
static JNINativeMethod method_table[] = 
{ 
    { "init_native", "()I", (void*)init_native }, 
    { "set_red", "(I)V", (void*)set_red }, 
}; 

```
The above structure contains three fields per method. 
The first field is the name of the method as defined in the LedService.java java class,

while the last field is the corresponding C/C++ imple‐mentation in the present file com_android_server_led_LedService.cpp

The Sececond Field say about argument and return type of C/C++ function ,
The content of the parentheses are the parameters passed from
Java, and the letter on the right of the parentheses is the return value. 

init_native() for instance takes no parameters and returns an integer, 
while det_red() take an int(I) parameter and returns void(V) type.


That registration function tells Dalvik about the native methods implemented in com_android_server_led_LedService.cpp for
the LedService  class and how they can be called

```
int register_android_server_led_LedService(JNIEnv *env) 
{ 
    returnjni RegisterNativeMethods(env,"com/android/server/led/LedService", 
    method_table, NELEM(method_table)); 
} 

```
frameworks/base/services/core/jni/com_android_server_led_LedService.cpp

```
#include "jni.h" 
#include "JNIHelp.h" 
#include "android_runtime/AndroidRuntime.h" 
#include <utils/misc.h> 
#include <utils/Log.h> 
#include <stdio.h>


+#include <hardware/hardware.h> 
+#include <hardware/led.h> 



namespace android 
{ 
struct hw_module_t *ledModule = NULL; 
struct led_device_t *ledDevice = NULL; 

/** 
 * JNI Layer init function; 
 * Load HAL and its method table for later use; 
 * return a pointer to the same structure 
 * (not mandatory btw, we can use a global variable) 
 */ 
static jint init_native(JNIEnv *env, jobject clazz) 
{ 
    int err, fd; 
    struct hw_module_t *module; 
    struct hw_device_t *device; 

    // Load the HAL module using libhardware hw_get_module 
    err = hw_get_module(LED_HARDWARE_MODULE_ID, (hw_module_t const**)&module); 
    if (err) { 
        ALOGE("Error while gettting led module \n"); 
        return -1; 
    } 
   
  fd = module->methods->open(module, "LED", &device); 
    if (fd < 0) { 
        ALOGE("Error while opening Led \n"); 
        return -1; 
   } 

    

    ledDevice =  (struct led_device_t *)device; 
   
    return fd; 
} 


static void set_red(JNIEnv *env, jobject clazz, int value){ 
    ALOGD("***set_red****\n"); 
    int result = ledDevice->set_red(value); 
   ALOGD("result=%d",result); 
} 


static JNINativeMethod method_table[] = 
{ 
    { "init_native", "()I", (void*)init_native }, 
    { "set_red", "(I)V", (void*)set_red }, 
}; 

int register_android_server_led_LedService(JNIEnv *env) 
{ 
   returnjni RegisterNativeMethods(env,"com/android/server/led/LedService", 
    method_table, NELEM(method_table)); 
} 
}; 

```

First, notice that there are two (JNIEnv *env and jobject clazz) or more parameters than in the JNI declaration above. All JNI’ed calls start with the same two parameters: a handle to the VM making the call( env ), and the this object corresponding to the class making the call ( clazz ).

Then First, the call to hw_get_module() which requests that the HAL load the module that implements support for the LED_HARDWARE_MODULE_ID type of hardware. 

```
struct hw_module_t *module; 
int err = hw_get_module(LED_HARDWARE_MODULE_ID, (hw_module_t
const**)&module); 

```
Second, there’s the call to the loaded module’s open() function.By Which
we got device handle to hardware

```
struct hw_device_t *device; 
int fd = module->methods->open(module, "LED", &device);
struct led_device_t *ledDevice  =  (struct led_device_t *)device;  
```
former will result in a .so being loaded into the system service’s address
space, and the latter will result in the hardware-specific functions implemented in that library’s functions, such as set_red() being callable from 

com_android_server_led_LedService.cpp,

```
static void set_red(JNIEnv *env, jobject clazz, int value)
{ 
  
    int result = ledDevice->set_red(value); 
} 

```
Now modify Android.mk and onload.cpp in frameworks/base/services/core/jni/ 
directory to take into account a new com_android_server_led_LedService.cpp


frameworks/base/services/core/jni/Android.mk
```

LOCAL_SRC_FILES += \ 
    $(LOCAL_REL_DIR)/com_android_server_AlarmManagerService.cpp \ 
    $(LOCAL_REL_DIR)/com_android_server_am_BatteryStatsService.cpp \ 
    ...
    +$(LOCAL_REL_DIR)/com_android_server_led_LedService.cpp \ 
    $(LOCAL_REL_DIR)/onload.cpp 

```
frameworks/base/services/core/jni/onload.cpp

```
namespace android { 
int register_android_server_ActivityManagerService(JNIEnv* env); 
int register_android_server_AlarmManagerService(JNIEnv* env); 
...
+int register_android_server_led_LedService(JNIEnv* env); 

};

```

```
extern "C" jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) 
{ 
    JNIEnv* env = NULL; 
    jint result = -1; 

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) { 
        ALOGE("GetEnv failed!"); 
        return result; 
    } 
    ALOG_ASSERT(env, "Could not retrieve the env!"); 

    register_android_server_ActivityManagerService(env); 
    register_android_server_PowerManagerService(env); 
    ...
    +register_android_server_led_LedService(env); 

    return JNI_VERSION_1_4; 
} 

```
 

 ##                                                               HAL  CHANGES                                                               


Basic Skeliton for android HAL 

#ls hardware/libhardware/modules/
```
Android.mk	     
camera	 
fingerprint  	  
input_sub   
peripheral       
sensors
...	   
```
Create led folder in hardware/libhardware/modules/
```
mkdir -p hardware/libhardware/modules/led
```
hardware/libhardware/include/hardware/
```
+ led.h

```


hardware/libhardware/modules/led/
```
+ led.cpp
+ Android.mk
+ CleanSpec.mk
```
hardware/libhardware/modules/
```
Android.mk 
```
The HAL, which is in hardware/, provides the hw_get_module() call above. And if you follow the code, you’ll see that hw_get_module() ends up calling the classic dlopen() ,which enables us to load a shared library into a process’s address space.

The HAL won’t, however, just load any shared library. When you request a given hard‐ware type, it’ll look in /system/lib/hw for a filename that matches that given hardware type and the device it’s running on.

So, for instance, in the case of the present new typeof hardware, it’ll look for led.msm8953.so, The actual name of the device used for the middle part of the filename is retrieved from one of the following global properties: ro.hardware , ro.prod uct.board , ro.board.platform , or ro.arch . Also, the shared library must have a struct that provides HAL information and that is called HAL_MODULE_INFO_SYM_AS_STR . 

We’ll see an example next. The definition for the new hardware type itself is just another header file, in this case led.h, along with the other hardware definitions in hardware/libhardware/include/hardware/:


hardware/libhardware/include/hardware/led.h 
```
+#ifndef ANDROID_LED_H 
+#define ANDROID_LED_H 

#include <stdint.h> 
#include <sys/cdefs.h> 
#include <sys/types.h> 
#include <hardware/hardware.h> 

__BEGIN_DECLS 
+#define LED_HARDWARE_MODULE_ID "led" 

struct led_device_t 
{ 
  /* Will be used in HAL open method */ 
  struct hw_device_t common; 

+  int (*set_red)(int value); 

}; 

__END_DECLS 
+#endif //ANDROID_LED_H 
```
In addition to the prototype definitions for set_red() , note that this is where LED_HARDWARE_MODULE_ID is defined. The latter serves as the basis for the file‐name looked for on the filesystem that contains the actual HAL module implementation.

Typ‐ically, the HAL modules are added to the AOSP sources in the lib* directory within device/<vendor>/<product>/.

  ```
device/OrganisationName/project/project.mk 
 PRODUCT_PACKAGES +=  \ 
     uart.msm8953 \ 
     gpio.msm8953 \ 
+    led.msm8953 \

  ```
In order for the library resulting from the build of this file to be recognized as a real HAL module, it ends with this snippet:

hardware/libhardware/modules/led/led.cpp

```
static struct hw_module_methods_t led_methods = 
{ 
  .open =  open_led, 
}; 

struct hw_module_t HAL_MODULE_INFO_SYM = 
{ 
  .tag = HARDWARE_MODULE_TAG, 
  .version_major = 0, 
  .version_minor = 1, 
  .id = LED_HARDWARE_MODULE_ID, 
  .name = "LED HAL", 
  .author = "OrganisationName", 
  .methods = &led_methods, 
}; 

```

Note the presence of the structure called HAL_MODULE_INFO_SYM . Furthermore, note the led_methods and the open() function pointer it contains. This is the very same open() called by init_native() earlier once the HAL module is loaded. And here’s what the corresponding open_led() does

```

/** 
* A pointer to this method is stored in 
* hw_module_method_t.open; 
* 
* Once JNI loads the hw_module_method_t symbol, it 
* can call this function and "open" the HAL layer 
* receiving pointers to this module's additional methods 
**/

+#include <hardware/led.h>

+class PThread* redLED; 		//Global pointer declaration

+static int open_led(const struct hw_module_t *module, char const *name, struct hw_device_t **device)
 
+int set_red(int value);

+int close_led(hw_device_t* device) 



static int open_led(const struct hw_module_t *module, 
                       char const *name, struct hw_device_t **device) 
{ 
    

    struct led_device_t *dev = (struct led_device_t *)   malloc(sizeof(*dev)); 

    if (NULL == dev) { 
        return -ENOMEM; 
    } 
   
    +dev->set_red = set_red;     //Method to set RED LED state on/off/blink

    /* Initialize common fields */ 
    dev->common.tag = HARDWARE_DEVICE_TAG; 
    dev->common.version = 0; 
    dev->common.module = (struct hw_module_t *)module; 
    +dev->common.close = close_led; 

    /* Store this module's structure in the output parameter 'device' */ 
    /* Remember the first field of your HAL device must be an hw_device_t */ 
    *device = (struct hw_device_t *)dev; 

    +redLED   =new Pthread(RED_LED_PATH); 

    return 0; 
} 

/** This is mandatory, and part of hw_device_t */ 

int close_led(hw_device_t* device) 
{ 
    ALOGD("close_led: free device"); 
    +delete redLED;                       //Deallocate object and threads. 
    free(device); 
    return 0; 
} 

```
This led_open() function’s main purpose is to initialize the dev struct, which is of struct led_device_t type, the same type defined by led.h,
And open the corresponding device entry in /dev, thereby connecting to the underlying device driver loaded into the kernel.
Obviously some device drivers might require some initialization here, but for our purposes ,this is sufficient

  
  ### setRed() API call flow At a Galance

The system service’s setRed() call results in a JNI call to set_red() which, by way of the HAL, results in a call to the HAL module’ set_red() .

  ```
JAVA API:
public void setRed(int value) 
{  
 
      set_red(value); 
} 

  ```

  NATIVE CALL:

```
  private static native void set_red(int value);

```
JNI:

  ```
static void set_red(JNIEnv *env, jobject clazz, int value)
{ 
  
    int result = ledDevice->set_red(value); 
}

{ "set_red", "(I)V", (void*)set_red },

  
//C/C++ HAL:
dev->set_red = set_red;

  ```
Implementation:

  ```
class Pthread*   redLED=new Pthread( "/sys/class/gpio/gpio43/value" ); 

int set_red(int value) { 
 
    ALOGD("set_red value=%d :",value); 
    if(!redLED) return -1; 
 
    if(value == 0) { 
        // Make LED OFF 
        redLED->suspendBlink(); 
        redLED->ledOFF(); 
 
    } else if (value == 1) { 
        // Make LED ON 
        redLED->suspendBlink(); 
       redLED->ledON(); 
 
    } else if (value == 2) { 
        // Make LED FLASHING 
          redLED->resumeBlink(); 
   } 
  
    return 1; 
} 
  
  ```
========================================================================  
```

+#define on 1 
+#define off 0 

+#define RED_LED_PATH            "/sys/class/gpio/gpio43/value" 

+class PThread; 
+PThread* redLED; 

+class PThread { 
+ 
+    private: 
+    pthread_t threadID; 
+    pthread_mutex_t lock; 
+ 
+    public: 
+    const char * sysfs_path; 
+    volatile int suspended; 
+ 
+    PThread(const char* path) : sysfs_path(path) 
+    {   
+        this->suspended =0; 
+        pthread_mutex_init(&this->lock,NULL); 
+        pthread_create(&this->threadID, NULL, led_Flash, (void*)this ); 
+        
+    } 
+    ~PThread() 
+    { 
+      pthread_join(this->threadID , NULL); 
+      pthread_mutex_destroy(&this->lock); 
+    } 
+ 
+    void suspendBlink() { 
+        ALOGD("suspendBlink"); 
+        pthread_mutex_lock(&this->lock); 
+        this->suspended = 1; 
+        pthread_mutex_unlock(&this->lock); 
+    } 
+ 
+    void resumeBlink() { 
+        ALOGD("resumeBlink"); 
+        pthread_mutex_lock(&this->lock); 
+        this->suspended = 0; 
+        pthread_mutex_unlock(&this->lock); 
+    } 
+ 
+    void ledON(){ 
+        ALOGD("ledON"); 
+        ledGPIO_write(this->sysfs_path,on); 
+    } 
+ 
+    void ledOFF(){ 
+        ALOGD("ledOFF"); 
+        ledGPIO_write(this->sysfs_path,off); 
+    } 
+    
+}; 
+ 


+void* led_Flash(void* args) { 
+    PThread* pt= (PThread*) args; 
+    int val = on; 
+    while(1) {   
+        if(!(pt->suspended)) { 
+            pthread_mutex_lock(&mutex); 
+            val = (val == off ? on : off); 
+            ledGPIO_write(pt->sysfs_path, val); 
+            usleep(100000); 
+            pthread_mutex_unlock(&mutex); 
+        } 
+   } 
+   return NULL; 
+} 
+ 


+void ledGPIO_write(const char *buf, int value) { 
+    FILE *fp; 
+    fp = fopen(buf , "w"); 
+    if(!fp) { 
+       ALOGE("ledGPIO_write Unable to access GPIO %s \n", buf); 
+       return; 
+    } 
+    fprintf(fp, value?"1":"0"); 
+    fclose(fp); 
+} 
+ 



+ 
+int set_red(int value) { 
+ 
+    ALOGD("set_red value=%d :",value); 
+    if(!redLED) return -1; 
+ 
+    if(value == 0) { 
+        // Make LED OFF 
+        redLED->suspendBlink(); 
+        redLED->ledOFF(); 
+ 
+    } else if (value == 1) { 
+        // Make LED ON 
+        redLED->suspendBlink(); 
+        redLED->ledON(); 
+ 
+    } else if (value == 2) { 
+        // Make LED FLASHING 
+          redLED->resumeBlink(); 
+    } 
+  
+    return 1; 
+} 

```
hardware/libhardware/modules/led/Android.mk

```
+LOCAL_PATH:= $(call my-dir) 
+include $(CLEAR_VARS) 
+ 
+LOCAL_SRC_FILES := led.cpp 
+LOCAL_MODULE_RELATIVE_PATH := hw 
+LOCAL_MODULE := led.$(TARGET_BOARD_PLATFORM) 
+LOCAL_MODULE_TAGS := optional 
+LOCAL_SHARED_LIBRARIES := liblog libcutils 
+include $(BUILD_SHARED_LIBRARY) 
```
  
hardware/libhardware/modules/led/CleanSpec.mk

  ```
+# ************************************************ 
+# NEWER CLEAN STEPS MUST BE AT THE END OF THE LIST 
+# ************************************************ 

 
+$(call add-clean-step, rm -rf (PRODUCT_OUT)/obj/SHARED_LIBRARIES/libMcClient_intermediates) 

```

hardware/libhardware/modules/Android.mk  

  
  ```
hardware_modules := gralloc hwcomposer audio nfc nfc-nci local_time \ 
power usbaudio audio_remote_submix camera usbcamera consumerir \
peripheral sensors uart vibrator \ 
+  tv_input fingerprint input vehicle thermal vr gps gpio pwm pwmservice led 

 include $(call all-named-subdir-makefiles,$(hardware_modules)) 


```










  ##             				     KERNEL CHANNGES     			                    


device/qcom/common/rootdir/etc/init.qcom.rc

  ```
+    # Red color LED 
+    write /sys/class/gpio/export 43 
+    write /sys/class/gpio/gpio43/direction out 
+    chmod 0666 /sys/class/gpio/gpio43/value 
+    chown system.system /sys/class/gpio/gpio43/value 

  ```

Test:
  
  ```
#ls -l /sys/class/gpio/ | grep 43
gpio43

#cd /sys/class/gpio/
#cat gpio43/direction 
out

#cat gpio43/value
0

# echo 1 > gpio43/value       

#cat gpio43/value
1 	[*Check board Glowing RED LED *]

```