<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
<?php include("../site-headermeta.htm"); ?>
<meta name="Title" content="Raspberry Pi Weather Station" />
<meta name="Description" content="Raspberry Pi Weather Station in Tokyo, Setagaya-ku, Futako-Tamagawa" />
<meta name="Keywords" content="Raspberri Pi, Weather Station, Tokyo" />
<meta name="Classification" content="Weather Station" />
<title>Raspberry Pi Weather Station</title>
</head>

<body>
<div id="wrapper">
<div id="banner">
<h1>Raspberry Pi Weather Station</h1>
<h2>Local weather data for Tokyo, Setagaya-ku, Futako-Tamagawa Station</h2>
</div>
<?php include("../site-navigation.htm"); ?>

<div id="content">
<div class="copyright">Frank4DD, @2016</div>
<h3>Raspberry Pi Camera View:</h3>
<hr />
<div class="weatherpic">
   <table align="center" valign="center">
   <tr><td>
      <img src="images/raspicam.jpg" alt="raspi weather camera" />
   </td></tr>
   </table>
</div>

<h3>Raspberry Pi Sensor Data: <?php
    if(file_exists("/opt/raspi/data/sensor.txt")) {
       $output = "Last update received on ".date("Y-m-d H:i:s",filectime("/opt/raspi/data/sensor.txt"));
    }
    else {
        $output = "No data submission at ".date("Y-m-d H:i:s");
    }
    echo $output ?></h3>
    <hr />
    <div class="sensorgraph">
    <img src="images/am2302_temp.png" alt="Temperature Graph">
    </div>
    <div class="sensordata">
     Air Temperature:
     <span class="sensorvalue">
     <?php $raspidata = file_get_contents("/opt/raspi/data/sensor.txt");
      // Example sensor result: #1472511049 Temp=25.9*C  Humidity=54.0%
      $values = preg_split('/ +/', $raspidata);
      $temp = substr($values[1], 5);
      echo str_replace("*C","&deg;C", $temp); ?></span>
    </div>
    <p></p>
    <div class="sensorgraph">
    <img src="images/am2302_humi.png" alt="Humidity Graph">
    </div>
    <div class="sensordata">
     Relative Humidity:
     <span class="sensorvalue">
     <?php $raspidata = file_get_contents("/opt/raspi/data/sensor.txt");
      // Example sensor result: #Temp=25.9*C  Humidity=54.0%
      $values = preg_split('/ +/', $raspidata);
      $humi = substr($values[2], 9);
      echo $humi; ?></span>
    </div>
    <p></p>
    <div class="sensorgraph">
    <img src="images/am2302_real.png" alt="RealFeel Graph">
    </div>
    <div class="sensordata">
     Temperature Humidity Index:
     <span class="sensorvalue">
     <?php $raspidata = file_get_contents("/opt/raspi/data/sensor.txt");
      // Example sensor result: #Temp=25.9*C  Humidity=54.0%
      $values = preg_split('/ +/', $raspidata);
      $temp = substr($values[1], 5);
      $temp = str_replace("*C","", $temp);
      $values = preg_split('/ +/', $raspidata);
      $humi = substr($values[2], 9);
      if($temp && $humi) {
        // convert degree Celsius into Fahrenheit
        $tempF = ($temp * 9/5) + 32;
        // THI calculation constants
        $c1= -42.379;   $c2= 2.0490152;  $c3= 10.1433312;
        $c4= -0.224755; $c5= -0.0068378; $c6= -0.05481717;
        $c7= 0.001228;  $c8= 0.0008528;  $c9= -0.00000199;
        // ------------------------------------------------------------------------------
        // Formula: https://en.wikipedia.org/wiki/Heat_index
        // HI = c1 + c2*T + c3*R + c4*T*R + c5*T^2 + c6*R^2 + c7*T^2*R + c8*T*R^2 + c9*T^2*R^2 
        // ------------------------------------------------------------------------------
        $realF = $c1 + ($c2 * $tempF) + ($c3 * $humi) + 
                 ($c4 * $tempF * $humi) + ($c5 * $tempF * $tempF) +
                 ($c6 * $humi * $humi) + ($c7 * $tempF * $tempF * $humi) +
                 ($c8 * $tempF * $humi * $humi) + ($c9 * $tempF * $tempF * $humi * $humi);
        // ------------------------------------------------------------------------------
        // per http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
        // two adjustments are necessary to account for extreme high and low humidity 
        // when Rothfusz regression is used. (Rothfusz regression is not valid for 
        // temperature and relative humidity beyond the range of data considered by Steadman. 
        // ------------------------------------------------------------------------------
        // adjustment for very low humidity: ADJ = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17} 
        if($humi < 13 && $tempF < 110) {
           $adj  = ((13-$humi)/4) * sqrt((17-ABS($tempF-95))/17);
           $realF = $realF - $adj;
        }
        // adjustment for very high humidity: ADJ = [(RH-85)/10] * [(87-T)/5] 
        if($humi > 85 && $tempF < 87) {
           $adj  = (($humi-85)/10) * ((87-$tempF)/5);
           $realF = $realF + $adj;
        }
        // convert result back to degree Celsius
        $real = (($realF - 32)/1.8);
    }
    printf("%3.1f&deg;C", $real); ?></span>
    </div>
    <p>Data provided by AM2302 sensor and RasPi camera, uploaded in one minute intervals.</p>
    <h3>Weekly View:</h3>
    <hr />
    <div class="fullgraph">
    <img src="images/monthly_temp.png" alt="Temperature Graph">
    </div>
    <div class="fullgraph">
    <img src="images/monthly_humi.png" alt="Humidity Graph">
    </div>
    <div class="fullgraph">
    <img src="images/monthly_real.png" alt="RealFeel Graph">
    </div>
    <h3>Yearly View:</h3>
    <hr />
    <div class="fullgraph">
    <img src="images/yearly_temp.png" alt="Temperature Graph">
    </div>
    <div class="fullgraph">
    <img src="images/yearly_humi.png" alt="Humidity Graph">
    </div>
    <div class="fullgraph">
    <img src="images/yearly_real.png" alt="RealFeel Graph">
    </div>
  </div>

  <div id="sidecontent">
    <p>
    <script type="text/javascript"><!--
      google_ad_client = "pub-6688183504093504";
      /* Virtual Server Hosting 120x600 */
      google_ad_slot = "7506816809";
      google_ad_width = 120;
      google_ad_height = 600;
      //-->
    </script>
    <script type="text/javascript"
      src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
    </script>
    </p>
  </div>
<?php include("../site-footerdata.htm"); ?>
</div>
</body>
</html>
