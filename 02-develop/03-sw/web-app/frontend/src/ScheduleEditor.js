import React, { useState, useEffect } from 'react';

const ScheduleEditor = ({ sendCommand, currentSchedule }) => {
  const [schedule, setSchedule] = useState(currentSchedule);

  // Update internal schedule state when currentSchedule prop changes
  useEffect(() => {
    setSchedule(currentSchedule);
  }, [currentSchedule]);

  useEffect(() => {
    // Request the current schedule from the ESP32 when the component mounts
    // This ensures the editor starts with the latest schedule from the device
    sendCommand('smart-garden/schedule/get', '');
  }, [sendCommand]);

  const handleAddProgram = () => {
    setSchedule((prevSchedule) => ({
      ...prevSchedule,
      programs: [
        ...prevSchedule.programs,
        {
          enabled: true,
          days: [],
          actions: [],
        },
      ],
    }));
  };

  const handleProgramChange = (index, field, value) => {
    const newPrograms = [...schedule.programs];
    newPrograms[index][field] = value;
    setSchedule((prevSchedule) => ({
      ...prevSchedule,
      programs: newPrograms,
    }));
  };

  const handleAddAction = (programIndex) => {
    const newPrograms = [...schedule.programs];
    newPrograms[programIndex].actions.push({
      valve: 1,
      time: '00:00',
      duration_minutes: 5,
    });
    setSchedule((prevSchedule) => ({
      ...prevSchedule,
      programs: newPrograms,
    }));
  };

  const handleActionChange = (programIndex, actionIndex, field, value) => {
    const newPrograms = [...schedule.programs];
    newPrograms[programIndex].actions[actionIndex][field] = value;
    setSchedule((prevSchedule) => ({
      ...prevSchedule,
      programs: newPrograms,
    }));
  };

  const handleSaveSchedule = () => {
    const scheduleJson = JSON.stringify(schedule);
    console.log('Attempting to save schedule:', scheduleJson);
    // Send the updated schedule to the ESP32
    sendCommand('smart-garden/schedule/set', scheduleJson);
  };

  return (
    <div className="schedule-editor">
      <h2>Schedule Editor</h2>
      <button onClick={handleAddProgram}>Add New Program</button>
      {schedule.programs.map((program, programIndex) => (
        <div key={programIndex} className="program-card">
          <h3>Program {programIndex + 1}</h3>
          <label>
            Enabled:
            <input
              type="checkbox"
              checked={program.enabled}
              onChange={(e) =>
                handleProgramChange(programIndex, 'enabled', e.target.checked)
              }
            />
          </label>
          <div>
            Days:
            {['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'].map(day => (
              <label key={day}>
                <input
                  type="checkbox"
                  checked={program.days.includes(day)}
                  onChange={(e) => {
                    const newDays = e.target.checked
                      ? [...program.days, day]
                      : program.days.filter((d) => d !== day);
                    handleProgramChange(programIndex, 'days', newDays);
                  }}
                />
                {day}
              </label>
            ))}
          </div>
          <h4>Actions</h4>
          <button onClick={() => handleAddAction(programIndex)}>Add Action</button>
          {program.actions.map((action, actionIndex) => (
            <div key={actionIndex} className="action-card">
              <h5>Action {actionIndex + 1}</h5>
              <label>
                Valve:
                <select
                  value={action.valve}
                  onChange={(e) =>
                    handleActionChange(
                      programIndex,
                      actionIndex,
                      'valve',
                      parseInt(e.target.value)
                    )
                  }
                >
                  <option value={1}>1</option>
                  <option value={2}>2</option>
                  <option value={3}>3</option>
                </select>
              </label>
              <label>
                Time:
                <input
                  type="time"
                  value={action.time}
                  onChange={(e) =>
                    handleActionChange(
                      programIndex,
                      actionIndex,
                      'time',
                      e.target.value
                    )
                  }
                />
              </label>
              <label>
                Duration (minutes):
                <input
                  type="number"
                  value={action.duration_minutes}
                  onChange={(e) =>
                    handleActionChange(
                      programIndex,
                      actionIndex,
                      'duration_minutes',
                      parseInt(e.target.value) || 0 // Ensure a number, default to 0 if NaN
                    )
                  }
                />
              </label>
            </div>
          ))}
        </div>
      ))}
      <button onClick={handleSaveSchedule}>Save Schedule</button>
    </div>
  );
};

export default ScheduleEditor;