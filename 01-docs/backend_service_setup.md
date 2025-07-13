# Backend Service Setup Guide (systemd)

This guide explains how to configure the Node.js backend server to run as a `systemd` service on a Raspberry Pi. This ensures the server starts automatically on boot and restarts if it crashes.

## 1. Deploying the Backend Code

Before creating the service, you must copy your backend application code to the Raspberry Pi.

#### Recommended Method: Git Repository

The standard and most reliable way to deploy code is by using a version control system. You can host your project on a service like GitHub or GitLab and then clone it directly onto your Raspberry Pi.

```bash
# Example command to clone a repository
git clone <your_repository_url> /home/pi/smart-garden-backend
```

#### Manual Method: SCP

If you are not using Git, you can copy the files manually from your development machine to the Raspberry Pi using `scp`. This command will recursively copy the entire backend directory.

```bash
# Run this from your local machine
# Replace <pi_ip_address> with your Raspberry Pi's IP address
scp -r path/to/your/backend/directory pi@<pi_ip_address>:/home/pi/smart-garden-backend
```

After deploying the code, you may need to install its dependencies:

```bash
# Run this on the Raspberry Pi
cd /home/pi/smart-garden-backend
npm install
```


This guide explains how to configure the Node.js backend server to run as a `systemd` service on a Raspberry Pi. This ensures the server starts automatically on boot and restarts if it crashes.

## Prerequisites

1.  The project code is located at `/home/pi/smart-garden-backend` on the Raspberry Pi.
2.  Node.js is installed, and the executable is located at `/usr/bin/node`.
3.  You have `sudo` privileges on the Raspberry Pi.

## Steps

### 1. Create the Service File

First, you need to create a `systemd` unit file that defines the service.

Create a file named `smart-garden-backend.service` with the following content:

```ini
[Unit]
Description=Smart Garden Backend Service
After=network.target

[Service]
User=pi
Group=pi
WorkingDirectory=/home/pi/smart-garden-backend
ExecStart=/usr/bin/node /home/pi/smart-garden-backend/server.js
Restart=always

[Install]
WantedBy=multi-user.target
```

### 2. Deploy the Service File to the Raspberry Pi

Next, you need to copy this file to your Raspberry Pi and move it to the correct system directory.

1.  **Copy the file to your Pi.**
    Use a tool like `scp` (Secure Copy) to transfer the `smart-garden-backend.service` file from your computer to the Raspberry Pi's home directory. From your local machine's terminal, run:
    ```bash
    # Replace <pi_ip_address> with your Raspberry Pi's actual IP address
    scp path/to/your/smart-garden-backend.service pi@<pi_ip_address>:/home/pi/
    ```

2.  **Move the file into the systemd directory.**
    Connect to your Raspberry Pi via SSH and run the following command:
    ```bash
    sudo mv /home/pi/smart-garden-backend.service /etc/systemd/system/
    ```

### 3. Enable and Start the Service

Once the file is in place, you can enable and start the service.

1.  **Reload the systemd daemon.**
    This command makes `systemd` aware of the new service.
    ```bash
    sudo systemctl daemon-reload
    ```

2.  **Enable the service.**
    This command hooks the service into the boot process, so it starts automatically.
    ```bash
    sudo systemctl enable smart-garden-backend.service
    ```

3.  **Start the service.**
    This command starts the backend server immediately.
    ```bash
    sudo systemctl start smart-garden-backend.service
    ```

### 4. Verify the Service Status

You can check if the service is running correctly at any time.

1.  **Check the status.**
    ```bash
    sudo systemctl status smart-garden-backend.service
    ```
    This will show you if the service is `active (running)` and display the latest log output.

2.  **Exit the status view.**
    To return to the command line from the status screen, simply press the `q` key.
