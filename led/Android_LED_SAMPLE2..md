


1 Till device boot Completes Red – Flashing, Amber- Flashing, Green – Flashing.

2 Normal Operation without LTE Red – OFF, Amber- OFF, Green – Flashing.

3 Normal Operation with LTE Connected adb shell ifconfig Red – OFF, Amber- OFF, Green – ON.

4 Download data/firmware. adb shell am startservice < test_app_package_name>/.serviceName Red – OFF, Amber- Flashing, Green – OFF.
 
5 Purposefully do a Boot Error. Red – ON, Amber- OFF, Green – OFF.
 
 ```
uses-permission android:name="android.permission.RECEIVE_BOOT_COMPLETED"
action android:name="android.intent.action.BOOT_COMPLETED"



GPIOx3 at  our board

GPIO_43  LED_RED_EN 
GPIO_46  LED_GREEN_EN
GPIO_53  LED_AMBER_EN

write /sys/class/gpio/export 43
write /sys/class/gpio/gpio43/direction out
chmod 777 /sys/class/gpio/gpio43/value


write /sys/class/gpio/export 46
write /sys/class/gpio/gpio46/direction out
chmod 777 /sys/class/gpio/gpio46/value


write /sys/class/gpio/export 53
write /sys/class/gpio/gpio53/direction out
chmod 777 /sys/class/gpio/gpio53/value

#define RED_LED_PATH            "/sys/class/gpio/gpio43/value"

fd = open(RED_LED_PATH, O_RDWR);
snprintf(buffer, sizeof(int), "%d\n", 255);
snprintf(buffer, sizeof(int), "%d\n", 0);
write(fd, buffer, strlen(buffer));

```

### INPUT connection to APQ



 Tri-color LED are working in charging mode also. Below are tri-color requirements,
    
     Solid Green -> Indicates fully charged as long as the cable is connected
     Solid Red -> Charging under low battery , i.e battery level <=14%
     Solid Yellow --> Charging from 15% to 99%



kernel/arch/arm/boot/dts/qcom/msm8916-qrd-skut1.dtsi

```
gpio-leds {
                compatible = "gpio-leds";
                status = "okay";
                pinctrl-names = "default";
                pinctrl-0 = <&gpio_led_off>;

                red {
                        gpios = <&msm_gpio 28 0>;
                        label = "red";
                        linux,default-trigger = "none";
                        default-state = "off";
                        retain-state-suspended;
                };

                green {
                        gpios = <&msm_gpio 118 0>;
                        label = "green";
                        linux,default-trigger = "none";
                        default-state = "off";
                        retain-state-suspended;
                };
                blue {
                        gpios = <&msm_gpio 117 0>;
                        label = "blue";
                        linux,default-trigger = "none";
                        default-state = "off";
                        retain-state-suspended;
                };
        };



 gpio_led_pins {
                        qcom,pins = <&gp 8>, <&gp 9>, <&gp 10>;
                        qcom,num-grp-pins = <3>;
                        label = "gpio-led-pins";
                        gpio_led_off: led_off {
                                drive-strength = <2>;
                                bias-disable;
                                output-low;
                        };
                };


leds {

compatible = "gpio-leds";

led@1 {
label = "red";
gpios = <&gpio0 26 GPIO_ACTIVE_HIGH>;
default-state = "keep";
};

led@2 {
label = "blue";
gpios = <&gpio0 23 GPIO_ACTIVE_HIGH>;
default-state = "keep";
};

led@3 {
label = "green";
gpios = <&gpio0 22 GPIO_ACTIVE_HIGH>;
default-state = "keep";
};

}

```

kernel/drivers/leds/leds-gpio.c

