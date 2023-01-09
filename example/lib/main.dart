import 'dart:developer';

import 'package:agora_rtc_engine/agora_rtc_engine.dart';
import 'package:agora_rtc_rawdata/agora_rtc_rawdata.dart';
import 'package:agora_rtc_rawdata_example/config/agora.config.dart' as config;
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late RtcEngine engine;
  bool startPreview = false, isJoined = false;
  List<int> remoteUid = [];

  @override
  void initState() {
    super.initState();
    this._initEngine();
  }

  @override
  void dispose() {
    super.dispose();
    this._deinitEngine();
  }

  _initEngine() async {
    if (defaultTargetPlatform == TargetPlatform.android) {
      await [Permission.microphone, Permission.camera].request();
    }

    engine = createAgoraRtcEngine();
    await engine.initialize(RtcEngineContext(
        appId: config.appId,
        channelProfile: ChannelProfileType.channelProfileLiveBroadcasting));

    engine.registerEventHandler(RtcEngineEventHandler(
        onJoinChannelSuccess: (RtcConnection connection, int elapsed) {
      log('onJoinChannelSuccess connection: ${connection.toJson()} elapsed: $elapsed');
      setState(() {
        isJoined = true;
      });
    }, onUserJoined: (RtcConnection connection, int rUid, int elapsed) {
      log('onUserJoined connection: ${connection.toJson()} remoteUid: $rUid elapsed: $elapsed');
      setState(() {
        remoteUid.add(rUid);
      });
    }, onUserOffline:
            (RtcConnection connection, int rUid, UserOfflineReasonType reason) {
      log('onUserOffline connection: ${connection.toJson()} remoteUid: $rUid reason: $reason');
      setState(() {
        remoteUid.remove(rUid);
      });
    }));
    await engine.enableVideo();
    await engine.startPreview();
    setState(() {
      startPreview = true;
    });
    var handle = await engine.getNativeHandle();
    await AgoraRtcRawdata.registerAudioFrameObserver(handle);
    await AgoraRtcRawdata.registerVideoFrameObserver(handle);

    await engine.joinChannel(
        token: config.token,
        channelId: config.channelId,
        uid: config.uid,
        options: ChannelMediaOptions());
  }

  _deinitEngine() async {
    await AgoraRtcRawdata.unregisterAudioFrameObserver();
    await AgoraRtcRawdata.unregisterVideoFrameObserver();
    await engine.release();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Stack(
          children: [
            if (startPreview)
              AgoraVideoView(
                controller: VideoViewController(
                  rtcEngine: engine,
                  canvas: VideoCanvas(uid: 0),
                ),
              ),
            Align(
              alignment: Alignment.topLeft,
              child: SingleChildScrollView(
                scrollDirection: Axis.horizontal,
                child: Row(
                  children: List.of(remoteUid.map(
                    (e) => Container(
                      width: 120,
                      height: 120,
                      child: AgoraVideoView(
                        controller: VideoViewController.remote(
                          rtcEngine: engine,
                          canvas: VideoCanvas(uid: e),
                          connection: RtcConnection(
                            channelId: config.channelId,
                          ),
                        ),
                      ),
                    ),
                  )),
                ),
              ),
            )
          ],
        ),
      ),
    );
  }
}
