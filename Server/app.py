from flask import Flask, render_template, request, jsonify
from datetime import datetime
import socket, json, os, time

app = Flask(__name__)
ADMIN_PASS = "0000"
HISTORY_FILE = "alarm_history.json"
MAX_HISTORY = 50
MAX_CHART = 40
OFFLINE_TIMEOUT = 10

system_state = "DISARMED"
alert_history = []
device_info = {"ip": None, "last_seen_ts": 0, "sensors": {}}
sensor_history = {"hall": [], "ldr": [], "max": MAX_CHART}

def get_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except:
        return "127.0.0.1"

def load_hist():
    global alert_history
    if os.path.exists(HISTORY_FILE):
        try:
            with open(HISTORY_FILE, "r", encoding="utf-8") as f:
                alert_history = json.load(f)
        except:
            alert_history = []

def save_hist():
    while len(alert_history) > MAX_HISTORY:
        alert_history.pop()
    with open(HISTORY_FILE, "w", encoding="utf-8") as f:
        json.dump(alert_history, f, ensure_ascii=False, indent=2)

def add_log(typ, det="", data=None):
    entry = {
        "timestamp": datetime.now().isoformat(),
        "time": datetime.now().strftime("%H:%M:%S"),
        "date": datetime.now().strftime("%Y-%m-%d"),
        "type": typ,
        "details": det,
        "sensor_data": data or {}
    }
    alert_history.insert(0, entry)
    save_hist()
    print(f"[LOG] {entry['time']} | {typ} | {det}")

def update_chart(h, l):
    if h is not None:
        sensor_history["hall"].append(h)
        if len(sensor_history["hall"]) > sensor_history["max"]:
            sensor_history["hall"].pop(0)
    if l is not None:
        sensor_history["ldr"].append(l)
        if len(sensor_history["ldr"]) > sensor_history["max"]:
            sensor_history["ldr"].pop(0)

@app.route("/")
def dashboard():
    last_ts = device_info.get("last_seen_ts", 0)
    last_str = datetime.fromtimestamp(last_ts).strftime("%H:%M:%S") if last_ts > 0 else "?"
    return render_template("index.html", 
                          server_ip=get_ip(), 
                          current_state=system_state,
                          device_ip=device_info.get("ip", "?"), 
                          last_seen=last_str,
                          sensors=device_info.get("sensors", {}))

@app.route("/api/status", methods=["GET"])
def get_status():
    last_ts = device_info.get("last_seen_ts", 0)
    online = (time.time() - last_ts) < OFFLINE_TIMEOUT if last_ts > 0 else False
    last_str = datetime.fromtimestamp(last_ts).strftime("%H:%M:%S") if last_ts > 0 else "?"
    
    return jsonify({
        "state": system_state,
        "device_ip": device_info.get("ip"),
        "last_seen": last_str,
        "online": online,
        "sensors": device_info.get("sensors", {}),
        "server_ip": get_ip()
    })

@app.route("/api/alarm", methods=["POST"])
def receive_alarm():
    global system_state
    data = request.json or {}
    
    device_info["ip"] = data.get("ip")
    device_info["last_seen_ts"] = time.time()
    device_info["sensors"] = {
        "hall": data.get("hall"),
        "ldr": data.get("ldr"),
        "night": False
    }
    
    update_chart(data.get("hall"), data.get("ldr"))
    
    sensor = data.get("sensor", "")
    if sensor == "periodic_update":
        return jsonify({"status": "updated", "state": system_state})
    
    manual = data.get("manual", False)
    
    if system_state == "ARMED" or manual:
        old = system_state
        system_state = "TRIGGERED"
        
        if manual:
            add_log("MANUAL", f"دستی: {sensor}", data)
        else:
            add_log("ALARM", f"نفوذ: {sensor}", data)
        
        return jsonify({
            "status": "received",
            "state": "TRIGGERED",
            "previous": old
        })
    
    return jsonify({"status": "ignored", "state": system_state})

# ✅ arm() بدون شرط - همیشه ARM شود
@app.route("/api/arm", methods=["POST"])
def arm():
    global system_state
    
    print(f"[API] /api/arm called, current state={system_state}")
    
    system_state = "ARMED"
    add_log("ARMED", "فعال از داشبورد")
    
    print(f"[API] State changed to ARMED")
    
    return jsonify({"status": "ok", "state": system_state})

@app.route("/api/disarm", methods=["POST"])
def disarm():
    global system_state
    data = request.json or {}
    
    if data.get("pass") == ADMIN_PASS:
        if system_state != "DISARMED":
            system_state = "DISARMED"
            add_log("DISARMED", "غیرفعال از داشبورد")
        return jsonify({"status": "ok", "state": "DISARMED"})
    
    add_log("AUTH_FAIL", "رمز اشتباه")
    return jsonify({"status": "error", "message": "رمز اشتباه"}), 401

@app.route("/api/history", methods=["GET"])
def history():
    return jsonify(alert_history)

@app.route("/api/history/clear", methods=["POST"])
def clear_hist():
    global alert_history
    alert_history = []
    save_hist()
    add_log("SYSTEM", "تاریخچه پاک شد")
    return jsonify({"status": "cleared"})

@app.route("/api/sensors/history", methods=["GET"])
def chart_hist():
    return jsonify({
        "hall": sensor_history["hall"],
        "ldr": sensor_history["ldr"]
    })

if __name__ == "__main__":
    load_hist()
    print(f"Server: http://{get_ip()}:5000 | Pass: {ADMIN_PASS}")
    app.run(host="0.0.0.0", port=5000, debug=False)