```
static const struct of_device_id of_gpio_leds_match[] = {
        { .compatible = "gpio-leds", },
        {},
};

static struct platform_driver gpio_led_driver = {
        .probe          = gpio_led_probe,
        .remove         = gpio_led_remove,
        .driver         = {
                .name   = "leds-gpio",
                .owner  = THIS_MODULE,
                .of_match_table = of_match_ptr(of_gpio_leds_match),
        },
};


 led.gpio = of_get_gpio_flags(child, 0, &flags);
 led.active_low = flags & OF_GPIO_ACTIVE_LOW;
 led.name = of_get_property(child, "label", NULL) ? : child->name;
 led.default_trigger =of_get_property(child, "linux,default-trigger", NULL);
 state = of_get_property(child, "default-state", NULL);
   if (!strcmp(state, "keep"))
                                led.default_state = LEDS_GPIO_DEFSTATE_KEEP;
                        else if (!strcmp(state, "on"))
                                led.default_state = LEDS_GPIO_DEFSTATE_ON;
                        else
                                led.default_state = LEDS_GPIO_DEFSTATE_OFF;


 gpio_set_value(led_dat->gpio, level);

```

```
led_classdev (Userspace export)
kernel/drivers/leds/led-class.c


        __ATTR(brightness, 0644, led_brightness_show, led_brightness_store),
        __ATTR(max_brightness, 0644, led_max_brightness_show,
                        led_max_brightness_store),
      
#ifdef CONFIG_LEDS_TRIGGERS
        __ATTR(trigger, 0644, led_trigger_show, led_trigger_store),
#endif

```
```
system/core/healthd$ vim healthd_mode_charger.cpp 

#define RED_LED_PATH            "/sys/class/leds/red/brightness"
#define GREEN_LED_PATH          "/sys/class/leds/green/brightness"
#define BLUE_LED_PATH           "/sys/class/leds/blue/brightness"


 healthd_mode_charger_heartbeat()
    --> handle_power_supply_state(charger, now);
           ---> 
              soc = get_battery_capacity(charger);
   		 if (old_soc != soc) {
  	         set_battery_soc_leds(soc); }
                   ---- > 
                            set_tricolor_led(1, color);
                                       ----> on=1
                                              fd = open("/sys/class/leds/red/brightness", O_RDWR);
                                                
                                               if (on)
               				          snprintf(buffer, sizeof(int), "%d\n", 255);  //buffer=255
            				       else
                				snprintf(buffer, sizeof(int), "%d\n", 0);
            				        write(fd, buffer, strlen(buffer));            // write to fd


    


```

### Framework

-- frameworks/base/services/core/java/com/android/server/BatteryService.java

```
setColor(mBatteryLowARGB);

 public final class BatteryService extends SystemService {
                     com.android.internal.R.integer.config_notificationsBatteryLedOff);
         }

+        private class MyThread extends Thread {
+            public void run() {
+                while(isBlinking) {
+                    mWakeLock.acquire();
+	            mBatteryLight.setColor(mBatteryLowARGB);
+              	    try {
+	                Thread.sleep(200);
+ 	            } catch (InterruptedException e) { };
+	            mBatteryLight.turnOff();
+                    mWakeLock.release();
+                    if(!isBlinking)
+                        break;
+              	    try {
+	                Thread.sleep(2000);
+ 	            } catch (InterruptedException e) { };
+                }
+            }



) {
+
+            } else if((status == BatteryManager.BATTERY_STATUS_FULL) && (mBatteryProps.chargerAcOnline || 
+				mBatteryProps.chargerUsbOnline || mBatteryProps.chargerWirelessOnline) ) {
+
                     // 
+        };
+        private MyThread myThread;
+ 


```

https://stackoverflow.com/questions/44930443/how-are-the-steps-to-access-gpios-in-linux-kernel-modules



1 Till device boot Completes Red – Flashing, Amber- Flashing, Green – Flashing.

2 Normal Operation without LTE Red – OFF, Amber- OFF, Green – Flashing.

3 Normal Operation with LTE Connected adb shell ifconfig Red – OFF, Amber- OFF, Green – ON.

4 Download data/firmware. adb shell am startservice < test_app_package_name>/.serviceName Red – OFF, Amber- Flashing, Green – OFF.
 
