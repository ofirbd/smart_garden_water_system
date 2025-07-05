import 'dart:async';
import 'package:web_socket_channel/web_socket_channel.dart';

class WebSocketService {
  WebSocketChannel? _channel;
  final StreamController<dynamic> _streamController = StreamController<dynamic>.broadcast();
  bool _isConnected = false;

  Stream<dynamic> get stream => _streamController.stream;
  bool get isConnected => _isConnected;

  void connect(String url) {
    if (_isConnected) return;

    try {
      _channel = WebSocketChannel.connect(Uri.parse(url));
      _isConnected = true;
      _streamController.add('{"status": "connected"}');

      _channel!.stream.listen(
        (data) {
          _streamController.add(data);
        },
        onDone: () {
          _isConnected = false;
          _streamController.add('{"status": "disconnected"}');
        },
        onError: (error) {
          _isConnected = false;
          _streamController.add('{"status": "error", "message": "$error"}');
        },
      );
    } catch (e) {
      _isConnected = false;
      _streamController.add('{"status": "error", "message": "Connection failed: $e"}');
    }
  }

  void sendMessage(String message) {
    if (_isConnected && _channel != null) {
      _channel!.sink.add(message);
    }
  }

  void disconnect() {
    if (_channel != null) {
      _channel!.sink.close();
      _isConnected = false;
    }
  }
}
