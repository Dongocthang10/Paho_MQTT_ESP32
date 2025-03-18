import tkinter as tk
from tkinter import ttk, scrolledtext
import paho.mqtt.client as mqtt
import json

class MQTTController:
    def __init__(self, root):
        self.root = root
        self.root.title("MQTT Device Controller")
        self.root.geometry("800x800")
        
        # MQTT Settings Frame
        settings_frame = ttk.LabelFrame(root, text="MQTT Settings")
        settings_frame.pack(fill="x", padx=5, pady=5)
        
        # MQTT Broker settings
        ttk.Label(settings_frame, text="MQTT Broker IP:").pack()
        self.ip_entry = ttk.Entry(settings_frame)
        self.ip_entry.insert(0, "192.168.1.52")
        self.ip_entry.pack()
        
        ttk.Label(settings_frame, text="Port:").pack()
        self.port_entry = ttk.Entry(settings_frame)
        self.port_entry.insert(0, "1883")
        self.port_entry.pack()
        
        ttk.Label(settings_frame, text="Username:").pack()
        self.username_entry = ttk.Entry(settings_frame)
        self.username_entry.insert(0, "thang")
        self.username_entry.pack()
        
        ttk.Label(settings_frame, text="Password:").pack()
        self.password_entry = ttk.Entry(settings_frame, show="*")
        self.password_entry.insert(0, "123456")
        self.password_entry.pack()
        
        # Connect Button
        self.connect_button = ttk.Button(settings_frame, text="Connect", command=self.connect_mqtt)
        self.connect_button.pack(pady=5)
        
        # Devices Frame
        devices_frame = ttk.LabelFrame(root, text="Devices Control")
        devices_frame.pack(fill="x", padx=5, pady=5)
        
        # Device controls
        self.devices = {
            "led": {"name": "LED", "state": False},
            "relay1": {"name": "Relay 1", "state": False},
            "relay2": {"name": "Relay 2", "state": False},
            "relay3": {"name": "Relay 3", "state": False},
            "relay4": {"name": "Relay 4", "state": False}
        }
        
        for device_id, device_info in self.devices.items():
            frame = ttk.Frame(devices_frame)
            frame.pack(fill="x", padx=5, pady=2)
            
            ttk.Label(frame, text=device_info["name"]).pack(side="left")
            
            on_button = ttk.Button(frame, text="ON", 
                                 command=lambda d=device_id: self.control_device(d, "ON"))
            on_button.pack(side="left", padx=5)
            
            off_button = ttk.Button(frame, text="OFF", 
                                  command=lambda d=device_id: self.control_device(d, "OFF"))
            off_button.pack(side="left")
        
        # Log Frame
        log_frame = ttk.LabelFrame(root, text="Log")
        log_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10)
        self.log_text.pack(fill="both", expand=True)
        
        self.client = None
        
    def connect_mqtt(self):
        broker_address = self.ip_entry.get()
        broker_port = int(self.port_entry.get())
        username = self.username_entry.get()
        password = self.password_entry.get()
        
        self.client = mqtt.Client()
        self.client.username_pw_set(username, password)
        self.client.on_message = self.on_message
        self.client.on_connect = self.on_connect
        
        try:
            self.client.connect(broker_address, broker_port)
            self.client.loop_start()
            self.log("Connected to MQTT broker")
        except Exception as e:
            self.log(f"Error connecting to MQTT broker: {e}")
    
    def on_connect(self, client, userdata, flags, rc):
        self.client.subscribe("devices/status")
    
    def on_message(self, client, userdata, message):
        try:
            payload = json.loads(message.payload.decode())
            self.log(f"Received status update: {payload}")
            # Update device states
            for device_id, state in payload.items():
                if device_id in self.devices:
                    self.devices[device_id]["state"] = state
        except Exception as e:
            self.log(f"Error processing message: {e}")
    
    def control_device(self, device_id, command):
        if self.client and self.client.is_connected():
            message = {device_id: command}
            self.client.publish("devices/control", json.dumps(message))
            self.log(f"Sent command: {device_id} -> {command}")
        else:
            self.log("Not connected to MQTT broker")
    
    def log(self, message):
        self.log_text.insert(tk.END, f"{message}\n")
        self.log_text.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = MQTTController(root)
    root.mainloop()