5 Purposefully do a Boot Error. Red – ON, Amber- OFF, Green – OFF.
 
```
/home/azahar/myboard/frameworks/base/services/core/java/com/android/server/
uses-permission android:name="android.permission.RECEIVE_BOOT_COMPLETED"
action android:name="android.intent.action.BOOT_COMPLETED"

```

### [2] & [3] Use ConnectivityManager for LTE



https://stackoverflow.com/questions/25581912/how-to-monitor-network-type-and-get-notified-if-it-changes
https://stackoverflow.com/questions/13735100/is-there-a-way-to-detect-which-cellular-network-is-used-on-android/21721139#21721139

```
    TelephonyManager TelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
    TelephonyMgr.listen(new TeleListener(),
            PhoneStateListener.LISTEN_DATA_CONNECTION_STATE)

    public class TeleListener extends PhoneStateListener {  
    public void onDataConnectionStateChanged (int state, int networkType){
        super.onDataConnectionStateChanged(state, networkType);  
   //Whatever you wanna do  
      }  
}

```
the method onDataConnectionStateChanged(int state, int networkType) gets triggered when either networkstype(2G,3G,4G) changes or the connection state (Connecting, disconnecting etc) change

```
 // It Only works on Android 4.2+
    TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
    telephonyManager.listen(new PhoneStateListener(){

        @Override
        public void onCellInfoChanged(List<CellInfo> cellInfo) {
            super.onCellInfoChanged(cellInfo);
            for (CellInfo ci : cellInfo) {
               
                 if (ci instanceof CellInfoLte) {
                    Log.d("TAG", "This has LTE");
                }
            }
        }


    }, PhoneStateListener.LISTEN_CELL_INFO);

```
https://stackoverflow.com/questions/21697765/how-to-get-android-internet-connection-status?noredirect=1&lq=1
https://stackoverflow.com/questions/4238921/detect-whether-there-is-an-internet-connection-available-on-android

The getActiveNetworkInfo() method of ConnectivityManager returns a NetworkInfo instance representing the first connected network interface it can find or null if none of the interfaces are connected. Checking if this method returns null should be enough to tell if an internet connection is available or not.

```
private boolean isNetworkAvailable() {
    ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
    return activeNetworkInfo != null && activeNetworkInfo.isConnected();
}

```
You will also need: in your android manifest.

```
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
```


Edit:

Note that having an active network interface doesn't guarantee that a particular networked service is available. Network issues, server downtime, low signal, captive portals, content filters and the like can all prevent your app from reaching a server. For instance you can't tell for sure if your app can reach Twitter until you receive a valid response from the Twitter service.

https://www.tutorialspoint.com/android/android_network_connection.htm

```
[4]use DownloadManager
=======================================================
        STATUS_RUNNING
	ACTION_DOWNLOAD_COMPLETE
	STATUS_SUCCESSFUL

DownloadManager.Query query = null;
    Cursor c = null;
    DownloadManager downloadManager = null;
    downloadManager = (DownloadManager)getSystemService(Context.DOWNLOAD_SERVICE);
    query = new DownloadManager.Query();
     if(query!=null) {
                query.setFilterByStatus(DownloadManager.STATUS_FAILED|DownloadManager.STATUS_PAUSED|DownloadManager.STATUS_SUCCESSFUL|
                        DownloadManager.STATUS_RUNNING|DownloadManager.STATUS_PENDING);
            } else {
                return;
            }
    c = downloadManager.query(query);
    if(c.moveToFirst()) { 
    int status = c.getInt(c.getColumnIndex(DownloadManager.COLUMN_STATUS)); 
    switch(status) { 
    case DownloadManager.STATUS_PAUSED: 
    break; 
    case DownloadManager.STATUS_PENDING: 
    break; 
    case DownloadManager.STATUS_RUNNING: 
    
    break; 
    case DownloadManager.STATUS_SUCCESSFUL: 
    break; 
    case DownloadManager.STATUS_FAILED: 
    break; 
    }
}
```
