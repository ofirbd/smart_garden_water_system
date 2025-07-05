import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'services/web_socket_service.dart';
import 'package:mobile_web_app/screens/schedule_editor_screen.dart';

void main() {
  runApp(
    ChangeNotifierProvider(
      create: (context) => AppState(),
      child: const MyApp(),
    ),
  );
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Smart Garden Control',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      initialRoute: '/',
      routes: {
        '/': (context) => const DashboardScreen(),
        '/settings': (context) => const SettingsScreen(),
        '/schedule': (context) => const ScheduleEditorScreen(),
      },
    );
  }
}

class AppState extends ChangeNotifier {
  final WebSocketService _webSocketService = WebSocketService();
  String _backendStatus = "Disconnected";
  bool _isControllerOnline = false;
  final List<ValveState> _valves = [
    ValveState(name: 'Valve 1', isOpen: false),
    ValveState(name: 'Valve 2', isOpen: false),
    ValveState(name: 'Valve 3', isOpen: false),
  ];
  String _webSocketUrl = 'ws://localhost:3001'; // Default URL
  WateringSchedule _schedule = WateringSchedule(enabled: false, programs: []);

  String get backendStatus => _backendStatus;
  bool get isControllerOnline => _isControllerOnline;
  List<ValveState> get valves => _valves;
  String get webSocketUrl => _webSocketUrl;
  WateringSchedule get schedule => _schedule;

  AppState() {
    _webSocketService.stream.listen((message) {
      final data = jsonDecode(message);

      if (data['topic'] != null) {
        // Handle messages forwarded from MQTT
        final topic = data['topic'];
        final payload = data['message'];

        if (topic == 'smart-garden/controller/status') {
          _isControllerOnline = payload == 'online';
        } else if (topic.startsWith('smart-garden/valve/') && topic.endsWith('/state')) {
          final parts = topic.split('/');
          final valveIndex = int.parse(parts[2]) - 1;
          if (valveIndex >= 0 && valveIndex < _valves.length) {
            _valves[valveIndex].isOpen = payload == 'open';
          }
        } else if (topic == 'smart-garden/schedule/state') {
          _schedule = WateringSchedule.fromJson(jsonDecode(payload));
        }
      } else if (data['status'] != null) {
        // Handle direct connection status messages from the WebSocket service
        _backendStatus = data['status'];
      }
      notifyListeners();
    });
  }

  void connect() {
    _webSocketService.connect(_webSocketUrl);
  }

  void disconnect() {
    _webSocketService.disconnect();
  }

  void setWebSocketUrl(String url) {
    _webSocketUrl = url;
    notifyListeners();
  }

  void toggleValve(int valveIndex, bool isOpen) {
    // Optimistically update the UI
    _valves[valveIndex].isOpen = isOpen;
    notifyListeners();

    final command = {
      'type': 'valve_control',
      'payload': {
        'valve': valveIndex + 1,
        'command': isOpen ? 'OPEN' : 'CLOSE'
      }
    };
    _webSocketService.sendMessage(jsonEncode(command));
  }

  void saveSchedule(WateringSchedule schedule) {
    _schedule = schedule;
    final command = {
      'type': 'schedule_set',
      'payload': schedule.toJson(),
    };
    _webSocketService.sendMessage(jsonEncode(command));
    notifyListeners();
  }
}

class WateringSchedule {
  bool enabled;
  List<Program> programs;

  WateringSchedule({required this.enabled, required this.programs});

  factory WateringSchedule.fromJson(Map<String, dynamic> json) {
    return WateringSchedule(
      enabled: json['enabled'],
      programs: (json['programs'] as List).map((p) => Program.fromJson(p)).toList(),
    );
  }

  Map<String, dynamic> toJson() => {
        'enabled': enabled,
        'programs': programs.map((p) => p.toJson()).toList(),
      };
}

class Program {
  bool enabled;
  List<String> days;
  List<WateringAction> actions;

  Program({required this.enabled, required this.days, required this.actions});

  factory Program.fromJson(Map<String, dynamic> json) {
    return Program(
      enabled: json['enabled'],
      days: List<String>.from(json['days']),
      actions: (json['actions'] as List).map((a) => WateringAction.fromJson(a)).toList(),
    );
  }

