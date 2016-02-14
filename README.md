# H5RtmpClient
this project's goal is to play live rtmp video in Html5
# What you need to compile and run
csharprtmp(my another project)、Broadway、Speex.js
all these projects are on github

you need to modify js/index.html to connect to right server address

you can publish video by use testClient.swf,or you can write a flash client by yourself.

    var h264Settings:H264VideoStreamSettings = new H264VideoStreamSettings();
    h264Settings.setProfileLevel(H264Profile.BASELINE, H264Level.LEVEL_3);
    ns.videoStreamSettings = h264Settings;
    
    var mic:Microphone = Microphone.getMicrophone(-1);
	mic.codec = SoundCodec.SPEEX;
	mic.framesPerPacket = 1;