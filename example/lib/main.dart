import 'dart:developer';

import 'package:agora_rtc_engine/rtc_engine.dart';
import 'package:agora_rtc_engine/rtc_local_view.dart' as RtcLocalView;
import 'package:agora_rtc_engine/rtc_remote_view.dart' as RtcRemoteView;
import 'package:agora_rtc_rawdata/agora_rtc_rawdata.dart';
import 'package:agora_rtc_rawdata_example/config/agora.config.dart' as config;
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  RtcEngine _engine = null;

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool isJoined = false;
  int remoteUid;

  @override
  void initState() {
    super.initState();
    this._initEngine();
  }

  @override
  void dispose() {
    super.dispose();
    widget._engine?.destroy();
  }

  _initEngine() async {
    if (defaultTargetPlatform == TargetPlatform.android) {
      await [Permission.microphone, Permission.camera].request();
    }

    widget._engine = await RtcEngine.create(config.appId);
    widget._engine?.setEventHandler(
        RtcEngineEventHandler(joinChannelSuccess: (channel, uid, elapsed) {
      log('joinChannelSuccess $channel $uid $elapsed');
      setState(() => isJoined = true);
    }, userJoined: (uid, elapsed) {
      log('userJoined  $uid $elapsed');
      setState(() => remoteUid = uid);
    }));
    await widget._engine.enableVideo();
    await AgoraRtcRawdata.registerAudioFrameObserver(
        await widget._engine.getNativeHandle());
    await AgoraRtcRawdata.registerVideoFrameObserver(
        await widget._engine.getNativeHandle());
    await widget._engine
        ?.joinChannel(config.token, config.channelId, null, config.uid);
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
            if (isJoined) RtcLocalView.SurfaceView(),
            if (remoteUid != null)
              Align(
                alignment: Alignment.topLeft,
                child: Container(
                  width: 200,
                  height: 200,
                  child: RtcRemoteView.SurfaceView(uid: remoteUid),
                ),
              )
          ],
        ),
      ),
    );
  }
}
