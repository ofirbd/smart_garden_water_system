import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../main.dart';

class ScheduleEditorScreen extends StatefulWidget {
  const ScheduleEditorScreen({super.key});

  @override
  _ScheduleEditorScreenState createState() => _ScheduleEditorScreenState();
}

class _ScheduleEditorScreenState extends State<ScheduleEditorScreen> {
  late WateringSchedule _schedule;

  @override
  void initState() {
    super.initState();
    // Deep copy the schedule to allow for local edits
    _schedule = WateringSchedule.fromJson(Provider.of<AppState>(context, listen: false).schedule.toJson());
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Watering Schedule'),
        actions: [
          IconButton(
            icon: const Icon(Icons.save),
            onPressed: () {
              Provider.of<AppState>(context, listen: false).saveSchedule(_schedule);
              Navigator.pop(context);
            },
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(8.0),
        children: [
          SwitchListTile(
            title: const Text('Schedule Enabled'),
            value: _schedule.enabled,
            onChanged: (bool value) {
              setState(() {
                _schedule.enabled = value;
              });
            },
          ),
          const Divider(),
          ..._schedule.programs.asMap().entries.map((entry) {
            int idx = entry.key;
            Program prog = entry.value;
            return ProgramCard(program: prog, onUpdate: (updatedProg) {
              setState(() {
                _schedule.programs[idx] = updatedProg;
              });
            }, onDelete: () {
              setState(() {
                _schedule.programs.removeAt(idx);
              });
            });
          }).toList(),
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: ElevatedButton.icon(
              icon: const Icon(Icons.add),
              label: const Text('Add Program'),
              onPressed: () {
                setState(() {
                  _schedule.programs.add(Program(enabled: true, days: [], actions: []));
                });
              },
            ),
          ),
        ],
      ),
    );
  }
}

class ProgramCard extends StatelessWidget {
  final Program program;
  final ValueChanged<Program> onUpdate;
  final VoidCallback onDelete;

  const ProgramCard({super.key, required this.program, required this.onUpdate, required this.onDelete});

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 2.0,
      margin: const EdgeInsets.symmetric(vertical: 8.0),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: [
            SwitchListTile(
              title: const Text('Program Enabled'),
              value: program.enabled,
              onChanged: (bool value) {
                program.enabled = value;
                onUpdate(program);
              },
              secondary: IconButton(
                icon: const Icon(Icons.delete, color: Colors.red),
                onPressed: onDelete,
              ),
            ),
            DaySelector(selectedDays: program.days, onSelectionChanged: (days) {
              program.days = days;
              onUpdate(program);
            }),
            const Divider(),
            ...program.actions.asMap().entries.map((entry) {
              int idx = entry.key;
              WateringAction action = entry.value;
              return ActionTile(action: action, onUpdate: (updatedAction) {
                program.actions[idx] = updatedAction;
                onUpdate(program);
              }, onDelete: () {
                program.actions.removeAt(idx);
                onUpdate(program);
              });
            }).toList(),
            TextButton.icon(
              icon: const Icon(Icons.add_circle_outline),
              label: const Text('Add Action'),
              onPressed: () {
                program.actions.add(WateringAction(valve: 1, time: '08:00', duration_minutes: 10));
                onUpdate(program);
              },
            ),
          ],
        ),
      ),
    );
  }
}

class DaySelector extends StatelessWidget {
  final List<String> selectedDays;
  final ValueChanged<List<String>> onSelectionChanged;
  final Map<String, String> _days = {
    'Monday': 'Mon',
    'Tuesday': 'Tue',
    'Wednesday': 'Wed',
    'Thursday': 'Thu',
    'Friday': 'Fri',
    'Saturday': 'Sat',
    'Sunday': 'Sun',
  };

  DaySelector({super.key, required this.selectedDays, required this.onSelectionChanged});

  @override
  Widget build(BuildContext context) {
    return Wrap(
      spacing: 8.0,
      children: _days.entries.map((entry) {
        final fullName = entry.key;
        final shortName = entry.value;
        final isSelected = selectedDays.contains(fullName);
        return FilterChip(
          label: Text(shortName),
          selected: isSelected,
          onSelected: (bool selected) {
            final newSelection = List<String>.from(selectedDays);
            if (selected) {
              newSelection.add(fullName);
            } else {
              newSelection.remove(fullName);
            }
            onSelectionChanged(newSelection);
          },
        );
      }).toList(),
    );
  }
}

class ActionTile extends StatelessWidget {
  final WateringAction action;
  final ValueChanged<WateringAction> onUpdate;
  final VoidCallback onDelete;

  const ActionTile({super.key, required this.action, required this.onUpdate, required this.onDelete});

  @override
  Widget build(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.water_drop),
      title: Text('Valve ${action.valve} at ${action.time}'),
      subtitle: Text('for ${action.duration_minutes} minutes'),
      trailing: IconButton(
        icon: const Icon(Icons.delete_outline, color: Colors.redAccent),
        onPressed: onDelete,
      ),
      onTap: () async {
        final result = await showDialog<WateringAction>(
          context: context,
          builder: (context) => ActionEditorDialog(action: action),
        );
        if (result != null) {
          onUpdate(result);
        }
      },
    );
  }
}

class ActionEditorDialog extends StatefulWidget {
  final WateringAction action;

  const ActionEditorDialog({super.key, required this.action});

  @override
  _ActionEditorDialogState createState() => _ActionEditorDialogState();
}

class _ActionEditorDialogState extends State<ActionEditorDialog> {
  late int _valve;
  late TimeOfDay _time;
  late int _duration;

  @override
  void initState() {
    super.initState();
    _valve = widget.action.valve;
    final timeParts = widget.action.time.split(':');
    _time = TimeOfDay(hour: int.parse(timeParts[0]), minute: int.parse(timeParts[1]));
    _duration = widget.action.duration_minutes;
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('Edit Action'),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          DropdownButtonFormField<int>(
            value: _valve,
            items: [1, 2, 3].map((v) => DropdownMenuItem(value: v, child: Text('Valve $v'))).toList(),
            onChanged: (value) {
              if (value != null) setState(() => _valve = value);
            },
            decoration: const InputDecoration(labelText: 'Valve'),
          ),
          const SizedBox(height: 16),
          ListTile(
            title: const Text('Start Time'),
            subtitle: Text(_time.format(context)),
            onTap: () async {
              final newTime = await showTimePicker(context: context, initialTime: _time);
              if (newTime != null) {
                setState(() => _time = newTime);
              }
            },
          ),
          const SizedBox(height: 16),
          TextFormField(
            initialValue: _duration.toString(),
            decoration: const InputDecoration(labelText: 'Duration (minutes)'),
            keyboardType: TextInputType.number,
            onChanged: (value) {
              _duration = int.tryParse(value) ?? _duration;
            },
          ),
        ],
      ),
      actions: [
        TextButton(onPressed: () => Navigator.pop(context), child: const Text('Cancel')),
        ElevatedButton(
          onPressed: () {
            final updatedAction = WateringAction(
              valve: _valve,
              time: '${_time.hour.toString().padLeft(2, '0')}:${_time.minute.toString().padLeft(2, '0')}',
              duration_minutes: _duration,
            );
            Navigator.pop(context, updatedAction);
          },
          child: const Text('Save'),
        ),
      ],
    );
  }
}