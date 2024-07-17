from flask import Flask, request, jsonify, render_template, send_file
from datetime import datetime
import sqlite3
import io

app = Flask(__name__)
DATABASE = 'activity.db'

def init_db():
    with sqlite3.connect(DATABASE) as conn:
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS activity
                     (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, computername TEXT, domainname TEXT, last_active TEXT)''')
        c.execute('''CREATE TABLE IF NOT EXISTS screenshots
                     (id INTEGER PRIMARY KEY AUTOINCREMENT, activity_id INTEGER, timestamp TEXT, image BLOB, FOREIGN KEY(activity_id) REFERENCES activity(id))''')
        conn.commit()

@app.route('/api/activity', methods=['POST'])
def activity():
    data = request.json
    username = data['username']
    computername = data['computername']
    domainname = data['domainname']
    last_active = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    with sqlite3.connect(DATABASE) as conn:
        c = conn.cursor()
        c.execute('''INSERT INTO activity (username, computername, domainname, last_active)
                     VALUES (?, ?, ?, ?)''', (username, computername, domainname, last_active))
        conn.commit()
    
    return jsonify({"status": "success"}), 200

@app.route('/api/screenshot', methods=['POST'])
def screenshot():
    file = request.files['file']
    activity_id = request.form['activity_id']
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    image = file.read()
    
    with sqlite3.connect(DATABASE) as conn:
        c = conn.cursor()
        c.execute('''INSERT INTO screenshots (activity_id, timestamp, image)
                     VALUES (?, ?, ?)''', (activity_id, timestamp, image))
        conn.commit()
    
    return jsonify({"status": "success"}), 200

@app.route('/')
def index():
    with sqlite3.connect(DATABASE) as conn:
        c = conn.cursor()
        c.execute('''SELECT a.id, a.username, a.computername, a.domainname, a.last_active, s.timestamp, s.image
                     FROM activity a LEFT JOIN screenshots s ON a.id = s.activity_id''')
        data = c.fetchall()
    
    return render_template('index.html', data=data)

@app.route('/screenshot/<int:id>')
def get_screenshot(id):
    with sqlite3.connect(DATABASE) as conn:
        c = conn.cursor()
        c.execute('SELECT image FROM screenshots WHERE id = ?', (id,))
        data = c.fetchone()
    
    if data:
        return send_file(io.BytesIO(data[0]), mimetype='image/jpeg')
    else:
        return 'Screenshot not found', 404

if __name__ == '__main__':
    init_db()
    app.run(debug=True)
