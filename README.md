hand-motion-tracker
===============

Introduction
---------------
Hand Motion Tracking Tool leverage Kinect for openFrameworks - in early alpha stage:

This is an enhancement to the original openFrameworks (version 073+) demonstrating effective hand tracking leveraging Kinect Sensor.
It depends on other addons, mainly ofxOpenCV and ofxOpenNI. In order to fully utilize its functionality, Kinect sensor and its drivers are required. Also, some sections utilize shaders, thereby, a decent graphics accelerator is necessary.
In-addition, there are sample demos provided to showcase the features and presentation. Currently, project files are only generated for MS VS but XCode project files will be added soon.

This tool originated from my academic project in Computer and Information Science at Ljubljana, Slovenia and is currently in an early alpha stage. Some areas of the code may be unstructured, repeated or not optimally written due to rapid development. However, improvements will continually be made and its functionality enhanced. The main challenge is improving the robustness of the hand tracking and its alignment with the kinematic model.

Keep updated with the latest improvements through the 0.8.1 branch.

Requirements
--------------
For a complete setup, you will need ofxOpenNI (install OpenNI, NITE) and SensorKinect drivers:
https://github.com/gameoverhack/ofxOpenNI <br/> https://github.com/avin2/SensorKinect

License
--------------
The code in this repository is available under the [CC BY-SA 2.5 License](http://creativecommons.org/licenses/by-sa/2.5/)

Images (to be updated in the future)
--------------
![ofxAddons Thumbnail](ofxaddons_thumbnail.png?raw=true 'ofxAddons thumbnail')