import React, { useState, useEffect, useRef, useCallback } from 'react';
import './App.css';
import ScheduleEditor from './ScheduleEditor';

function App() {
  const [valveStates, setValveStates] = useState({
    1: 'unknown',
    2: 'unknown',
    3: 'unknown',
  });
  const [controllerStatus, setControllerStatus] = useState('unknown');
  const [mqttMessages, setMqttMessages] = useState([]);
  const [showScheduleEditor, setShowScheduleEditor] = useState(false);
  const [currentSchedule, setCurrentSchedule] = useState({
    enabled: true,
    programs: [],
  });

  const wsRef = useRef(null); // Use useRef to hold the WebSocket instance

  const connectWebSocket = useCallback(() => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      return; // Already connected
    }

    const ws = new WebSocket('ws://localhost:3001');

    ws.onopen = () => {
      console.log('WebSocket Connected');
      wsRef.current = ws; // Store the connected WebSocket instance
    };

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log('Received MQTT message:', data);

      if (data.topic.startsWith('smart-garden/valve/')) {
        const valveNum = parseInt(data.topic.split('/')[2]);
        setValveStates((prevStates) => ({
          ...prevStates,
          [valveNum]: data.message,
        }));
      } else if (data.topic === 'smart-garden/controller/status') {
        setControllerStatus(data.message);
      } else if (data.topic === 'smart-garden/schedule/state') {
        try {
          const parsedSchedule = JSON.parse(data.message);
          setCurrentSchedule(parsedSchedule);
          console.log('Updated current schedule from MQTT:', parsedSchedule);
        } catch (e) {
          console.error('Failed to parse schedule JSON:', e);
        }
      }
      setMqttMessages((prevMessages) => [...prevMessages, data]);
    };

    ws.onclose = () => {
      console.log('WebSocket Disconnected. Attempting to reconnect...');
      wsRef.current = null; // Clear the ref
      setTimeout(connectWebSocket, 3000); // Attempt to reconnect after 3 seconds
    };

    ws.onerror = (error) => {
      console.error('WebSocket Error:', error);
      ws.close(); // Close the connection to trigger onclose and reconnection
    };
  }, [setValveStates, setControllerStatus, setCurrentSchedule, setMqttMessages]); // Dependencies for useCallback

  useEffect(() => {
    connectWebSocket();

    return () => {
      if (wsRef.current) {
        wsRef.current.close();
      }
    };
  }, [connectWebSocket]); // Dependency for useEffect

  const sendCommand = useCallback(async (topic, message) => {
    console.log(`Sending command: Topic='${topic}', Message='${message}'`);
    try {
      const response = await fetch('/api/command', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ topic, message }),
      });
      const data = await response.json();
      console.log('Command response:', data);
    } catch (error) {
      console.error('Error sending command:', error);
    }
  }, []); // Empty dependency array means this function is created once

  return (
    <div className="App">
      <header className="App-header">
        <h1>Smart Garden Dashboard</h1>
        <nav>
          <button onClick={() => setShowScheduleEditor(false)}>Dashboard</button>
          <button onClick={() => setShowScheduleEditor(true)}>Schedule Editor</button>
        </nav>

        {!showScheduleEditor ? (
          <>
            <p>Controller Status: {controllerStatus}</p>
            <div className="valve-controls">
              {Object.entries(valveStates).map(([valveNum, state]) => (
                <div key={valveNum} className="valve-card">
                  <h2>Valve {valveNum}</h2>
                  <p>State: {state}</p>
                  <button onClick={() => sendCommand(`smart-garden/valve/${valveNum}/command`, 'OPEN')}>Open</button>
                  <button onClick={() => sendCommand(`smart-garden/valve/${valveNum}/command`, 'CLOSE')}>Close</button>
                </div>
              ))}
            </div>
            <div className="mqtt-log">
              <h2>MQTT Message Log</h2>
              {mqttMessages.map((msg, index) => (
                <p key={index}>
                  <strong>{msg.topic}:</strong> {msg.message}
                </p>
              ))}
            </div>
          </>
        ) : (
          <ScheduleEditor sendCommand={sendCommand} currentSchedule={currentSchedule} />
        )}
      </header>
    </div>
  );
}

export default App;