  Map<String, dynamic> toJson() => {
        'enabled': enabled,
        'days': days,
        'actions': actions.map((a) => a.toJson()).toList(),
      };
}

class WateringAction {
  int valve;
  String time;
  int duration_minutes;

  WateringAction({required this.valve, required this.time, required this.duration_minutes});

  factory WateringAction.fromJson(Map<String, dynamic> json) {
    return WateringAction(
      valve: json['valve'],
      time: json['time'],
      duration_minutes: json['duration_minutes'],
    );
  }

  Map<String, dynamic> toJson() => {
        'valve': valve,
        'time': time,
        'duration_minutes': duration_minutes,
      };
}

class ValveState {
  final String name;
  bool isOpen;

  ValveState({required this.name, required this.isOpen});
}

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  _DashboardScreenState createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  @override
  void initState() {
    super.initState();
    // Connect automatically when the dashboard is shown
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<AppState>(context, listen: false).connect();
    });
  }

  @override
  Widget build(BuildContext context) {
    final appState = Provider.of<AppState>(context);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Smart Garden Dashboard'),
        actions: [
          IconButton(
            icon: const Icon(Icons.calendar_today),
            onPressed: () {
              Navigator.pushNamed(context, '/schedule');
            },
          ),
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () {
              Navigator.pushNamed(context, '/settings');
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            Text(
              'System Status',
              style: Theme.of(context).textTheme.headlineSmall,
            ),
            const SizedBox(height: 16),
            Card(
              elevation: 4,
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  children: [
                    StatusRow(
                      title: 'Backend Connection:',
                      status: appState.backendStatus,
                      color: appState.backendStatus == 'connected' ? Colors.green : Colors.red,
                    ),
                    const SizedBox(height: 12),
                    StatusRow(
                      title: 'ESP32 Controller:',
                      status: appState.isControllerOnline ? 'Online' : 'Offline',
                      color: appState.isControllerOnline ? Colors.green : Colors.red,
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 24),
            Text(
              'Valve Control',
              style: Theme.of(context).textTheme.headlineSmall,
            ),
            const SizedBox(height: 16),
            Expanded(
              child: ListView.builder(
                itemCount: appState.valves.length,
                itemBuilder: (context, index) {
                  final valve = appState.valves[index];
                  return Card(
                    elevation: 2,
                    margin: const EdgeInsets.symmetric(vertical: 8),
                    child: ListTile(
                      leading: Icon(
                        valve.isOpen ? Icons.opacity : Icons.tonality,
                        color: valve.isOpen ? Colors.blue : Colors.grey,
                        size: 40,
                      ),
                      title: Text(valve.name),
                      subtitle: Text(
                        valve.isOpen ? 'Open' : 'Closed',
                        style: TextStyle(
                          color: valve.isOpen ? Colors.green : Colors.red,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      trailing: Switch(
                        value: valve.isOpen,
                        onChanged: (bool value) {
                          appState.toggleValve(index, value);
                        },
                      ),
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  _SettingsScreenState createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  late TextEditingController _urlController;

  @override
  void initState() {
    super.initState();
    _urlController = TextEditingController(text: Provider.of<AppState>(context, listen: false).webSocketUrl);
  }

  @override
  Widget build(BuildContext context) {
    final appState = Provider.of<AppState>(context, listen: false);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: <Widget>[
            TextField(
              controller: _urlController,
              decoration: const InputDecoration(
                labelText: 'WebSocket URL',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: () {
                appState.setWebSocketUrl(_urlController.text);
                appState.disconnect();
                appState.connect();
                Navigator.pop(context);
              },
              child: const Text('Save and Reconnect'),
            ),
          ],
        ),
      ),
    );
  }
}

class StatusRow extends StatelessWidget {
  final String title;
  final String status;
  final Color color;

  const StatusRow({
    super.key,
    required this.title,
    required this.status,
    required this.color,
  });

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text(title, style: Theme.of(context).textTheme.titleMedium),
        Row(
          children: [
            Container(
              width: 12,
              height: 12,
              decoration: BoxDecoration(
                color: color,
                shape: BoxShape.circle,
              ),
            ),
            const SizedBox(width: 8),
            Text(
              status,
              style: Theme.of(context).textTheme.titleMedium?.copyWith(fontWeight: FontWeight.bold),
            ),
          ],
        ),
      ],
    );
  }
}
